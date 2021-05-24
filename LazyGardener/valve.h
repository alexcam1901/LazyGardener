bool set_valve(int valve, bool state) {
    if (valve > -1 && valve <= NO_OF_RELAYS) {
        snprintf(sbuf, sizeof(sbuf), "[set_valve] State: %u\n", state);
        telnetSerial(sbuf);

        digitalWrite(RELAY_24V_PIN, state);
        delay(5);

        if (state && relay_on > -1) {  // only one can be on at the time, if another is already on, turn it off
            snprintf(sbuf, sizeof(sbuf), "[set_valve] Turning valve %i OFF\n", relay_on);
            telnetSerial(sbuf);

            digitalWrite(relay[relay_on], VALVE_OFF);
            digitalWrite(led[relay_on], VALVE_OFF);
            send_state(relay_on, false);
        }

        relay_timeout = default_relay_timeout[valve];

        if (state) relay_last_on = millis();

        snprintf(sbuf, sizeof(sbuf), "[set_valve] Turning valve %i to: %s", valve, (state ? MQTT_STATE_ON : MQTT_STATE_OFF));
        telnetSerial(sbuf);

        if (state) {
            snprintf(sbuf, sizeof(sbuf), " for %lu ms = %lu min", relay_timeout, relay_timeout / 60000);
            telnetSerial(sbuf);
        }

        snprintf(sbuf, sizeof(sbuf), "\n");
        telnetSerial(sbuf);

        relay_on = (state ? valve : -1);
        snprintf(sbuf, sizeof(sbuf), "[set_valve] Relay_on State: %i\n", relay_on);
        telnetSerial(sbuf);
        
        digitalWrite(relay[valve], (state ? VALVE_ON : VALVE_OFF));
        digitalWrite(led[valve], state);

        send_state(valve, state);

        return true;
    }

    return false;
}

void failsafe() {
  for (byte i = 0; i < NO_OF_RELAYS; i++) {
      //snprintf(sbuf, sizeof(sbuf), "relay pin: %i\n failsafe_timeout: %i\n", digitalRead(relay[i]), failsafe_relay_timeout[i] );
      //telnetSerial(sbuf);
     
        if (digitalRead(relay[i]) == VALVE_ON && failsafe_relay_timeout[i] == 0) {
          snprintf(sbuf, sizeof(sbuf), "[failsafe] Adding valve %i to failsafe array\n", i);
          telnetSerial(sbuf);
          failsafe_relay_timeout[i] = millis();
        }

        if (digitalRead(relay[i]) == VALVE_OFF && failsafe_relay_timeout[i] != 0) {
          snprintf(sbuf, sizeof(sbuf), "[failsafe] Removing valve %i from failsafe array\n", i);
          telnetSerial(sbuf);
          failsafe_relay_timeout[i] = 0;
        }

        if (digitalRead(relay[i]) == VALVE_ON && millis() - failsafe_relay_timeout[i] > FAILSAFE_TIMEOUT) {
          snprintf(sbuf, sizeof(sbuf), "[failsafe] !! Failsafe triggered for valve %i !!\n", i);
          telnetSerial(sbuf);
          set_valve(i, false);
          digitalWrite(relay[i], VALVE_OFF);
        }
    }
}
