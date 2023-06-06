/*
 * ��������ܣ������������������ɽ�ͨ�ƿ��ƣ��������ʾ��ǰ��ͨ����ʣʱ�䣬ÿһ����ֵ���٣����20�룬�Ƶ�10s���̵�15s��
 * ����������ͨѶ��Զ�̿����޸�����ܵ���ʾֵ�����̵���ʱ���������ȶ������˿���ͨ�������ñ���˿��·���������˴�����ʾ
 * ��һ������˿�̣�����λ������Ӣ�ġ�someone across the road����
 * */

/**********************************
* @author: achieve_dream
* @date: 2023/6/5 23:00
* @file: test.cpp
***********************************/
#include <Arduino.h>
// R, G, Y�Ƶ�����
#define LED_R 13
#define LED_G 14
#define LED_Y 2
// ��ʾһ������, Ȼ����ͣFREQ����, ��ʾ��һ������
#define FREQ 30
// ��������
#define LEFT 11
// ������Ҳ�
#define RIGHT 12
// ������Ƶ��
#define SPEAKER_FREQ 700
// ������IO
#define SPEAKER 15
// �ֽڻ�������С
#define BUFFER_SIZE 8
// ���ڽ�����������ʱ��
#define TIME_OUT 10
// ��ѹ��IO
#define VOLTS 16
// ����� 0 - 9 �ı���
static const unsigned char numbers[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110,
                                          0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110};


class DreamOS {
public:
/// �������ʾһ������, ֻ��Ϊ: 0 - 9, ������ʾ
/// \param num ��ʾ������
/// \param isLeft �Ƿ���ʾ�����, Ĭ��Ϊ��
    static void showOneNumber(unsigned char num, bool isLeft) {
        if (num > 9) {
            Serial.println("out of range, only can show: 0 - 9");
            return;
        }
        digitalWrite(LEFT, LOW); // ��಻��
        digitalWrite(RIGHT, LOW); // �Ҳ಻��
        if (isLeft) {
            digitalWrite(LEFT, HIGH); // �����
        } else {
            digitalWrite(RIGHT, HIGH); // �Ҳ���
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

/// ��ʾ��λ����
/// \param num ��ʾ������
    static void showNumber(unsigned char num) {
        if (num > 99) {
            Serial.println("out of range, only can show: 0 - 99");
            return;
        }
        unsigned char left_num = num / 10;
        unsigned char right_num = num % 10;
        // ��ʾ1����, ��˸��ʾ
        for (int count = 0; count < 1000 / (FREQ * 4); ++count) {
            showOneNumber(left_num, true); // ��ʾ�������
            showOneNumber(right_num, false); // ��ʾ�Ҳ�����
        }
    }

    /// �ж�һ���ַ��Ƿ��������ַ�
    /// \param c ���жϵ��ַ�
    /// \return bool
    static bool isNumber(char c) {
        if (c >= 0x30 && c <= 0x39) return true;
        return false;
    }

    DreamOS() {
        // ���ڽ�����������ʱ��
        Serial.setTimeout(TIME_OUT);
    }

    /// �жϺ��̵Ƶ���ʱ�����Ƿ��޸�
    /// \return bool
    bool hasChanged() {
        // ���ӵ�����
        if (Serial.available()) {
            // ���������յ�2λ�ַ���1λ�ַ�, ���������Ϊ�쳣
            size_t len = Serial.readBytes(serialBuffer, BUFFER_SIZE);
            if (len == 1 && isNumber(serialBuffer[0])) { // ���յ�һλ��
                num = serialBuffer[0] - 0x30;
                Serial.print("[success]: num = ");
                Serial.println(num);
                return true;
            } else if (len == 2 && isNumber(serialBuffer[0]) && isNumber(serialBuffer[1])) { // ���յ���λ��
                unsigned char tensDigit, digits;
                digits = serialBuffer[1] - 0x30;
                tensDigit = serialBuffer[0] - 0x30;
                num = tensDigit * 10 + digits;
                Serial.print("[success]: num = ");
                Serial.println(num);
                return true;
            } else { // ����λ��, ���ͷŵ����������
                for (; Serial.readBytes(serialBuffer, BUFFER_SIZE) > 0;);
                Serial.println("[ERROR]: only allowed input a number: 0 - 99.");
            }
        }
        return false;
    }

    /// ����ϵͳ
    void run() {
        redLed();
        greenLed();
        yellowLed();
    }

private:
    // �����ڽ������ݴ��뵽������
    char serialBuffer[BUFFER_SIZE]{};
    // Զ�̿��޸ĵĵ���ʱ����
    unsigned char num{};
    // �Ƿ��ں��ģʽ
    bool isRedMode{};

    /// ����ܵ���ʱ
    /// \param time ��ʾ�ĵ���ʱʱ��
    void countdown(unsigned char time) {
        // ����ʱ
        for (short i = time; i > 0; --i) {
            if (hasChanged()) i = num;// ������ֱ��޸�, ��ô���޸ĺ�����ֿ�ʼ����ʱ
            showNumber(i);
            if (isRedMode && analogRead(VOLTS) == 0) { // ���ģʽ,���ҵ�ѹ��Ϊ0
                Serial.println("Someone across the road!");
            }
        }
    }

    /// ���
    void redLed() {
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_Y, LOW);
        isRedMode = true; // ���ģʽ
        countdown(20);
        isRedMode = false; // �Ǻ��ģʽ
    }

    /// �̵�
    void greenLed() {
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_Y, LOW);
        tone(SPEAKER, SPEAKER_FREQ);
        countdown(15);
        noTone(SPEAKER);
    }

    /// �Ƶ�
    void yellowLed() {
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_Y, HIGH);
        countdown(10);
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