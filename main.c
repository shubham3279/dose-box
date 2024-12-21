#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT 32
#define LOGO_WIDTH 70

RTC_DS3231 rtc;

Servo s1;
Servo s2;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int hour_1 = 9;
int min_1 = 10;
const int buzzer = D6;
const int button = D7;

unsigned long previous;
unsigned long current;
int state;
int alarm = 0;

const char* ssid = "realme7";
const char* password = "";
const char* address1 = "http://maker.ifttt.com/trigger/button/with/key/bEuQ_p07_1yZ23wn5K480X";
const char* address2 = "http://maker.ifttt.com/trigger/not_pressed/with/key/bEuQ_p07_1yZ23wn5K480X";
const uint16_t port = 17;

void ifttt(int);
void drawbitmap();

static const unsigned char PROGMEM logo_bmp[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x20, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x0F, 0x80, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x10, 0x00,
    0x1C, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFE, 0x1E, 0x00, 0x38, 0x00,
    0x00, 0x01, 0xFF, 0xFF, 0xC0, 0x00, 0xF0, 0x30, 0x00, 0x00, 0x01,
    0xFF, 0xFF, 0xE0, 0x00, 0xF0, 0x60, 0x00, 0x00, 0x01, 0xFF, 0xFF,
    0xFC, 0x0F, 0x00, 0x60, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x1E,
    0x00, 0xE0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x1E, 0x00, 0xC0,
    0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x3E, 0x00, 0xC0, 0x00, 0x00,
    0x01, 0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0xC0, 0x00, 0x00, 0x01, 0xFF,
    0xFF, 0xFF, 0xBF, 0x00, 0xC0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF,
    0xBF, 0x00, 0xC0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFE, 0x00,
    0xE0, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x60, 0x00,
    0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x70, 0x00, 0x00, 0x01,
    0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x30, 0x00, 0x00, 0x01, 0xFF, 0xFF,
    0xFF, 0xFC, 0x00, 0x38, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xF8,
    0x00, 0x1E, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x0F,
    0x80, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xE0, 0x00, 0x03, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00
};

void setup() {
    pinMode(buzzer, OUTPUT);
    pinMode(button, INPUT);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("!!");
    }
#ifndef ESP8266
    while (!Serial);
#endif
    Serial.begin(9600);
    delay(3000);
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, lets set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    drawbitmap();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F(" MEDICINE DISPENSER "));
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(F("Made By:"));
    display.println(F("Akshat and Shubham "));
    display.display();
    delay(4000);
}

void loop() {
    DateTime now = rtc.now();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print(now.year(), DEC);
    display.print('/');
    display.print(now.month(), DEC);
    display.print('/');
    display.print(now.day(), DEC);
    display.print(" ");
    display.print(daysOfTheWeek[now.dayOfTheWeek()]);
    display.println();
    display.setTextSize(2);
    display.print(now.hour(), DEC);
    display.print(':');
    display.print(now.minute(), DEC);
    display.print(':');
    display.print(now.second(), DEC);
    display.println();
    display.setTextSize(1);
    display.print("Temperature: ");
    display.print(rtc.getTemperature());
    display.println(" C");
    display.display();

    state = digitalRead(button);
    if (state == HIGH) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 1);
        display.println(" Dispensing.....");
        display.display();
        s1.attach(D3);
        s1.write(0);
        delay(1000);
        s1.write(180);
        delay(1000);
        s1.detach();
        display.clearDisplay();
        display.println(" Pill dispensed!!");
        display.display();
        ifttt(1);
        Serial.println("Pill taken!");
    }
    if (hour_1 == now.hour() && min_1 == now.minute() && (now.second() == 0 || now.second() == 1)) {
        alarm = 1;
        previous = millis();
        current = previous;
        while (current - previous < 60000) {
            current = millis();
            state = digitalRead(button);
            digitalWrite(buzzer, HIGH);
            if (state == 1) {
                digitalWrite(buzzer, LOW);
                display.clearDisplay();
                display.setTextSize(1);
                display.setCursor(0, 1);
                display.println(" Dispensing.....");
                display.display();
                s1.attach(D3);
                s1.write(80);
                delay(1000);
                s1.detach();
                display.clearDisplay();
                display.println(" Pill dispensed!!");
                display.display();
                ifttt(1);
                alarm = 0;
                digitalWrite(buzzer, LOW);
                break;
            }
        }
        if (alarm == 1) {
            ifttt(2);
            display.clearDisplay();
            display.setTextSize(1);
            display.setCursor(0, 1);
            display.println("Pill wasn't dispensed!");
            display.display();
        }
    }
    delay(1000);
}

void ifttt(int opt) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient wificlient;
        if (opt == 1) {
            http.begin(wificlient, address1);
        } else if (opt == 2) {
            http.begin(wificlient, address2);
        }
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        int httpResponseCode = http.POST("");
        http.end();
    }
}

void drawbitmap() {
    display.clearDisplay();
    display.drawBitmap((display.width() - LOGO_WIDTH) / 2, (display.height() - LOGO_HEIGHT) / 2, logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.display();
    delay(2000);
}
