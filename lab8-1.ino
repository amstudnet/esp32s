#include<ESP8266WiFi.h>

const char* ssid = "Galaxy A21s9060";
const char* password = "rmcx1237";

WiFiServer server(80);

String header;
String outputState = "off";

const int output2 = 2;

void setup(){
  Serial.begin(115200);
  pinMode(output2,OUTPUT);
  digitalWrite(output2,LOW);

  Serial.print("Connecting to");
  Serial.print(ssid);
  WiFi.begin(ssid,password);

  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFiconnected");
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());
  server.begin();
}



void loop(){
  Serial.println(WiFi.localIP());
  WiFiClient client = server.available(); //等待 clients 連線
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
  
  
  
            if (header.indexOf("GET /2/on") >= 0){
              Serial.println("GPIO 2 on");
              outputState = "on";
              digitalWrite(output2, HIGH);
            }
            else if (header.indexOf("GET /2/off") >= 0){
              Serial.println("GPIO 2 off");
              outputState="off";
              digitalWrite(output2, LOW);
            }
    
            client.println("<html>");
            client.println("<head>");
            client.println("<link rel=\"icon\" href=\"data:,\">");
        
            // 設定 on/off 按鈕的CSS
            client.println("<style>html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #19586A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            client.println("<body><h1>No.10</html>");
            client.println("<p>GPIO 2-State " + outputState + "</p>");

            if (outputState=="off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            //使用空白行結束HTTP回應
            client.println();
            break;
        }else{
          currentLine="";
        }
      }else if(c!='\r'){
          currentLine += c;
      }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnect");
    Serial.println("");
  }
}
