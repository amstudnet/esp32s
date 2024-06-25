#include <SoftwareSerial.h>
SoftwareSerial BT(50, 51);    //blue tooth pin (tx, rx)
void (*resetFunc)(void) = 0;  //declare reset function at address 0
//use arduino uno
// 前左馬達控制設定(front left)
const byte FL_in1 = 12;  //In1
const byte FL_in2 = 11;  //In2
const byte FL_ENA = 13;  //ENA

// 前右馬達控制設定(front right)
const byte FR_in3 = 10;  //In3
const byte FR_in4 = 9;   //In4
const byte FR_ENB = 8;   //ENB

// 後左馬達控制設定(back left)
const byte BL_in1 = 6;  //In1
const byte BL_in2 = 5;  //In2
const byte BL_ENA = 7;  //ENA

// 後右馬達控制設定(back right)
const byte BR_in3 = 4;  //In3
const byte BR_in4 = 3;  //In4
const byte BR_ENB = 2;  //ENB  A5,A6是壞的

// 蓋子控制設定
const byte in1 = 44;  //In1
const byte in2 = 45;  //In2
const byte ENA = 46;  //ENA

byte motorSpeed = 100;  // 設定PWM輸出值

//use for test PWM
byte motorSpeed_L = 53;
byte motorSpeed_R = 40;

//5路循跡感測器
const byte out1 = 38;
const byte out2 = 36;
const byte out3 = 34;
const byte out4 = 32;
const byte out5 = 30;
const int code[32][5] = {
    {0, 0, 0, 0, 0},  //0 stop or start
    {0, 0, 0, 0, 1},  //1
    {0, 0, 0, 1, 0},  //2
    {0, 0, 0, 1, 1},  //3
    {0, 0, 1, 0, 0},  //4
    {0, 0, 1, 0, 1},  //5
    {0, 0, 1, 1, 0},  //6
    {0, 0, 1, 1, 1},  //7
    {0, 1, 0, 0, 0},  //8
    {0, 1, 0, 0, 1},  //9
    {0, 1, 0, 1, 0},  //10
    {0, 1, 0, 1, 1},  //11
    {0, 1, 1, 0, 0},  //12
    {0, 1, 1, 0, 1},  //13
    {0, 1, 1, 1, 0},  //14
    {0, 1, 1, 1, 1},  //15
    {1, 0, 0, 0, 0},  //16
    {1, 0, 0, 0, 1},  //17  二號房
    {1, 0, 0, 1, 0},  //18
    {1, 0, 0, 1, 1},  //19 向左脫軌
    {1, 0, 1, 0, 0},  //20
    {1, 0, 1, 0, 1},  //21
    {1, 0, 1, 1, 0},  //22
    {1, 0, 1, 1, 1},  //23
    {1, 1, 0, 0, 0},  //24  一號房
    {1, 1, 0, 0, 1},  //25 向右脫軌
    {1, 1, 0, 1, 0},  //26
    {1, 1, 0, 1, 1},  //27 forward
    {1, 1, 1, 0, 0},  //28
    {1, 1, 1, 0, 1},  //29
    {1, 1, 1, 1, 0},  //30
    {1, 1, 1, 1, 1},  //31
};
bool start = true;
void store_in(int* storage);
int compare(const int* test);
char direction(const int* storage);
bool system_on = false;
int count_forward = 0;
bool over = false;
char room_number;

void forward() {  //前進 (完成)
                  //control front
    digitalWrite(FL_in1, HIGH);
    digitalWrite(FL_in2, LOW);
    digitalWrite(FR_in3, LOW);
    digitalWrite(FR_in4, HIGH);
    //control back
    digitalWrite(BL_in1, HIGH);
    digitalWrite(BL_in2, LOW);
    digitalWrite(BR_in3, HIGH);
    digitalWrite(BR_in4, LOW);
    //PWM
    analogWrite(FL_ENA, 62);
    analogWrite(FR_ENB, 54);
    analogWrite(BL_ENA, 62);
    analogWrite(BR_ENB, 54);
}

