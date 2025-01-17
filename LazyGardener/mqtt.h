void mqttCallback(const MQTT::Publish& pub) {
  snprintf(sbuf, sizeof(sbuf), "[mqttCallback] Message arrived [%s] topic: %s\n", pub.payload_string().c_str(), pub.topic().c_str());
  telnetSerial(sbuf);

  String topic = pub.topic();

  if (topic == MQTT_RELAY_TOPIC_RELAY_STATE) {
    for (uint8_t i = 0; i < NO_OF_RELAYS; i++) {
      send_state(i, (relay_on == i ? 1 : 0));
    }

  } else if ( topic.substring(topic.length() - 3, topic.length()) == MQTT_RELAY_TOPIC_SET_APPENDIX) {

    uint8_t relay = topic.substring(topic.length() - 5, topic.length() - 4).toInt();

    if ( topic.charAt(topic.length() - 7) == '/') {
      relay = topic.substring(topic.length() - 6, topic.length() - 4).toInt();
    }

    String state = pub.payload_string();

    //snprintf(sbuf, sizeof(sbuf), "[mqttCallback] State: %s\n", state);
    //telnetSerial(sbuf);

    if (state == MQTT_STATE_ON) {
      //snprintf(sbuf, sizeof(sbuf), "[mqttCallback state_on] State: %s\n", state);
      //telnetSerial(sbuf);
      set_valve(relay, true);
    }

    else if (state == MQTT_STATE_OFF) {
      //snprintf(sbuf, sizeof(sbuf), "[mqttCallback state_off] State: %s\n", state);
      //telnetSerial(sbuf);
      set_valve(relay, false);
    }
  }
}

bool send_state(int valve, bool state) {
  char buffer[20];
  String topic = MQTT_RELAY_TOPIC_RELAY_STATE;
  topic.concat("/");
  topic.concat(valve);

  snprintf(sbuf, sizeof(sbuf), "[send_state] Sending MQTT msg, valve %i - state: %s, on topic: %s\n", valve, (state ? MQTT_STATE_ON : MQTT_STATE_OFF), topic.c_str());
  telnetSerial(sbuf);

  bool ok = mqtt.publish(MQTT::Publish(topic, (state ? MQTT_STATE_ON : MQTT_STATE_OFF)).set_retain().set_qos(MQTT_QOS));

  if (ok) {
    snprintf(sbuf, sizeof(sbuf), "[send_state] Sending state OK\n");

  } else {
    snprintf(sbuf, sizeof(sbuf), "[send_state] Sending state failed\n");
  }

  telnetSerial(sbuf);

  return ok;
}

bool sendStatus() {
  bool ok = mqtt.publish(MQTT::Publish(MQTT_STATUS_TOPIC, MQTT_STATUS_ALIVE).set_retain().set_qos(MQTT_QOS));

  if (ok) {
    ok = mqtt.publish(MQTT::Publish(MQTT_IP_TOPIC, WiFi.localIP().toString().c_str()).set_retain().set_qos(MQTT_QOS));
  }

  if (ok) {
    char buf[5];
    snprintf(buf, sizeof(buf), "%i", WiFi.RSSI());
    ok = mqtt.publish(MQTT::Publish(MQTT_RSSI_TOPIC, buf)
                      .set_retain()
                      .set_qos(MQTT_QOS)
                     );
  }

  if (ok) {
    snprintf(sbuf, sizeof(sbuf), "Sending status OK\n");

  } else {
    snprintf(sbuf, sizeof(sbuf), "Sending status Failed\n");
  }

  telnetSerial(sbuf);

  return ok;
}

void setupMqtt() {
  mqtt.set_callback(mqttCallback);
  blink_enabled = true;
}

bool mqttReconnect() {

  static uint32_t lastReconnectAttempt = 0;
  static uint32_t firstReconnectAttempt = 0;

  if (!mqtt.connected()) {
    blink_enabled = true;
    uint32_t now = millis();

    if (now - firstReconnectAttempt > MQTT_CONNECTION_TIMEOUT) {
      snprintf(sbuf, sizeof(sbuf), "Max MQTT reconnect timeout reached, resetting.\n");
      telnetSerial(sbuf);
      ESP.restart();
    }

    if (now - lastReconnectAttempt > CONNECTION_INTERVAL) {
      if (firstReconnectAttempt == 0) {
        firstReconnectAttempt == now;
      }
      
      snprintf(sbuf, sizeof(sbuf), "Connecting to MQTT: ");
      telnetSerial(sbuf);
      lastReconnectAttempt = now;

      if (mqtt.connect(MQTT::Connect(DEVICE_NAME)
                       .set_will(MQTT_STATUS_TOPIC, MQTT_STATUS_DEAD, MQTT_QOS, true))) {
        String topic = MQTT_RELAY_TOPIC_RELAY_STATE;
        topic.concat("/+/");
        topic.concat(MQTT_RELAY_TOPIC_SET_APPENDIX);
        mqtt.subscribe(MQTT::Subscribe()
                       .add_topic(MQTT_RELAY_TOPIC_RELAY_STATE, MQTT_QOS)
                       .add_topic(MQTT_RELAY_TOPIC_SET_APPENDIX)
                       .add_topic(topic));

        lastReconnectAttempt = 0;
        firstReconnectAttempt = 0;
        
        snprintf(sbuf, sizeof(sbuf), "OK\n");
        telnetSerial(sbuf);

        sendStatus();

        // send state on reconnect
        for (unsigned int i = 0; i < NO_OF_RELAYS; i++) {
          send_state(i, (relay_on == i ? 1 : 0));
        }

      } else {
        snprintf(sbuf, sizeof(sbuf), "Failed\n");
        telnetSerial(sbuf);
        return false;
      }

      return mqtt.connected();
    }
  }

  blink_enabled = false;
  digitalWrite(LED_INFO_PIN, HIGH);
  return true;
}
