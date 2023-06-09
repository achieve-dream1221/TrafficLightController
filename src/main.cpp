/**********************************
* @author: achieve_dream
* @date: 2023/6/5 23:00
* @file: main.cpp
***********************************/
#include <Arduino.h>
// R, G, Y�Ƶ�����
#define LED_R 13
#define LED_G 14
#define LED_Y 2
// ������Ƶ��
#define SPEAKER_FREQ 700
// ������IO
#define SPEAKER 15
// ��ѹ��IO
#define VOLTS 16
// ��ʾһ������, Ȼ����ͣFREQ����, ��ʾ��һ������
#define FREQ 30
// ����Ŀʹ�õ��������ֵ������
// �Ƿ��ǹ�����, ����Ϊtrue��false
#define COMMON_ANODE true
// �������ʾ�������IO
#define LEFT 11
// �������ʾ�Ҳ�����IO
#define RIGHT 12
// ����ܸ������ÿ��С�������IO
static const unsigned char digital_pins[8] = {3, 4, 5, 6, 7, 8, 9, 10};
// ����� 0 - 9 �ı���
static const unsigned char numbers[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
                                          0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110};


class DreamOS {
public:
    /// ����ϵͳ
    void run() {
        onRed();
        onGreen();
        onYellow();
    }

private:
    // Զ�̿��޸ĵĵ���ʱ����
    unsigned char num{};

    /// �������ʾһ��һ������, ����ֻ����˸���ݵ�һ��
/// \param num ��ʾ������
/// \param isLeft �Ƿ���ʾ�����
    static void showOnceNumber(unsigned char num, bool isLeft) {
        /*    if (num > 9) {
                Serial.println("out of range, only can show: 0 - 9");
                return;
            }*/
        // ���������
        isLeft ? digitalWrite(LEFT, HIGH) : digitalWrite(RIGHT, HIGH);
        num = numbers[num];
        for (char i = 7; i >= 0; --i) {
#if COMMON_ANODE // ������
            if (num & (1 << i)) { // 1

                digitalWrite(digital_pins[7 - i], LOW); // ������ҪLOW�Ż���, ������ҪHIGH����
            } else { // 0
                digitalWrite(digital_pins[7 - i], HIGH);
            }
#else // ������
            if (num & (1 << i)) { // 1

                digitalWrite(digital_pins[7 - i], HIGH); // ������ҪLOW�Ż���, ������ҪHIGH����
            } else { // 0
                digitalWrite(digital_pins[7 - i], LOW);
            }
#endif

        }
        delay(FREQ);
        // �ر������
        isLeft ? digitalWrite(LEFT, LOW) : digitalWrite(RIGHT, LOW);
    }

    /// ��ʾ��λ����
    /// \param num ��ʾ������
    static void showTwoNumber(unsigned char num) {
        /*       if (num > 99) {
                   Serial.println("out of range, only can show: 0 - 99");
                   return;
               }*/
        // ��ʾ1����, ��˸��ʾ, �����Ӿ������ﵽ��ʾ�������
        for (int count = 0; count < 1000 / (FREQ * 4); ++count) {
            showOnceNumber(num / 10, true); // ��ʾ�������
            showOnceNumber(num % 10, false); // ��ʾ�Ҳ�����
        }
    }

    /// �жϴ����Ƿ�ı��˵���ʱʱ��
    /// \return bool
    bool isChanged() {
        if (Serial.available()) { //������������
            int first = Serial.read(); // read: ��������, �򷵻��ַ����򷵻�-1
            if (first >= '0' && first <= '9') {
                int second = Serial.read();
                if (second == -1) { // ֻ��һλ��
                    num = first - '0';
                    Serial.print("[success]: num = ");
                    Serial.println(num);
                    return true;
                }
                if (Serial.read() == -1 && second >= '0' && second <= '9') { //�����ڵ���λ�����ҵڶ�λ����������
                    num = (first - '0') * 10 + second - '0';
                    Serial.print("[success]: num = ");
                    Serial.println(num);
                    return true;
                }
            }
            // �ͷŶ��������
            for (; Serial.read() != -1;);
            Serial.println("[ERROR]: only allowed input a number: 0 - 99.");
        }
        return false;
    }


    /// ����ܵ���ʱ
    /// \param time ��ʾ�ĵ���ʱʱ��
    void countdown(unsigned char time) {
        // ����ʱ
        for (short i = time; i > 0; --i) {
            // ���ģʽ,���ҵ�ѹ��Ϊ0, ���ȼ�Ӧ�ñ��������������, ��˷��ڵ�һ��
            if (digitalRead(LED_R) && analogRead(VOLTS) == 0) Serial.println("Someone across the road!");
            if (isChanged()) i = num;// ������ֱ��޸�, ��ô���޸ĺ�����ֿ�ʼ����ʱ
            showTwoNumber(i);
        }
    }

    /// ���
    void onRed() {
        digitalWrite(LED_R, HIGH);
        countdown(20);
        digitalWrite(LED_R, LOW);
    }

    /// �̵�
    void onGreen() {
        digitalWrite(LED_G, HIGH);
        tone(SPEAKER, SPEAKER_FREQ);
        countdown(15);
        noTone(SPEAKER);
        digitalWrite(LED_G, LOW);
    }

    /// �Ƶ�
    void onYellow() {
        digitalWrite(LED_Y, HIGH);
        countdown(10);
        digitalWrite(LED_Y, LOW);
    }
};

DreamOS dream;

void setup() {
    // ���ڲ�����, ��ֹ����
    Serial.begin(9600);
    // ��ʼ������״̬
    for (unsigned char i = 2; i < 16; ++i) { // 2 - 15����������Ϊ���ģʽ
        pinMode(i, OUTPUT);
    }
    pinMode(VOLTS, INPUT); // ��ȡ��ѹ��, ����Ϊ����ģʽ
}

void loop() {
    dream.run();
}