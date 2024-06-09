#include "use_mqtt.h"

const char* TAG = "ExampleMQTT";

WiFiClient soc_l3;
My_PubSubClient client(soc_l3);

char my_user[20];
const char* ssid = "arroz_com_feijao_5";
const char* password = "Arrozcomfeijao2023";

void my_reconnect_callback(My_PubSubClient& obj){
    obj._publish("bf_test/out", "hello world");
    obj._publish("bf_test/id", my_user);
    obj.subscribe("bf_test/in");
}

void callback(char* topic, byte* payload, unsigned int length) {
  ESP_LOGI(TAG, "[%s] %s", topic, payload);
}

void setup() {
  sprintf(my_user,"my_esp_%uld",esp_random()/1000);

  Serial.begin(115200);
  ESP_LOGI(TAG, "Hi!");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) delay(200);

  ESP_LOGI(TAG, "Connected");
  ESP_LOGI(TAG, "my_user: %s",my_user);

  client.setServer("broker.emqx.io", 1883);
  client._setUser(my_user);
  client.setCallback(callback);
  client.setReconnectCallback(my_reconnect_callback);

  xTaskCreatePinnedToCore(
    My_PubSubClient::TaskReconnect,  // Función que se ejecutará
    "TaskCore1",                     // Nombre de la tarea
    20000,                           // Tamaño de la pila de la tarea (en palabras)
    (void*)&client,                  // Parámetro de la tarea
    1,                               // Prioridad de la tarea
    NULL,                            // Handle de la tarea
    1                                // Número del núcleo donde se ejecutará la tarea (0 o 1)
  );
}

void loop() {

  if (Serial.available()) {

    if (Serial.peek() == 'w') WiFi.begin(ssid, password);
    if (Serial.peek() == 'm') client._publish("bf_test/serial", "Hi");
    if (Serial.peek() == 'n') client._publish("bf_test/serial", "Hola Ninaa\nComo estas?");

    while (Serial.available()) {
      Serial.read();
    }
  }
}
