#include <Arduino.h>      //arduino基本功能
#include <ArduinoJson.h>  //解析json
#include <HTTPClient.h>   //訪問render
#include <WiFi.h>         //WIFI聯網功能
#include <SPI.h>          //RFID
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
 long count_3s;
 long count_5s;
template <typename T>
class Queue {
    struct Node {
        T room_number;
        int menu[10];
        int UID[4];
        Node *next;
        long waiting_time;
    };
    Node *head;
    Node *tail;
    int qsize;
    long start_time;
 
   public:
    Queue() {
        head = NULL;
        tail = NULL;
        qsize = 0;
    }
 
    int size() {
        return qsize;
    }
 
    bool empty() {
        if (qsize == 0) {
            return true;
        } else {
            return false;
        }
    }
    void put(int (&menu)[10], const T &room_number, int (&UID)[4]) {
        Node *newNode = new Node;
        if (qsize) {
            tail->next = newNode;
            newNode->room_number = room_number;
            for (int i = 0; i < 10; i++)
                newNode->menu[i] = menu[i];
            for (int i = 0; i < 4; i++)
                newNode->UID[i] = UID[i];
            newNode->waiting_time = millis();
            newNode->next = NULL;
            tail = newNode;
        } else {
            head = tail = newNode;
            newNode->room_number = room_number;
            for (int i = 0; i < 10; i++)
                newNode->menu[i] = menu[i];
            for (int i = 0; i < 4; i++)
                newNode->UID[i] = UID[i];
            newNode->waiting_time = millis();
            newNode->next = NULL;
        }
        qsize++;
    }
 
    void delete_node() {
        Node *temp;
 
        if (empty()) {
            printf("queue is empty\n");
        } else {
            temp = head;
            head = head->next;
            delete temp;
            qsize--;
        }
    }
 
    long read_time() {
        return millis() - head->waiting_time;
    }
 
    int menu(int value) {
        if (empty()) {
            printf("empty menu\n");
            return -1;
        } else if (value < 0 && value > 10) {
            printf("invalid menu number\n");
            return -1;
        } else {
            return head->menu[value];
        }
    }
 
    int uid(int value) {
        if (empty()) {
            printf("empty uid\n");
            return -1;
 
        } else if (value < 0 && value > 4) {
            printf("invalid uid number\n");
            return -1;
        } else {
            return head->UID[value];
        }
    }
 
    char room_number() {
        if (empty()) {
            printf("iempty room_number\n");
            return 'a';  //error messenage
        } else {
            return head->room_number;
        }
    }
 
    void destroyQueue() {
        while (!empty()) {
            printf("delete\n");
            delete_node();
        }
    }
 
    ~Queue() {
        destroyQueue();
    }
};
 
// WIFI
const char ssid[] = "V1930";      //修改為你家的WiFi網路名稱
const char pwd[] = "20022002";  //修改為你家的WiFi密碼
 
//lcd
LiquidCrystal_I2C lcd(0x27,16,2);
void lcd_wifi();
void lcd_empty();
void lcd_cook();
void lcd_deliver();
void lcd_cancel();
int joystick_state = 0;
String lcd_stirng ="";
int rfid_err = 0;
void lcd_rfid_inti();
 
// queue for starving customer
Queue<char> queue;

// button
bool enter;
bool done;
bool get_order;
 
// HTTP client (accces 雲端)
String domain = "https://0301-1-174-12-172.ngrok-free.app";//自己的ngrok
String update_api = "/Update";
String order_api = "/Order";
 
HTTPClient http;
bool update_render();
void update_order();
void notice_status(String API);
long render_interval = 0;

int vrx, vry, sw;
 
//status
int state = 0;
void state_for_cook();
void state_to_room();
void arrive_announce();
void state_for_RFID();
void state_for_get_order();
void state_go_back();


// byte uid[] = {0x44, 0xE7, 0x02, 0x28};
//這是我們指定的卡片UID，可由讀取UID的程式取得特定卡片的UID，再修改這行
 