void backward() {  //後退
                   //control front
    digitalWrite(FL_in1, LOW);
    digitalWrite(FL_in2, HIGH);
    digitalWrite(FR_in3, HIGH);
    digitalWrite(FR_in4, LOW);
    //control back
    digitalWrite(BL_in1, LOW);
    digitalWrite(BL_in2, HIGH);
    digitalWrite(BR_in3, LOW);
    digitalWrite(BR_in4, HIGH);
    //PWM
    analogWrite(FL_ENA, motorSpeed_L);
    analogWrite(FR_ENB, motorSpeed_R);
    analogWrite(BL_ENA, motorSpeed_L);
    analogWrite(BR_ENB, motorSpeed_R);
}

void right_horizontal() {  // 右邊橫向
                           //control front
    digitalWrite(FL_in1, HIGH);
    digitalWrite(FL_in2, LOW);
    digitalWrite(FR_in3, HIGH);
    digitalWrite(FR_in4, LOW);
    //control back
    digitalWrite(BL_in1, LOW);
    digitalWrite(BL_in2, HIGH);
    digitalWrite(BR_in3, HIGH);
    digitalWrite(BR_in4, LOW);
    //PWM
    analogWrite(FL_ENA, 70);
    analogWrite(FR_ENB, 50);
    analogWrite(BL_ENA, 60);
    analogWrite(BR_ENB, 50);
}

void left_horizontal() {  // 左邊橫向
                          //control front
    digitalWrite(FL_in1, LOW);
    digitalWrite(FL_in2, HIGH);
    digitalWrite(FR_in3, LOW);
    digitalWrite(FR_in4, HIGH);
    //control back
    digitalWrite(BL_in1, HIGH);
    digitalWrite(BL_in2, LOW);
    digitalWrite(BR_in3, LOW);
    digitalWrite(BR_in4, HIGH);
    //PWM
    analogWrite(FL_ENA, motorSpeed_L);
    analogWrite(FR_ENB, motorSpeed_R);
    analogWrite(BL_ENA, motorSpeed_L);
    analogWrite(BR_ENB, motorSpeed_R);
}

void right_rotate() {  //原地右轉
                       //control front
    digitalWrite(FL_in1, HIGH);
    digitalWrite(FL_in2, LOW);
    digitalWrite(FR_in3, HIGH);
    digitalWrite(FR_in4, LOW);
    //control back
    digitalWrite(BL_in1, HIGH);
    digitalWrite(BL_in2, LOW);
    digitalWrite(BR_in3, LOW);
    digitalWrite(BR_in4, HIGH);
    //PWM
    analogWrite(FL_ENA, 70);
    analogWrite(FR_ENB, 60);
    analogWrite(BL_ENA, 70);
    analogWrite(BR_ENB, 60);
}

void left_rotate() {  //原地左轉
                      //control front
    digitalWrite(FL_in1, LOW);
    digitalWrite(FL_in2, HIGH);
    digitalWrite(FR_in3, LOW);
    digitalWrite(FR_in4, HIGH);
    //control back
    digitalWrite(BL_in1, LOW);
    digitalWrite(BL_in2, HIGH);
    digitalWrite(BR_in3, HIGH);
    digitalWrite(BR_in4, LOW);
    //PWM
    analogWrite(FL_ENA, motorSpeed);
    analogWrite(FR_ENB, motorSpeed);
    analogWrite(BL_ENA, motorSpeed);
    analogWrite(BR_ENB, motorSpeed);
}

void rotate_axis_BL() {  //以BL為軸左旋轉
    //control front
    digitalWrite(FL_in1, LOW);
    digitalWrite(FL_in2, LOW);
    digitalWrite(FR_in3, LOW);
    digitalWrite(FR_in4, HIGH);
    //control back
    digitalWrite(BL_in1, LOW);
    digitalWrite(BL_in2, LOW);
    digitalWrite(BR_in3, HIGH);
    digitalWrite(BR_in4, LOW);
    //PWM
    analogWrite(FL_ENA, 0);
    analogWrite(FR_ENB, 58);
    analogWrite(BL_ENA, 0);
    analogWrite(BR_ENB, 58);
}

