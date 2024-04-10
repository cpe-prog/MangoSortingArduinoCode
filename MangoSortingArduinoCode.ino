#include <Servo.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

Servo s1;
Servo s2;
Servo s3;
Servo s4;
Servo s5;


#define WIFI_SSID "I'm in!"
#define WIFI_PASSWORD "connected"
#define API_KEY ""
#define DATABASE_URL "" 


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool mangoStatus = false;

void setup() {
  Serial.begin(9600);
  s1.attach(D0);
  s2.attach(D1);
  s3.attach(D2);
  s4.attach(D3);
  s5.attach(D4);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
   while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
   Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}


void loop() {
  if (Firebase.ready() && signupOK && (millis() -  sendDataPrevMillis > 1000 || sendDataPrevMillis == 0 )) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.getBool(&fbdo, "mango/1/maturity")){
      if (fbdo.dataType() == "boolean"){
      mangoStatus = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + mangoStatus + "(" + fbdo.dataType() + ")");
        if (mangoStatus == true){
          Serial.print("Servo Running to 180 ");
          s1.write(180);
        }
        else{
          Serial.print("Servo Running to 0 ");
          s1.write(0);
        }
      }
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }
}
