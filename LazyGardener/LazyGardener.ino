

//
//Board: Node32s
//

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <PubSubClient_JSON.h>
#include <MQTT.h>

#include "ESPmDNS.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include "pins.h"
#include "setting.h"
#include "fn.h"
#include "network.h"
#include "mqtt.h"
#include "valve.h"

AsyncWebServer server(80);

void setup() {
#if defined(DEBUG_SERIAL)
    Serial.begin(115200);
#endif

    snprintf(sbuf, sizeof(sbuf), "\n\nBooting %s\n", DEVICE_NAME);
    telnetSerial(sbuf);

    pinMode(RELAY_24V_PIN, OUTPUT);
    pinMode(LED_INFO_PIN, OUTPUT);
    digitalWrite(RELAY_24V_PIN, LOW);
    digitalWrite(LED_INFO_PIN, LOW);

    for (byte i = 0; i < NO_OF_RELAYS; i++) {
        pinMode(relay[i], OUTPUT);
        pinMode(led[i], OUTPUT);
        delay(1);
        digitalWrite(relay[i], VALVE_OFF);
        digitalWrite(led[i], LOW);
    }

    if (wifiReconnect()) {
        snprintf(sbuf, sizeof(sbuf), "Setting up MQTT\n");
        telnetSerial(sbuf);
        setupMqtt();
    }
    else {
        ESP.restart();
    }

    setupBlinker();
    snprintf(sbuf, sizeof(sbuf), "Message: [%s]\n", MQTT_RELAY_TOPIC_RELAY_STATE);
    telnetSerial(sbuf);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hi! I control irrigation.");
    });
    
    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    snprintf(sbuf, sizeof(sbuf), "HTTP server started\n");
    telnetSerial(sbuf);
}

void loop() {
    if (valve_on() > -1) {
        if (millis() - relay_last_on >= relay_timeout) {
          snprintf(sbuf, sizeof(sbuf), "Valve %i timeout reached.\n", valve_on());
          telnetSerial(sbuf);
          set_valve(valve_on(), false);
        }
    }

    failsafe();
        
    if (wifiReconnect()) {
        //ArduinoOTA.handle();
        AsyncElegantOTA.loop();

        telnetLoop();
        mqtt_connected = mqttReconnect();

        if (mqtt_connected) {
            mqtt.loop();

            if (millis() - last_status >= MQTT_STATUS_INTERVAL) {
                last_status = millis();
                sendStatus();
            }
        }

    } else {
        digitalWrite(LED_INFO_PIN, LOW);
    }
}