void rotate_axis_BR() {  //以BR為軸右旋轉
    //control front
    digitalWrite(FL_in1, HIGH);
    digitalWrite(FL_in2, LOW);
    digitalWrite(FR_in3, LOW);
    digitalWrite(FR_in4, LOW);
    //control back
    digitalWrite(BL_in1, HIGH);
    digitalWrite(BL_in2, LOW);
    digitalWrite(BR_in3, LOW);
    digitalWrite(BR_in4, LOW);
    //PWM
    analogWrite(FL_ENA, 75);
    analogWrite(FR_ENB, 0);
    analogWrite(BL_ENA, 75);
    analogWrite(BR_ENB, 0);
}

void stop() {
    //control front
    digitalWrite(FL_in1, LOW);
    digitalWrite(FL_in2, LOW);
    digitalWrite(FR_in3, LOW);
    digitalWrite(FR_in4, LOW);
    //control back
    digitalWrite(BL_in1, LOW);
    digitalWrite(BL_in2, LOW);
    digitalWrite(BR_in3, LOW);
    digitalWrite(BR_in4, LOW);
    //PWM
    analogWrite(FL_ENA, 0);
    analogWrite(FR_ENB, 0);
    analogWrite(BL_ENA, 0);
    analogWrite(BR_ENB, 0);
}

void setup() {
    //about L298N
    pinMode(FL_in1, OUTPUT);
    pinMode(FL_in2, OUTPUT);
    pinMode(FL_ENA, OUTPUT);
    pinMode(FR_in3, OUTPUT);
    pinMode(FR_in4, OUTPUT);
    pinMode(FR_ENB, OUTPUT);
    pinMode(BL_in1, OUTPUT);
    pinMode(BL_in2, OUTPUT);
    pinMode(BL_ENA, OUTPUT);
    pinMode(BR_in3, OUTPUT);
    pinMode(BR_in4, OUTPUT);
    pinMode(BR_ENB, OUTPUT);

    //pinMode(out1, INPUT);
    //pinMode(out2, INPUT);
    //pinMode(out3, INPUT);
    //pinMode(out4, INPUT);
    //pinMode(out5, INPUT);

    Serial.begin(38400);
    BT.begin(38400);
}

void loop() {
    int storage[5];     //store from TCRT5000
    store_in(storage);  //store data
    char cmd;
    bool cmd_state = false;

    if (system_on) {  // recieve from sensor
        cmd = direction(storage);
        cmd_state = true;
    }

    if (BT.available()) {  // recieve from bluetooth
        cmd = BT.read();
        cmd_state = true;
    }

    if (cmd_state) {
        switch (cmd) {
            case 'w':  //前進
                //BT.println("forward");
                count_forward++;
                //BT.print("count_forward: ");
                //BT.println(count_forward);
                forward();

                break;
            case 'x':  //後退  wrong
                //BT.println("backward");
                backward();
                break;
            case 'a':  //左橫移  wrong
                //BT.println("left_horizontal");
                left_horizontal();
                break;
            case 'd':  //右橫移
                //BT.println("right_horizontal");
                right_horizontal();
                break;
            case 'v':  //左自轉
                //BT.println("left_rotate");
                left_rotate();
                break;
            case 'b':  //右自轉  wrong
                //BT.println("right_rotate");
                right_rotate();
                break;
            case 's':  //停止
                //BT.println("stop");
                count_forward = 0;  // reset count_forward
                stop();
                system_on = false;
                break;
            case 'c':  //complete straight  // shot down
                system_on = false;
                stop();
                BT.write('1');
                resetFunc();  //call reset exit(0);
            case 'o':         //Sensor open
                //BT.println("Sensor turn on");
                //BT.println();
                system_on = true;
                break;
            case 'p':  //Sensor close
                //BT.println("Sensor turn off");
                system_on = false;
                stop();
                break;
            case 'l':  //
                //BT.println("rotate_axis_BL");
                rotate_axis_BL();
                delay(300);
                stop();
                break;
            case 'r':  //
                //BT.println("rotate_axis_BR");
                rotate_axis_BR();
                delay(300);
                stop();
                break;
            case 'h':
            case 'j':
            case 'k':  // room number receive
                room_number = cmd;
                //system_on = true;
                simple_route();
                break;
            case 't':
                //BT.println("type in t");
                stop();
                while (true) {
                    char restart;
                    if (BT.available()) {  // recieve from bluetooth
                        restart = BT.read();
                    }
                    if (restart == '2') {
                        system_on = true;
                        break;
                    }
                }
                break;
            default:
                //stop();
                //BT.println("type in default");
                break;
        }
        cmd_state = false;
    }

    delay(70);
}

