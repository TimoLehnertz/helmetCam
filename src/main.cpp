#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

#define HELMET

#define AIR_UNIT 2

struct Message;

void beginESPNow();
void send(Message* m);

u8 broadcastAddress[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

struct Message {
  bool on = false;
};

Message m;


void setup() {
  // Serial.begin(115200);
  // #ifdef HELMET
  pinMode(AIR_UNIT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  // #endif
  beginESPNow();
}

uint32 lastSend = 0;

void loop() {
  #ifndef HELMET
  if(millis() - lastSend > (m.on ?  2000 : 1000)) {
    m.on = !m.on;
    digitalWrite(AIR_UNIT, m.on);
    digitalWrite(LED_BUILTIN, m.on);
    Serial.println(m.on);
    send(&m);
    lastSend = millis();
  }
  #endif
}

void send(Message* m) {
  esp_now_send(NULL, (u8*) m, sizeof(Message)); // NULL means send to all peers
}

void onSent(uint8_t *mac_addr, uint8_t sendStatus) {
  // Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    // Serial.println("Delivery success");
  }
  else{
    // Serial.println("Delivery fail");
  }
}

void onReceive(u8 *mac_addr, u8 *data, u8 len) {
  // Serial.print("<<");
  // Serial.write(data, len);
  // Serial.println();
  if(len >= 1) {
    digitalWrite(AIR_UNIT, data[0]);
  }
}


void beginESPNow() {
  WiFi.mode(WIFI_STA); // Station mode for esp-now controller
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    // Serial.println("ESP_Now init failed...");
    delay(100);
    ESP.restart();
  }
  
  #ifdef HELMET
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onReceive);
  #else
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(onSent);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  #endif
}