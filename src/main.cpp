#include <Arduino.h>
#include <microDS3231.h> //https://github.com/GyverLibs/microDS3231?tab=readme-ov-file#usage
#include <EEPROM.h>
#include <Wire.h>
#include "relay.h"
#include "parser.h"

const uint8_t RELAY_MAX = 3;
const uint8_t RELAY_0 = 6;
const uint8_t RELAY_1 = 7;
const uint8_t RELAY_2 = 8;
const uint8_t MINUTES_IN_HOUR = 60;
const uint8_t MAX_BUF = 40;    // Максимальная длина массива
char dataArray[MAX_BUF];       // Объявляем массив для хранения данных запроса

const char OK_slash_equals[] = "ok/=";
const char OK_slash_slash[] = "ok//;";
const char slash_slash[] = "//;";
const char slash_s[] = "/;";
const char slash[] = "/";
const char ER1[] = "er/1//;";
const char ER2[] = "er/2//;";
const char ER3[] = "er/3//;";

MicroDS3231 rtc;
DateTime date_time;
Relay relay[] = {RELAY_0, RELAY_1, RELAY_2};
uint16_t minutes;

void ShowTime();
void clearEEPROM();
bool ReadingBuffer();
void pinModeFast(uint8_t pin, uint8_t mode);
void digitalWriteFast(uint8_t pin, bool x);

void setup() {
    
    Serial.begin(9600);

    //clearEEPROM(); // Затираем EEPROM перед началом работы

    if (!rtc.begin()) {
        Serial.println(ER1);
        delay(60000);
        asm volatile("jmp 0x00");
    }

    // rtc.setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);
    if (rtc.lostPower()) {            // выполнится при сбросе батарейки
        Serial.println(ER2);
        rtc.setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);
    }

    date_time = rtc.getTime();
    delay(10);
    minutes = (uint16_t)date_time.hour * MINUTES_IN_HOUR + (uint16_t)date_time.minute;
    
    for (uint8_t i = 0; i < RELAY_MAX; ++i) {
        //EEPROM.put(i * sizeof(Relay), relay[i]); //
        EEPROM.get(i * sizeof(Relay), relay[i]); //
        uint8_t pin = relay[i].GET_pin();
        //pinMode(pin, OUTPUT);
        pinModeFast(pin, 1);
        //digitalWrite(pin, relay[i].GET_status_relay());
        digitalWriteFast(pin, (bool)relay[i].GET_status_relay());
    }
    //Serial.println("Start UNO");
    //ShowTime();
}