void setup() {
 
  // Serial port
  Serial.begin(115200);
  Serial.println();
  Serial.println("-----------");
 
  //lcd
  lcd.init();
  lcd.backlight();
 
  // button
    pinMode(15, INPUT);
   // pinMode(6, INPUT);
    Serial.println("button OK!");


  //WIFI
    WiFi.mode(WIFI_STA);  //設置WiFi模式
    WiFi.begin(ssid, pwd);
    Serial.print("WiFi connecting");
    //當WiFi連線時會回傳WL_CONNECTED，因此跳出迴圈時代表已成功連線
    while (WiFi.status() != WL_CONNECTED) {
        //Serial.print(".");
        //oled_wifi();
        delay(100);
        //if ((millis() % 7000) < 150)myDFPlayer.playLargeFolder(1, 2);  // 0002請確認無線網路連線.wav
    }
 
    Serial.println("");
    Serial.print("IP位址:");
    Serial.println(WiFi.localIP());  //讀取IP位址
    Serial.print("WiFi RSSI:");
    Serial.println(WiFi.RSSI());  //讀取WiFi強度
    Serial.println("WIFI Connection OK!");
    Serial.println("-----------");
 
    /*
    // RFID
    SPI.begin();
    mfrc522.PCD_Init(SS_PIN, RST_PIN);  // 初始化MFRC522卡
    Serial.print("Reader : ");
    mfrc522.PCD_DumpVersionToSerial();  // 顯示讀卡設備的版本
    Serial.println("RFID reader OK!");
    Serial.println("-----------");
    */

}
/*
 
// read data from Joystick
    int vrx, vry, sw;
    sw = analogRead(36);
    vrx = analogRead(39);
    vry = analogRead(34);
*/
void loop() {
  if (queue.empty() && millis() - render_interval > 3000) {  // 沒訂單時，3秒更新一次
       render_interval = millis();
        if (update_render())
            update_order();
    }
  // http client to render
  
 
      // read data from server
    // server.handleClient();
    switch (state) {
        case 0:
            state_for_cook();
            break;
        case 1:
            state_to_room();
            rfid_err = 0;
            break;
        case 2:
            //arrive_announce();
            state=3;
            break;
        case 3:
            state_for_RFID();
            break;
        case 4:
            state_for_get_order();
            break;
        case 5:
            state_go_back();
            break;
        default:
            break;
    }
   
     
 
}
// state == 0
void state_for_cook() {
    // read data from Joystick
    
    sw = analogRead(36);
    vrx = analogRead(39);
    vry = analogRead(34);
    //    printf("VRx=%d, VRy=%d, SW=%d\n", vrx, vry, sw);
 
    // analysis data from Joystick
    if (sw>3000)
        joystick_state = 0;
    else if (sw<1500)
        joystick_state = 1;
    
    //lcd
    /*if (queue.empty())
        lcd_empty();
    else
        lcd_cook();*/
 
    // bottom from button 15
    enter = digitalRead(15);
    Serial.println(enter);
      lcd.clear();
      lcd.noBlink();
       lcd.setCursor(0, 1);
      lcd.print("deliver");
      lcd.setCursor(0, 0);
      lcd.print("cancel");


      if( joystick_state == 1){
        lcd.clear();
        lcd.setCursor(14, 0);
        lcd.print("x");
      }
      else if(joystick_state == 0){
        lcd.clear();
        lcd.setCursor(14, 1);
        lcd.print("o");
      }
    // OLED選擇deliver，並按下按鈕
    //if (!queue.empty() && enter && joystick_state == 2)
    if (!queue.empty() &&enter && joystick_state == 0) {//1
        Serial.println("deliver confirm");
      
 
        count_5s = millis();
        while (millis() - count_5s < 5000) {  //5s
            lcd_deliver(count_5s);
            lcd_deliver();
            delay(50);
        }
        state = 1;
        //SerialBT.write(queue.room_number());  //車輛移動到h的位置
        notice_status("/MoveOn");             // notice line
    
        return;
    }
 
    // OLED選擇cancel，並按下按鈕
    //if (!queue.empty() && enter && joystick_state == 1)
    if (!queue.empty()&& enter && joystick_state == 1) {
        Serial.println("cancel confirm");
        count_3s = millis();
        notice_status("/Canceled");           // notice line
       
        while (millis() - count_3s < 3000) {  //5s
           lcd_cancel();
           delay(50);
        }
        queue.delete_node();
        return;
    }
}
// state == 1
void state_to_room() {
    
      state=2;
      notice_status("/Arrived");         // notice line
      //order_name = 0;
      /*
    //收到uno板的藍芽資訊，代表移動任務完成
    if (SerialBT.available()) {
        //char completed = SerialBT.read();
        //printf("completed 1 : %c\n", completed);
        Serial.write(completed);
        if (completed == '1') {
            state = 2;
            
            notice_status("/Arrived");         // notice line
            order_name = 0;
            return;
        }
    }*/
}
// state == 3
void state_for_RFID() {
    // 20 second -> timeout
    /*if (millis() - count_3s > 60000) {
        notice_status("/Timeout");         // notice line
        //myDFPlayer.playLargeFolder(5, 4);  // 0004逾時未完成交易手續，車輛將離開，此筆訂單不成立.wav
        state = 5;
        //SerialBT.write('3');
        //SerialBT.write('2');  //車輛移動到起始位置
        return;
    }*/

    // 顧客沒逼卡，就拿走東西
    /*
    weight = Get_Weight();  //更新重量值
    if (weight < 20) {
        notice_status("/Accidient");               // notice line
        Serial.println("顧客沒逼卡，就拿走東西");  //error
        myDFPlayer.playLargeFolder(5, 8);          //0008請先完成付款手續.wav
        long count4s = millis();
        while (1) {
            //5秒後撥放警報聲
            if (myDFPlayer.readState() == 0 && abs(millis() - count4s) > 5000)
                myDFPlayer.playLargeFolder(5, 5);  // 0005alarm.mp3

            weight = Get_Weight();  //更新重量值
            oled_weight();
            // 重量恢復正常
            if (abs(weight - ideal_weight) < 50) {
                myDFPlayer.pause();                //pause the mp3
                myDFPlayer.playLargeFolder(5, 2);  // 0002請將房卡放置於感應處，確認交易.wav
                return;
            }
        }
    }*/

    /*// OLED
    if (!rfid_err)
        oled_rfid_init();
    else
        oled_rfid_err();

    // RFID
    // 檢查是不是偵測到新的卡
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        // 顯示卡片的UID
        Serial.print("Card UID:");
        dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);  // 顯示卡片的UID
        Serial.println();
        Serial.print("PICC type: ");
        MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
        Serial.println(mfrc522.PICC_GetTypeName(piccType));  //顯示卡片的類型

        // 從queue中抓取目前的UID
        byte uid[] = {queue.uid(0), queue.uid(1), queue.uid(2), queue.uid(3)};

        //把取得的UID，拿來比對我們指定好的UID
        bool they_match = true;        // 初始值是假設為真
        for (int i = 0; i < 4; i++) {  // 卡片UID為4段，分別做比對
            if (uid[i] != mfrc522.uid.uidByte[i]) {
                they_match = false;  // 如果任何一個比對不正確，they_match就為false，然後就結束比對
                break;
            }
        }
       
        //在監控視窗中顯示比對的結果
        if (they_match) {
            Serial.println("Access Granted!");
            Serial.println("--------------");
            //myDFPlayer.playLargeFolder(5, 3);  // 0003請拿取餐點，祝您用餐愉快!.wav
            //SerialBT.write('3');               //傳3，開蓋
            state = 4;
            return;
        } else {
            Serial.println("Access Denied!");
            Serial.println("--------------");
            rfid_err++;
           // myDFPlayer.playLargeFolder(5, 7);  // 0007請使用正確的房卡感應.wav
            notice_status("/Rfid");            // notice line
        }
       // mfrc522.PICC_HaltA();  // 卡片進入停止模式
    }

    // RFID錯誤超過3次
    if (rfid_err == 3) {
        //myDFPlayer.playLargeFolder(5, 6);  // 0006RFID錯誤超過三次，車輛將離開，此筆訂單不成立
        state = 5;
        //SerialBT.write('3');  //車輛移動到起始位置
        //SerialBT.write('2');  //車輛移動到起始位置
        return;
    }*/
    state=4;
}
// 把讀取到的UID，用16進位顯示出來
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
// state == 4
void state_for_get_order() {
   // oled_get_order();
    //weight = Get_Weight();
    // 20 second -> timeout
    long count =millis();
    if (millis() - count > 60000) {
        notice_status("/Timeout");         // notice line
        //myDFPlayer.playLargeFolder(5, 4);  // 0004逾時未完成交易手續，車輛將離開，此筆訂單不成立.wav
        state = 5;
        //SerialBT.write('3');
        //SerialBT.write('2');  //車輛移動到起始位置
        return;
    }
    //get_order= digitalRead(6);
    //Serial.println(get_order);
    enter = digitalRead(15);
    if(enter) {
      notice_status("/Completed");  // notice line
      state = 5;
    }

    /*
    if (get_order) {
        notice_status("/Completed");  // notice line
        state = 5;
        //SerialBT.write('2');  //車輛移動到起始位置
        return;
    }
    */
}
// state == 5
void state_go_back() {
    // when music isn't playing
    //if (myDFPlayer.readState() == 0) {
        //myDFPlayer.playLargeFolder(2, 9);  // 0009uruha_rushia_bgm.mp3
    //}

    lcd_go_back();
    //收到uno板的藍芽資訊 for test
    //weight = Get_Weight();
    /*if (weight > 1500) {
        state = 0;
        myDFPlayer.pause();                //pause the mp3
        myDFPlayer.playLargeFolder(5, 9);  // 0009任務完成，繼續下一筆訂單.wav
        queue.delete_node();
        return;
    }*/

    //收到uno板的藍芽資訊，代表移動任務完成
    /*if (SerialBT.available()) {
        char completed = SerialBT.read();
        //printf("completed 2 : %c\n", completed);
        Serial.write(completed);
        if (completed == '1') {
            state = 0;
            // myDFPlayer.pause();                //pause the mp3
            //myDFPlayer.playLargeFolder(5, 9);  // 0009任務完成，繼續下一筆訂單.wav
            queue.delete_node();
            return;
        }
    }*/
    state = 0;
    queue.delete_node();
}/*
void oled_deliver(long count_5s) {
    // Clear the display first
    lcd.clear();
   
    // Row 1 - Status
    lcd.setCursor(0, 0);
    lcd.print("Start in");
   
    // Row 2 - Waiting time
    lcd.setCursor(9, 0);  // Adjust position for waiting time
    String oled_string = String(5 - (millis() - count_5s) / 1000);
    lcd.print(oled_string);
 
    // Row 3 - Progress bar
    int progress = (millis() - count_5s) * 127 / 5000;  // 0 to 127
    int numChars = progress / 8;  // Assuming each character represents 1/8th of the total width
    lcd.setCursor(0, 1);  // Position for the progress bar
    for (int i = 0; i < numChars; i++) {
        lcd.write(byte(255));  // Full block character
    }
    for (int i = numChars; i < 16; i++) {
        lcd.print(" ");  // Empty space for the rest of the bar
    }
}
void oled_cancel() {
    // Clear the display first
    lcd.clear();
   
    // Row 1 - State
    lcd.setCursor(0, 0);
    lcd.print("This order");
   
    // Row 2 - Notice
    lcd.setCursor(0, 1);
    lcd.print("is being canceled!");
   
    // Row 3 - Progress bar
    int progress = (millis() - count_3s) * 127 / 3000;  // 0 to 127 for 3 seconds
    int numChars = progress / 8;  // Each character represents 1/8th of the total width
    lcd.setCursor(0, 1);  // Position for the progress bar
    for (int i = 0; i < numChars; i++) {
        lcd.write(byte(255));  // Full block character
    }
    for (int i = numChars; i < 16; i++) {
        lcd.print(" ");  // Empty space for the rest of the bar
    }
}*/
// Update
bool update_render() {
    String url = domain + update_api;
    String payload = "";
 
    if ((WiFi.status() == WL_CONNECTED)) {
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
            payload = http.getString();
            //Serial.printf("回應本體：%s\n", payload.c_str());
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
 
        http.end();
        if (payload[0] == '1') {
            return true;
        } else {
            return false;
        }
    }
}
//傳api
void notice_status(String API) {
    String url = domain + API;
    printf("send notice to \"%s\" \n", url.c_str());
    if ((WiFi.status() == WL_CONNECTED)) {
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
}
void update_order() {
   //http:adsad/order
    String url = domain + order_api;
    String payload = "";
 
    if ((WiFi.status() == WL_CONNECTED)) {
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
            payload = http.getString();
            //             Serial.printf("回應本體：%s\n", payload.c_str());
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
    Serial.println(payload);
    
    // 解析json
    int menu_latest[10];
    int UID_latest[4];
    DynamicJsonDocument doc(800);
    deserializeJson(doc, payload);
    for (int i = 0; i < 10; i++)
        menu_latest[i] = (int)doc["menu"][i];
    String temp = doc["room"];
    char room_number_latest = temp[0];
    //  customer_name = doc["name"];
    for (int i = 0; i < 4; i++)
        UID_latest[i] = (int)doc["rfid"][i];
 
    printf("--------------\nwe get new order!\n");
    for (int i = 0; i < 10; i++)
        printf("menu[%d] : %d\n", i, menu_latest[i]);
    printf("room_number : %c\n", room_number_latest);
    for (int i = 0; i < 4; i++)
        printf("UID[%d] : %d\n", i, UID_latest[i]);
    printf("--------------\n");
    queue.put(menu_latest, room_number_latest, UID_latest);  // pass by ref
 
    //myDFPlayer.playLargeFolder(1, 3);  // 0003收到新訂單.wav
    /*
    // buzzer to gettig one order
    printf("buzzer notice\n");
    for (int i = 0; i < 10; i++) {
        ledcWriteTone(0, 1000);
        delay(50);
        ledcWriteTone(0, 2000);
        delay(50);
    }
    ledcWriteTone(0, 0);
    */
}
void lcd_empty(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("nothing");
}
void lcd_wifi(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("no wifi");
}
void lcd_cook(){
  lcd.clear();
  lcd.setCursor(0, 0);
  String order_msg = "+" + (String)queue.size();
  lcd.print(order_msg );
  lcd.clear();
  String line="cooking!!!";
  for(int i=16;i>=0;i--){
    lcd.setCursor(i, 0);
    lcd.print(line);
    lcd.print(" ");
    delay(500);
  }
  for(int i=0;i<=line.length();i++){
     lcd.setCursor(0, 0);
     lcd.print(line.substring(i+1,line.length()));
     lcd.print(" ");
     delay(500);
  }

}
void lcd_deliver(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("start in");
}
void lcd_cancel(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("is being canceled!");
}
void lcd_go_back(){
   lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("go back");
}