void store_in(int* storage) {
    storage[0] = digitalRead(out1);
    storage[1] = digitalRead(out2);
    storage[2] = digitalRead(out3);
    storage[3] = digitalRead(out4);
    storage[4] = digitalRead(out5);
}
int compare(const int* test) {
    int correct = 0;

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 5; j++) {
            if (test[j] == code[i][j]) {
                correct++;
                if (correct == 5) return i;
            } else {
                correct = 0;
                break;
            }
        }
    }
    return 0;
}
void print_store(const int* storage) {
    for (int i = 0; i < 5; i++) {
        BT.print(storage[i]);
    }
    BT.println();
}
char direction(const int* storage) {
    int event = compare(storage);
    over = false;
    //Serial.println(event); //for test even
    //print_store(storage);  // for test marker

    switch (event) {
        case 0:  //00000 all black
            //BT.print("Storage:");
            //print_store(storage); // for test-------
            if (start) {  // start
                //Serial.println("Start.....sensor");
                start = false;
                forward();
                delay(500);
                return 'w';
            } else if (count_forward > 3) {  //complete a straight
                //delay(50);
                start == true;
                return 'c';  // shot down
            }
            break;
        case 27:  //11011 forward
            return 'w';
            break;
        case 24:  // 11000 room 1
            //BT.println("room 1 arrive......");
            //--------------------do something----------------------//
            if (room_number == 'h') {
                stop();
                BT.write('1');
                while (true) {
                    char restart;
                    if (BT.available()) {  // recieve from bluetooth
                        restart = BT.read();
                        //BT.print("type in: ");
                        //BT.println(restart);
                    }
                    if (restart == '2') {
                        //BT.println("restart.......");
                        break;
                    }
                }
            } else {
                //BT.println(" not room1......");
                stop();
                delay(500);
            }
            forward();
            delay(500);
            return 'w';
        case 17:  //10001  room2
            //BT.println("room 2 arrive......");
            //---------------------do something--------------------//
            if (room_number == 'j') {
                stop();
                BT.write('1');
                while (true) {
                    char restart;
                    if (BT.available()) {  // recieve from bluetooth
                        restart = BT.read();
                        //BT.print("type in: ");
                        //BT.println(restart);
                    }
                    if (restart == '2') {
                        //BT.println("restart.......");
                        break;
                    }
                }
            } else {
                //BT.println(" not room2......");
                stop();
                delay(500);
            }
            forward();
            delay(500);
            return 'w';
        case 3:  //00011  room3
            //BT.println("room 3 arrive......");
            //---------------------do something--------------------//
            if (room_number == 'k') {
                stop();
                BT.write('1');
                while (true) {
                    char restart;
                    if (BT.available()) {  // recieve from bluetooth
                        restart = BT.read();
                        //BT.print("type in: ");
                        //BT.println(restart);
                    }
                    if (restart == '2') {
                        //BT.println("restart.......");
                        break;
                    }
                }
            } else {
                //BT.println(" not room3......");
                stop();
                delay(500);
            }
            forward();
            delay(500);
            return 'w';
        case 19:  //10011 向右脫軌
            //BT.println("fix trace to left.....");
            do {
                if (over_fix_to(25)) break;
                rotate_axis_BL();
                delay(200);
            } while (fix_imcomplete(25));

            return (over) ? 'r' : 'w';
            break;
        case 25:  //11001 向左脫軌
            //BT.println("fix trace to right.....");
            do {
                if (over_fix_to(19)) break;
                rotate_axis_BR();
                delay(200);
            } while (fix_imcomplete(19));
            return (over) ? 'l' : 'w';
            break;
        case 31:  //11111
            //BT.println("over path......");
            //do something
            return 's';
        default:  // code not define
            //BT.println("need correction......");
            return 's';
    }
}
bool over_fix_to(int number) {
    int temp_storage[5];
    int count;
    store_in(temp_storage);
    //BT.print("code in over_fix_to:");
    //print_store(temp_storage);

    for (int i = 0; i < 5; i++) {
        if (temp_storage[i] == code[number][i]) count++;
    }

    if (count == 5) {
        over = true;
        return true;
    } else {
        return false;
    }
}
bool fix_imcomplete(int number) {  // 修正回正軌
    int temp_storage[5];
    int count = 0;
    store_in(temp_storage);
    //BT.print("code in fix_imcomplete");
    //print_store(temp_storage);

    for (int i = 0; i < 5; i++) {
        if (temp_storage[i] == code[number][i]) count++;
    }
    if (count == 5) {
        //BT.println("in fix_imcomplete (true)");
        over = true;
        return false;
    }

    for (int i = 0; i < 5; i++) {
        if (temp_storage[i] != code[27][i]) return true;
    }
    over = false;
    return false;
}

