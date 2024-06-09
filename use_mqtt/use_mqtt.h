#ifndef _USE_MQTT_H
#define _USE_MQTT_H

#include <Arduino.h>
#include <WiFi.h>

//  https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MQTT_RECONNECT_CALLBACK_SIGNATURE std::function<void(My_PubSubClient& obj)> reconnect_callback
#endif

class My_PubSubClient : public PubSubClient {
private:
    Client* _client;
    char my_user[100];
    long int MQTT_FIFO = 0;
    long lastReconnectAttempt = 0;
    MQTT_RECONNECT_CALLBACK_SIGNATURE;

public:
    My_PubSubClient& setReconnectCallback(MQTT_RECONNECT_CALLBACK_SIGNATURE);
    My_PubSubClient(Client& client);
    ~My_PubSubClient();

    bool _publish(const char* payload, const char* message, bool LOG = true);
    void _publish_saved(long idx);
    void _setUser(const char* _usr);
    void _TaskReconnect();
    bool _reconnect();

    static void TaskReconnect(void* pvParameters);
    static void _my_reconnect_callback(My_PubSubClient& obj);
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // _USE_MQTT_H
