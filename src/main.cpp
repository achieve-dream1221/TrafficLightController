/**********************************
* @author: achieve_dream
* @date: 2023/6/5 23:00
* @file: main.cpp
***********************************/
#include <Arduino.h>
// R, G, Y灯的引脚
#define LED_R 13
#define LED_G 14
#define LED_Y 2
// 显示一个数字, 然后暂停FREQ毫秒, 显示另一个数字
#define FREQ 30
// 数码管左侧
#define LEFT 11
// 数码管右侧
#define RIGHT 12
// 蜂鸣器频率
#define SPEAKER_FREQ 700
// 蜂鸣器IO
#define SPEAKER 15
// 电压表IO
#define VOLTS 16
// 数码管 0 - 9 的编码
static const unsigned char numbers[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
                                          0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110};


class DreamOS {
public:
    /// 启动系统
    void run() {
        redLed();
        greenLed();
        yellowLed();
    }

private:
    // 远程可修改的倒计时数字
    unsigned char num{};
    // 是否处于红灯模式
    bool isRedMode{};

    /// 数码管显示一个数字, 只能为: 0 - 9, 否则不显示
/// \param num 显示的数字
/// \param isLeft 是否显示在左侧, 默认为真
    static void showOneNumber(unsigned char num, bool isLeft) {
    /*    if (num > 9) {
            Serial.println("out of range, only can show: 0 - 9");
            return;
        }*/
        digitalWrite(LEFT, LOW); // 左侧不亮
        digitalWrite(RIGHT, LOW); // 右侧不亮
        if (isLeft) {
            digitalWrite(LEFT, HIGH); // 左侧亮
        } else {
            digitalWrite(RIGHT, HIGH); // 右侧亮
        }
        num = numbers[num];
        unsigned char pin = 3;
        for (char i = 7; i >= 0; --i) {
            if (num & (1 << i)) {
                digitalWrite(pin, LOW);
            } else {
                digitalWrite(pin, HIGH);
            }
            ++pin;
        }
        delay(FREQ);
    }

/// 显示两位数字
/// \param num 显示的数字
    static void showNumber(unsigned char num) {
        /*       if (num > 99) {
                   Serial.println("out of range, only can show: 0 - 99");
                   return;
               }*/
        unsigned char left_num = num / 10;
        unsigned char right_num = num % 10;
        // 显示1秒钟, 闪烁显示
        for (int count = 0; count < 1000 / (FREQ * 4); ++count) {
            showOneNumber(left_num, true); // 显示左侧数字
            showOneNumber(right_num, false); // 显示右侧数字
        }
    }

    bool hasChanged() {
        if (Serial.available()) { //缓存区有数据
            int first = Serial.read(); // read: 若有数据, 则返回字符否则返回-1
            if (first >= '0' && first <= '9') {
                int second = Serial.read();
                if (second == -1) { // 只有一位数
                    num = first - '0';
                    Serial.print("[success]: num = ");
                    Serial.println(num);
                    return true;
                }
                if (Serial.read() == -1 && second >= '0' && second <= '9') { //不存在第三位数并且第二位数据是数字
                    num = (first - '0') * 10 + second - '0';
                    Serial.print("[success]: num = ");
                    Serial.println(num);
                    return true;
                }
            }
            // 释放多余的数据
            for (; Serial.read() != -1;);
            Serial.println("[ERROR]: only allowed input a number: 0 - 99.");
        }
        return false;
    }


    /// 数码管倒计时
    /// \param time 显示的倒计时时间
    void countdown(unsigned char time) {
        // 倒计时
        for (short i = time; i > 0; --i) {
            if (hasChanged()) i = num;// 如果数字被修改, 那么从修改后的数字开始倒计时
            showNumber(i);
            if (isRedMode && analogRead(VOLTS) == 0) { // 红灯模式,并且电压表为0
                Serial.println("Someone across the road!");
            }
        }
    }

    /// 红灯
    void redLed() {
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_Y, LOW);
        isRedMode = true; // 红灯模式
        countdown(20);
        isRedMode = false; // 非红灯模式
    }

    /// 绿灯
    void greenLed() {
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_Y, LOW);
        tone(SPEAKER, SPEAKER_FREQ);
        countdown(15);
        noTone(SPEAKER);
    }

    /// 黄灯
    void yellowLed() {
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_Y, HIGH);
        countdown(10);
    }
};

DreamOS dream;

void setup() {
    // 串口波特率, 防止乱码
    Serial.begin(9600);
    // 初始化引脚状态
    for (unsigned char i = 2; i < 16; ++i) { // 2 - 15号引脚设置为输出模式
        pinMode(i, OUTPUT);
    }
    pinMode(VOLTS, INPUT); // 读取电压表, 设置为输入模式
}

void loop() {
    dream.run();
}