void simple_route() {
    int Room_number;
    int drive;
    bool route_imcomplete = false;

    switch (room_number) {
        case 'h':
            Room_number = 1;
            break;
        case 'j':
            Room_number = 2;
            break;
        case 'k':
            Room_number = 3;
            break;
    }
    //forward to assign room
    forward();

    for (int i = 0; i < 3000 * Room_number; i++) {
        char T;
        delay(1);
        if (BT.available()) {  // recieve from bluetooth
            T = BT.read();
        }
        if (T == 't') {
            //BT.println("been stolen");
            stop();
            drive = i;
            drive = 3000 * Room_number - drive;
            route_imcomplete = true;
            break;
        }
    }

    // if been stolen，restart
    while (route_imcomplete) {
        char restart;
        if (BT.available()) {  // recieve from bluetooth
            restart = BT.read();
        }
        if (restart == '2') {
            forward();
            for (int i = 0; i < drive; i++) {
                delay(1);
            }
            break;
        }
    }

    stop();

    BT.write('1');  // write '1' to ESP32
    while (true) {
        char open;
        if (BT.available()) {  // recieve from bluetooth
            open = BT.read();
        }
        if (open == '3') {
            open_lid();
            break;
        }
    }

    while (true) {
        char restart;
        if (BT.available()) {  // recieve from bluetooth
            restart = BT.read();
        }
        if (restart == '2') {
            break;
        }
    }
    close_lid();
    delay(50);

    //return to start point
    right_rotate();
    delay(3570);  // -------------need to test
    stop();
    forward();
    delay(3000 * Room_number);
    stop();

    //move head to forward
    right_rotate();
    delay(3570);  // -------------need to test
    stop();

    //reset system
    BT.write('1');
    resetFunc();
}

void open_lid() {
    //control front
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    //PWM
    analogWrite(ENA, 40);

    //--------------//
    delay(240);
    stop_lid();
    //--------------//
}
void close_lid() {
    //control front
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    //PWM
    analogWrite(ENA, 10);

    //--------------//
    delay(140);
    stop_lid();
    //--------------//
}
void stop_lid() {
    //control front
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    //PWM
    analogWrite(ENA, 0);
}
