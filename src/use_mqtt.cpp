#include "use_mqtt.h"


const char* TAG_MQTT = "USE_MQTT";


void My_PubSubClient::initQueue() {
  stringQueue = xQueueCreate(MAX_MQTT_SAVED, sizeof(MQTT_Pair));
  if (stringQueue == NULL) {
      ESP_LOGI(TAG_MQTT,"Failed to create the queue.\n");
  }
}

void My_PubSubClient::addToMQTTQueue(const char* data1, const char* data2) {
    MQTT_Pair pair;
    strncpy(pair.topic, data1, MAX_TOPIC_LENGTH);
    strncpy(pair.msg, data2, MAX_MSG_LENGTH);
    if (xQueueSend(stringQueue, &pair, portMAX_DELAY) != pdPASS) {
      ESP_LOGI(TAG_MQTT,"Failed to send data to the queue.\n");
    }
    MQTT_FIFO++;
}

bool My_PubSubClient::getToMQTTQueue(MQTT_Pair& Buffer) {
    if (xQueueReceive(stringQueue, &Buffer, portMAX_DELAY) == pdTRUE) {
      MQTT_FIFO--;
      return true;
    } else {
      ESP_LOGI(TAG_MQTT,"Failed to receive data from the queue.\n");
      return false;
    }
}

My_PubSubClient::My_PubSubClient(Client& client)
  : PubSubClient(client) {
  setReconnectCallback(this->_my_reconnect_callback);
  _setUser("my_user");
  _client = &client;
  initQueue();
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
    if (SD_INSERTED()){
      SD_appendFile(SD, "/MQTT_OUT.log", payload,'\n');
    }else{

    }
  }

  bool res = this->publish(payload, message);

  if (!res) {
    if (LOG) {
      // Save idx
      if (SD_INSERTED()){
        SD_appendFile(SD, "/MQTT_FIFO.log", payload,'\n');
        SD_appendFile(SD, "/MQTT_FIFO.log", message,'\n');
      }else{
        this->addToMQTTQueue(payload,message);
      }
    }
  }
  return res;
}

void My_PubSubClient::_publish_saved(long idx) {
  if (idx == -1) {
    if (MQTT_FIFO > 0) {
      
      MQTT_Pair rec;
      if(getToMQTTQueue(rec)){

        if (this->_publish(rec.topic,rec.msg, false)) {
          ESP_LOGI(TAG_MQTT, "Resended OK");
        } else {
          ESP_LOGI(TAG_MQTT, "Resended FAIL");
          addToMQTTQueue(rec.topic,rec.msg);
        }
      }
      

    }else if (SD_INSERTED()){

        if (SD_takeMutex()){
          
          File file = SD.open("/MQTT_FIFO.log");
          if (!file) {
            ESP_LOGI(SD_TAG, "Failed to open file for reading");
            SD_releaseMutex();
            return;
          }

          if(!file.available()){
            file.close();
            SD_releaseMutex();
            return;
          }

          char topic_tmp[100];
          char msg_tmp[100];

          size_t pos_topic = SD_readFileOpennedUntil(file,topic_tmp,0,'\n');
          size_t pos_msg = SD_readFileOpennedUntil(file,msg_tmp,pos_topic+1,'\n');

          file.close();
          SD_releaseMutex();
    
          
          ESP_LOGI(TAG_MQTT,"Try to send in");
          ESP_LOGI(TAG_MQTT,"[%s] [%s]",topic_tmp,msg_tmp);


          bool res = this->_publish(topic_tmp, msg_tmp, false);

          if (res) {
            SD_cropfile(SD,"/MQTT_FIFO.log",pos_topic+pos_msg+2);
          } else {
            ESP_LOGI(TAG_MQTT, "Resended FAIL");
          }
          
        }else{
          return;
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
      if (now - lastReconnectAttempt > this->wait_to_reconnect) {
        lastReconnectAttempt = now;
        // Attempt to reconnect
        if (this->_reconnect()) {
          lastReconnectAttempt = 0;
          ESP_LOGI(TAG_MQTT, "Connected Again");
        }
      }
    } else {
      // Client connected
      this->_publish_saved(-1);

      this->loop();
    }
    

    vTaskDelay(this->delay_for_loop / portTICK_PERIOD_MS);
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