void loop() {
    if (ReadingBuffer()) {
        Parser pars(dataArray, MAX_BUF);
        if (pars.GET_error()) {
            Parser::Type type = pars.GET_type();
            if (type == Parser::Type::PING) {
                Serial.print(OK_slash_equals);                                  // запрос подключения
                Serial.print(millis());
                Serial.println(slash_slash);
            } else if (type == Parser::Type::GET) {  
                Parser::Type chapter = pars.GET_chapter();
                if (chapter == Parser::Type::TIME) {        
                    ShowTime();                                            // запрос времени
                } else if (chapter == Parser::Type::RELAY) { 
                    Parser::Type subsection_1 = pars.GET_subsection_1();
                    if (subsection_1 == Parser::Type::PIN) { 
                        Serial.print(OK_slash_equals);
                        for (uint8_t i = 0; i < RELAY_MAX; ++i) {         // запрос состояний реле           
                            Serial.print(i);
                            Serial.print(slash);
                            Serial.print(relay[i].GET_status_relay());
                            Serial.print(slash);
                        }
                        Serial.println(slash_s);
                    } else if (subsection_1 == Parser::Type::MOD) {       // запрос настроек реле (номер, режим)
                        uint8_t rel = pars.GET_value(0);
                        uint8_t mod = pars.GET_value(1);
                        uint16_t min_on = relay[rel].GET_minut_on(mod);
                        uint16_t min_off = relay[rel].GET_minut_off(mod);
                        Serial.print(OK_slash_equals);
                        Serial.print(relay[rel].GET_status_on(mod));
                        Serial.print(slash);
                        Serial.print(relay[rel].GET_status_off(mod));
                        Serial.print(slash);
                        Serial.println(relay[rel].GET_status_repeat(mod));
                        Serial.print(slash);
                        Serial.print(min_on / MINUTES_IN_HOUR);
                        Serial.print(slash);
                        Serial.print(min_on % MINUTES_IN_HOUR);
                        Serial.print(slash);
                        Serial.print(min_off / MINUTES_IN_HOUR);
                        Serial.print(slash);
                        Serial.print(min_off % MINUTES_IN_HOUR);
                        Serial.println(slash_slash);
                    }
                }
            } else if (type == Parser::Type::POST) {
                Parser::Type chapter = pars.GET_chapter();
                if (chapter == Parser::Type::TIME) {                       // установка времени
                    uint8_t sec_p = pars.GET_value(0);
                    uint8_t min_p = pars.GET_value(1);
                    uint8_t hour_p = pars.GET_value(2);
                    uint8_t day_p = pars.GET_value(3);
                    uint8_t month_p = pars.GET_value(4);
                    uint8_t year_p = pars.GET_value(5);
                    rtc.setTime(sec_p, min_p, hour_p, day_p, month_p, year_p - 48);
                    Serial.println(OK_slash_slash);
                } else if (chapter == Parser::Type::RELAY) {
                    Parser::Type subsection_1 = pars.GET_subsection_1();
                    if (subsection_1 == Parser::Type::PIN) {              // включить-выключить
                        uint8_t relay_p = pars.GET_value(0);
                        if (pars.GET_value(1)) {
                            relay[relay_p].SET_on_relay();
                            Serial.println(OK_slash_slash);
                        } else {
                            relay[relay_p].SET_off_relay();
                            Serial.println(OK_slash_slash);
                        }
                    } else if (subsection_1 == Parser::Type::MOD) {
                        Parser::Type subsection_2 = pars.GET_subsection_2();
                        if (subsection_2 == Parser::Type::ON) {              // режим включения
                            uint8_t relay_p = pars.GET_value(0);
                            uint8_t mod_p = pars.GET_value(1);
                            uint8_t min_p = pars.GET_value(2);
                            uint8_t hour_p = pars.GET_value(3);
                            relay[relay_p].SET_minut_on(mod_p, hour_p * MINUTES_IN_HOUR + min_p);
                            relay[relay_p].SET_on_on(mod_p);
                            relay[relay_p].SET_on_off(mod_p);
                            if (pars.GET_value(4)) {
                                relay[relay_p].SET_on_repeat(mod_p);
                                Serial.println(OK_slash_slash);
                            } else {
                                relay[relay_p].SET_off_repeat(mod_p);
                                Serial.println(OK_slash_slash);
                            }
                            if (pars.GET_value(5)) {
                                relay[relay_p].SET_on_relay();
                                Serial.println(OK_slash_slash);
                            }
                        } else if (subsection_2 == Parser::Type::OFF) {      // режим выключения
                            uint8_t relay_p = pars.GET_value(0);
                            uint8_t mod_p = pars.GET_value(1);
                            uint8_t min_p = pars.GET_value(2);
                            uint8_t hour_p = pars.GET_value(3);
                            relay[relay_p].SET_minut_off(mod_p, hour_p * MINUTES_IN_HOUR + min_p);
                            relay[relay_p].SET_on_on(mod_p);
                            relay[relay_p].SET_on_off(mod_p);
                            if (pars.GET_value(4)) {
                                relay[relay_p].SET_on_repeat(mod_p);
                                Serial.println(OK_slash_slash);
                            } else {
                                relay[relay_p].SET_off_repeat(mod_p);
                                Serial.println(OK_slash_slash);
                            }
                            if (pars.GET_value(5)) {
                                relay[relay_p].SET_on_relay();
                                Serial.println(OK_slash_slash);
                            }
                        } else if (subsection_2 == Parser::Type::REPIAT) {   // активация-деактивация повтора
                            uint8_t relay_p = pars.GET_value(0);
                            if (pars.GET_value(2)) {
                                relay[relay_p].SET_on_repeat(pars.GET_value(1));
                                Serial.println(OK_slash_slash);
                            } else {
                                relay[relay_p].SET_off_repeat(pars.GET_value(1));
                                Serial.println(OK_slash_slash);
                            }
                        } else if (subsection_2 == Parser::Type::ACTIV) {    // активация-деактивация режима
                            uint8_t relay_p = pars.GET_value(0);
                            uint8_t mod_p = pars.GET_value(1);
                            if (pars.GET_value(2)) {
                                relay[relay_p].SET_on_on(mod_p);
                                relay[relay_p].SET_on_off(mod_p);
                                Serial.println(OK_slash_slash);
                            } else {
                                relay[relay_p].SET_off_on(mod_p);
                                relay[relay_p].SET_off_off(mod_p);
                                Serial.println(OK_slash_slash);
                            }
                        }
                    }
                }
            } else if (type == Parser::Type::ERROR) {
                Serial.println(ER3);
            }
        }
        
    }
    date_time = rtc.getTime();
    delay(10);
    minutes = (uint16_t)date_time.hour * 60 + (uint16_t)date_time.minute;
    for (uint8_t z = 0; z < RELAY_MAX; ++z) {
        if (relay[z].Update(minutes)) {
            EEPROM.put(z * sizeof(Relay), relay[z]);
        }
        //digitalWrite(relay[z].GET_pin(), relay[z].GET_status_relay());
        digitalWriteFast(relay[z].GET_pin(), (bool)relay[z].GET_status_relay());
    }
}

