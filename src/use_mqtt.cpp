#include "use_mqtt.h"

const char* TAG_MQTT = "USE_MQTT";

My_PubSubClient::My_PubSubClient(Client& client)
  : PubSubClient(client) {
  setReconnectCallback(this->_my_reconnect_callback);
  _setUser("my_user");
  _client = &client;
}

My_PubSubClient::~My_PubSubClient() {
}

void My_PubSubClient::_my_reconnect_callback(My_PubSubClient& obj) {
  obj._publish("bf_test/out", "Please use setReconnectCallback()");
  ESP_LOGI(TAG_MQTT, "Please use setReconnectCallback()");
}

My_PubSubClient& My_PubSubClient::setReconnectCallback(MQTT_RECONNECT_CALLBACK_SIGNATURE) {
  this->reconnect_callback = reconnect_callback;
  return *this;
}


bool My_PubSubClient::_publish(const char* payload, const char* message, bool LOG) {

  if (LOG) {
    // Save to SD
  }

  bool res = this->publish(payload, message);

  if (!res) {
    if (LOG) {
      // Save idx
      MQTT_FIFO++;
    }
  }

  return res;
}

void My_PubSubClient::_publish_saved(long idx) {
  if (idx == -1) {
    if (MQTT_FIFO > 0) {
      // Read SD - idx
      bool res = this->_publish("bf_test/try_again", ".", false);
      if (res) {
        // Remove SD - idx
        ESP_LOGI(TAG_MQTT, "Resended OK");
        MQTT_FIFO--;
      } else {
        ESP_LOGI(TAG_MQTT, "Resended FAIL");
      }
    }
  }
}

void My_PubSubClient::TaskReconnect(void* pvParameters) {
  // My_PubSubClient *obj = static_cast<My_PubSubClient*>(pvParameters);
  // obj->_TaskReconnect();
  ((My_PubSubClient*)pvParameters)->_TaskReconnect();
  // pvParameters._TaskReconnect();
}

void My_PubSubClient::_TaskReconnect() {

  for (;;) {
    if (!this->connected()) {
      ESP_LOGI(TAG_MQTT, "Not Connected");
      long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (this->_reconnect()) {
          lastReconnectAttempt = 0;
          ESP_LOGI(TAG_MQTT, "Connected Again");
        }
      }
    } else {
      // Client connected
      for (int i = 0; i < 5; i++)
        this->_publish_saved(-1);

      this->loop();
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void My_PubSubClient::_setUser(const char* _usr){
  strcpy(my_user,_usr);
}

bool My_PubSubClient::_reconnect() {
  if (!this->connected()) {
    this->disconnect();
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Pequeña pausa para liberar recursos
    if (this->connect(my_user)) {
      reconnect_callback(*this);
      // Once connected, publish an announcement...
      ESP_LOGI(TAG_MQTT, "Connected Again");
      return true;
    }
  }
  return false;
}
