#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SimpleDHT.h>

char ssid[] = "Galaxy A21s9060";
char password[] = "rmcx1237";

String url = "http://api.thingspeak.com/update?api_key=XRQ9UNHNAB8U0JBK";
int pinDHT11 = 2;

SimpleDHT11 dht11(pinDHT11);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Linking...");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(1000);
  }

  Serial.println("Complete");
}

void loop() {

  delay(1000);
  // put your main code here, to run repeatedly:
  Serial.println("Core No.:");

  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;
  if((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.println("Temp ERR, sign=");
    Serial.println(err);
    delay(1000);
    return;
  }

  Serial.println("Success!");
  Serial.print((int)temperature);Serial.print(" *C, ");
  Serial.print((int)humidity);Serial.println(" H");
  
  //delay(1000);
  Serial.println("Starting Web");
  HTTPClient http;
  String url1 = url + "&field1=" + (int)temperature + "&field2=" + (int)humidity;
  http.begin(url1);

  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.print("Web Content:");
    Serial.println(payload);
  }else {
    Serial.println("Web Error");
  }
  http.end();
  delay(20000);
 
}