void clearEEPROM() {
    for (uint8_t y = 0; y < EEPROM.length(); y++) {
        EEPROM.write(y, 0); // Записываем 0 в каждый адрес EEPROM
    }
}

void ShowTime() {
    Serial.print(OK_slash_equals);
    Serial.print(date_time.second);
    Serial.print(slash);
    Serial.print(date_time.minute);
    Serial.print(slash);
    Serial.print(date_time.hour);
    Serial.print(slash);
    Serial.print(date_time.date);
    Serial.print(slash);
    Serial.print(date_time.month);
    Serial.print(slash);
    Serial.print(date_time.year);
    Serial.println(slash_slash);
}

bool ReadingBuffer() {
    if (Serial.available()) {
        uint8_t amount = Serial.readBytesUntil(';', dataArray, MAX_BUF);
        dataArray[amount] = '\0';
        return true;
    } else {
        return false;
    }
}

void pinModeFast(uint8_t pin, uint8_t mode) {
  switch (mode) {
    case INPUT:
      if (pin < 8) {
        bitClear(DDRD, pin);    
        bitClear(PORTD, pin);
      } else if (pin < 14) {
        bitClear(DDRB, (pin - 8));
        bitClear(PORTB, (pin - 8));
      } else if (pin < 20) {
        bitClear(DDRC, (pin - 14));   // Mode: INPUT
        bitClear(PORTC, (pin - 14));  // State: LOW
      }
      return;
    case OUTPUT:
      if (pin < 8) {
        bitSet(DDRD, pin);
        bitClear(PORTD, pin);
      } else if (pin < 14) {
        bitSet(DDRB, (pin - 8));
        bitClear(PORTB, (pin - 8));
      } else if (pin < 20) {
        bitSet(DDRC, (pin - 14));  // Mode: OUTPUT
        bitClear(PORTC, (pin - 14));  // State: LOW
      }
      return;
    case INPUT_PULLUP:
      if (pin < 8) {
        bitClear(DDRD, pin);
        bitSet(PORTD, pin);
      } else if (pin < 14) {
        bitClear(DDRB, (pin - 8));
        bitSet(PORTB, (pin - 8));
      } else if (pin < 20) {
        bitClear(DDRC, (pin - 14));  // Mode: OUTPUT
        bitSet(PORTC, (pin - 14));  // State: HIGH
      }
      return;
  }
}

void digitalWriteFast(uint8_t pin, bool x) {
  // раскомментируй, чтобы отключать таймер<br>  
  /*switch (pin) {            
    case 3: bitClear(TCCR2A, COM2B1);
      break;
    case 5: bitClear(TCCR0A, COM0B1);
      break;
    case 6: bitClear(TCCR0A, COM0A1);
      break;
    case 9: bitClear(TCCR1A, COM1A1);
      break;
    case 10: bitClear(TCCR1A, COM1B1);
      break;
    case 11: bitClear(TCCR2A, COM2A1);   // PWM disable 
      break;
  }*/
  if (pin < 8) {
    bitWrite(PORTD, pin, x);
  } else if (pin < 14) {
    bitWrite(PORTB, (pin - 8), x);
  } else if (pin < 20) {
    bitWrite(PORTC, (pin - 14), x);    // Set pin to HIGH / LOW 
  }
}