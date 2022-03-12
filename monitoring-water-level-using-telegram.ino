#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "TDN200";
const char* password = "isptdn200@@";

// Initialize Telegram BOT
#define BOTtoken "1824715191:AAGN4FLGTsSXvcWZvGZBRIcmQgSofoTjBwQ"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "975834352"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

#define trigPin 5
#define echoPin 4
int buzzer = 14;
const int RELAY_PIN = 13; 

long durasi;
int cm, wl;

int pumpStatus;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(buzzer, LOW);
  digitalWrite(RELAY_PIN, LOW);
  
  pinMode(buzzer,OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}
  
void loop() { 
  digitalWrite (trigPin, 0);
  delayMicroseconds(2);
  digitalWrite (trigPin, 1);
  delayMicroseconds(10);
  digitalWrite (trigPin, 0);
  delayMicroseconds(2);

  durasi = pulseIn(echoPin, HIGH);
  cm = (durasi * 0.0343) / 2;
  wl = 21 - cm; // 21 didapat dari pembacaan antara jarak sensor dengan dasar air
  Serial.print("Distance : ");
  Serial.print(cm);
  Serial.print(" cm");
  
  Serial.print("\t");
  Serial.print("Water level : ");
  Serial.print(wl);
  Serial.println(" cm");
  
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  } 
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/sensor_status to request current sensor status. \n";
      welcome += "/pump_status to request current pump status. \n";
      welcome += "/waterlevel_status to request current water level status. \n";
      welcome += "/pump_on to turn pump ON. \n";
      welcome += "/pump_off to turn pump OFF. \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    else if (text == "/sensor_status") {
      if (cm > 0) {
        bot.sendMessage(chat_id, "Sensor is ON", "");
        bot.sendMessage(chat_id, "Water level : " + String(wl)+ " cm", "");
        Serial.println("Sensor is ON");
      }
      else {
        bot.sendMessage(chat_id, "Sensor is OFF", "");
        Serial.println("Sensor is OFF");
      }
    }  

    else if (text == "/waterlevel_status") {
      if (wl > 18) {
        bot.sendMessage(chat_id, "Water level : " + String(wl)+ " cm", "");
        bot.sendMessage(chat_id, "Water tank is full. Pump is OFF", "");
        Serial.println("Water tank is  full. Pump is OFF");
        
        pumpStatus = 0;
        digitalWrite(RELAY_PIN, LOW);
      }
      else {
        bot.sendMessage(chat_id, "Water level : " + String(wl)+ " cm", "");
      }
    }  

    if (text == "/pump_on") {
      bot.sendMessage(chat_id, "Water tank is empty. Pump is ON", "");
      Serial.println("Water tank is empty. Pump is ON");
      pumpStatus = 1;
      digitalWrite(RELAY_PIN, HIGH);
    }

    if (text == "/pump_off") {
      bot.sendMessage(chat_id, "Water tank is full. Pump is OFF", "");
      Serial.println("Water tank is  full. Pump is OFF");
      
      pumpStatus = 0;
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(buzzer,HIGH);
      delay(1000);
      digitalWrite(buzzer,LOW);
    }
  
    else if (text == "/pump_status") {
      if (pumpStatus) {
        bot.sendMessage(chat_id, "Pump is ON", "");
      }
      else {
        bot.sendMessage(chat_id, "Pump is OFF", "");
      }
    }
   yield();
 }
 delay(1000);
}
