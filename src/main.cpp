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
// 蜂鸣器频率
#define SPEAKER_FREQ 700
// 蜂鸣器IO
#define SPEAKER 15
// 电压表IO
#define VOLTS 16
// 显示一个数字, 然后暂停FREQ毫秒, 显示另一个数字
#define FREQ 30
// 本项目使用的两个数字的数码管
// 是否是共阳极, 设置为true或false
#define COMMON_ANODE true
// 数码管显示左侧数字IO
#define LEFT 11
// 数码管显示右侧数字IO
#define RIGHT 12
// 数码管负责控制每个小灯亮灭的IO
static const unsigned char digital_pins[8] = {3, 4, 5, 6, 7, 8, 9, 10};
// 数码管 0 - 9 的编码
static const unsigned char numbers[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
                                          0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110};


class DreamOS {
public:
    /// 启动系统
    void run() {
        onRed();
        onGreen();
        onYellow();
    }

private:
    // 远程可修改的倒计时数字
    unsigned char num{};

    /// 数码管显示一次一个数字, 并且只会闪烁短暂的一次
/// \param num 显示的数字
/// \param isLeft 是否显示在左侧
    static void showOnceNumber(unsigned char num, bool isLeft) {
        /*    if (num > 9) {
                Serial.println("out of range, only can show: 0 - 9");
                return;
            }*/
        // 开启数码管
        isLeft ? digitalWrite(LEFT, HIGH) : digitalWrite(RIGHT, HIGH);
        num = numbers[num];
        for (char i = 7; i >= 0; --i) {
#if COMMON_ANODE // 共阳极
            if (num & (1 << i)) { // 1

                digitalWrite(digital_pins[7 - i], LOW); // 共阳极要LOW才会亮, 共阴极要HIGH才亮
            } else { // 0
                digitalWrite(digital_pins[7 - i], HIGH);
            }
#else // 共阴极
            if (num & (1 << i)) { // 1

                digitalWrite(digital_pins[7 - i], HIGH); // 共阳极要LOW才会亮, 共阴极要HIGH才亮
            } else { // 0
                digitalWrite(digital_pins[7 - i], LOW);
            }
#endif

        }
        delay(FREQ);
        // 关闭数码管
        isLeft ? digitalWrite(LEFT, LOW) : digitalWrite(RIGHT, LOW);
    }

    /// 显示两位数字
    /// \param num 显示的数字
    static void showTwoNumber(unsigned char num) {
        /*       if (num > 99) {
                   Serial.println("out of range, only can show: 0 - 99");
                   return;
               }*/
        // 显示1秒钟, 闪烁显示, 利用视觉暂留达到显示多个数字
        for (int count = 0; count < 1000 / (FREQ * 4); ++count) {
            showOnceNumber(num / 10, true); // 显示左侧数字
            showOnceNumber(num % 10, false); // 显示右侧数字
        }
    }

    /// 判断串口是否改变了倒计时时间
    /// \return bool
    bool isChanged() {
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
            // 红灯模式,并且电压表为0, 优先级应该比其他两个任务高, 因此放在第一句
            if (digitalRead(LED_R) && analogRead(VOLTS) == 0) Serial.println("Someone across the road!");
            if (isChanged()) i = num;// 如果数字被修改, 那么从修改后的数字开始倒计时
            showTwoNumber(i);
        }
    }

    /// 红灯
    void onRed() {
        digitalWrite(LED_R, HIGH);
        countdown(20);
        digitalWrite(LED_R, LOW);
    }

    /// 绿灯
    void onGreen() {
        digitalWrite(LED_G, HIGH);
        tone(SPEAKER, SPEAKER_FREQ);
        countdown(15);
        noTone(SPEAKER);
        digitalWrite(LED_G, LOW);
    }

    /// 黄灯
    void onYellow() {
        digitalWrite(LED_Y, HIGH);
        countdown(10);
        digitalWrite(LED_Y, LOW);
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