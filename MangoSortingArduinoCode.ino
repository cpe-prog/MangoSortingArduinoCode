#include <Servo.h>
#include <HX711_ADC.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

HX711_ADC LoadCell(7, 8); 

Servo s1;
Servo s2;
Servo s3;
Servo s4;
Servo s5;

String lastCategory = "";
unsigned long categoryStartTime = 0;
const unsigned long categoryStableTime = 1000;

#define WIFI_SSID "I'm in!"
#define WIFI_PASSWORD "connected"
#define API_KEY "api key"
#define DATABASE_URL "firebase url" 


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
  LoadCell.begin(); // start connection to HX711
  LoadCell.start(2000); // load cells gets 2000ms of time to stabilize
  LoadCell.setCalFactor(1000.0); // calibration factor for load cell => dependent on your individual setup
  Serial.println("Scale is ready");
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
  LoadCell.update();
  float weight = LoadCell.getData(); 
  String currentCategory = "";
  Serial.println(weight);
  if (weight >= 180 && weight <= 249) {
    currentCategory = "small";
  } else if (weight >= 250 && weight <= 349) {
    currentCategory = "medium";
  } else if (weight >= 350 && weight <= 400) {
    currentCategory = "large";
  }
  if (currentCategory != lastCategory) {
    lastCategory = currentCategory;
    categoryStartTime = millis(); 
  } else if (currentCategory != "" && millis() - categoryStartTime >= categoryStableTime) {
    Serial.println(currentCategory);
    LoadCell.tare();
  }

  if (Firebase.ready() && signupOK && (millis() -  sendDataPrevMillis > 1000 || sendDataPrevMillis == 0 )) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getBool(&fbdo, "mango/1/maturity")){
      if (fbdo.dataType() == "boolean"){
      mangoStatus = fbdo.boolData();
      Serial.println("Seccess: " + fbdo.dataPath() + ": " + mangoStatus + "(" + fbdo.dataType() + ")");
        if (mangoStatus == true){
          Serial.print("Servo Running to 180 ");
          s1.write(180);
          if (currentCategory = "medium"){// adding condition which if current category is equal to small, medium or large which the servo for ripe will be working depend for medium and large
            s2.write(180);
          } 
        }
        else{
          Serial.print("Servo Running to 0 "); 
          s1.write(0);
          // adding condition which if current category is equal to small, medium or large which the servo for raw will be working depend for medium and large
        }
      }
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }



}
