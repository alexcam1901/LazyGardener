#define RELAY1_PIN 12
#define RELAY2_PIN 27
#define RELAY3_PIN 15
#define RELAY4_PIN 13
#define RELAY5_PIN 14
#define RELAY6_PIN 26
#define RELAY_24V_PIN 25

#define LED1_PIN 19
#define LED2_PIN 23
#define LED3_PIN 18
#define LED4_PIN 5
#define LED5_PIN 17
#define LED6_PIN 16
#define LED_INFO_PIN 22

const byte NO_OF_RELAYS  = 6;

const byte relay[NO_OF_RELAYS] = {RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN, RELAY5_PIN, RELAY6_PIN};
const byte led[NO_OF_RELAYS] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN, LED5_PIN, LED6_PIN};
