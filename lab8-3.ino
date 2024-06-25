#include <ESP8266WiFi.h>
#include <DHT.h>
#include <TridentTD_LineNotify.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define LINE_TOKEN "vtP1BzFqVs0V6Kbscb48pKFZ8YvYhXvYY5e3xboN4Ss"
long previousTime = 0;
long interval = 2000;
int humidity = 0;
int temperature = 0;
const char* MY_SSID = "Galaxy A21s9060";
const char* MY_PWD = "rmcx1237";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  connectWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentTime = millis();
  if(currentTime - previousTime > interval) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    LINE.setToken(LINE_TOKEN);
    LINE.notify((String)(h) + "H, " + (String)(t) + "*C");
    previousTime = currentTime;
  }
}

void connectWiFi() {
  WiFi.begin(MY_SSID, MY_PWD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("ccccc...");
  }
}
