bool set_valve(int valve, bool state) {
    if (valve > -1 && valve <= NO_OF_RELAYS) {
        snprintf(sbuf, sizeof(sbuf), "Turning 24V to: %u\n", !state);
        telnetSerial(sbuf);

        digitalWrite(RELAY_24V_PIN, !state);
        delay(5);

        if (!state && relay_on > -1) {  // only one can be on at the time, if another is already on, turn it off
            snprintf(sbuf, sizeof(sbuf), "Turning valve %i OFF\n", relay_on);
            telnetSerial(sbuf);

            digitalWrite(relay[relay_on], HIGH);
            digitalWrite(led[relay_on], HIGH);
            send_state(relay_on, false);
        }

        relay_timeout = default_relay_timeout[valve];

        if (!state) relay_last_on = millis();


        snprintf(sbuf, sizeof(sbuf), "Turning valve %i to: %u", valve, state);
        telnetSerial(sbuf);



        if (!state) {
            snprintf(sbuf, sizeof(sbuf), " for %lu ms = %lu min", relay_timeout, relay_timeout / 60000);
            telnetSerial(sbuf);
        }

        snprintf(sbuf, sizeof(sbuf), "\n");
        telnetSerial(sbuf);


        relay_on = (state ? -1 : valve);
        snprintf(sbuf, sizeof(sbuf), "Relay_on State: %i\n", relay_on);
        telnetSerial(sbuf);
        
        digitalWrite(relay[valve], state);
        digitalWrite(led[valve], state);

        send_state(valve, state);

        return true;
    }

    return false;
}
