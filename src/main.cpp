#include <Arduino.h>
#include <microDS3231.h> //https://github.com/GyverLibs/microDS3231?tab=readme-ov-file#usage
#include <EEPROM.h>
//#include <SoftwareSerial.h>
#include <Wire.h>
#include "smart_home.hpp"
#include "request_from_server.hpp"

const uint8_t PIN_DIR         = 2;  // Пин для переключения режима RS485
const uint8_t RELAY_0         = 6;
const uint8_t RELAY_1         = 7;
const uint8_t RELAY_2         = 8;
const uint8_t MINUTES_IN_HOUR = 60;
const uint8_t MAX_BUF_in      = 11;  // Максимальная длина входного массива
const uint8_t MAX_BUF_out     = 21;  // Максимальная длина выходного массива
uint8_t dataIn[MAX_BUF_in]    = {0}; // Объявляем массив для хранения данных запроса
uint8_t dataOut[MAX_BUF_out]  = {0}; // Объявляем массив для хранения данных ответа

//SoftwareSerial RS485Serial(3, 2); // RX, TX
MicroDS3231 rtc;
SmartHome smart_home(RELAY_0, RELAY_1, RELAY_2);
uint16_t minutes;

void clearEEPROM();
bool receiveBytesFromUART();
void pinModeFast(uint8_t pin, uint8_t mode);
void digitalWriteFast(uint8_t pin, bool x);
void SmartHome_UART();
uint8_t calculate_checksum(const uint8_t *data, uint8_t size);
bool verify_checksum(const uint8_t *data, uint8_t size);

void setup() {
  Serial.begin(9600);
  //RS485Serial.begin(115200);

  //clearEEPROM(); // Затираем EEPROM перед началом работы

  if (!rtc.begin()) {
      delay(60000);
      asm volatile("jmp 0x00");
  }

  // rtc.setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);
  if (rtc.lostPower()) {            // выполнится при сбросе батарейки
      rtc.setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);
  }

  //EEPROM.put(20, smart_home); //
  EEPROM.get(20, smart_home); //

  smart_home.SET_time(rtc.getTime());
  delay(10);
  minutes = smart_home.GET_time().hour * MINUTES_IN_HOUR + smart_home.GET_time().minute;

  pinModeFast(13, OUTPUT);
  digitalWriteFast(13, LOW);

  pinModeFast(PIN_DIR, OUTPUT);
  // в режим получения данных
  digitalWriteFast(PIN_DIR, LOW);
  
  for (uint8_t i = 0; i < 3; ++i) {
    pinModeFast(smart_home.GET_pin(i), OUTPUT);
    digitalWriteFast(smart_home.GET_pin(i), smart_home.GET_status_relay(i));
  }
}

void loop() {
    if (receiveBytesFromUART()) {
      // обработка запроса
      RequestFromServer rfs;
      if (rfs.deserialize(dataIn, MAX_BUF_in - 1)) {
        if (rfs.GET_Type() == RequestType::PING) {
          SmartHome_UART();
        } else if (rfs.GET_Type() == RequestType::PIN_ON) {
          smart_home.SET_on_relay(rfs.GET_relay());
          SmartHome_UART();
        } else if (rfs.GET_Type() == RequestType::PIN_ON_MIN) {
          smart_home.SET_minut_off(rfs.GET_relay(), rfs.GET_mod(),
                                    rfs.GET_hour_off() * MINUTES_IN_HOUR + rfs.GET_min_off());
          SmartHome_UART();
        } else if (rfs.GET_Type() == RequestType::PIN_OFF) {
          smart_home.SET_off_relay(rfs.GET_relay());
          SmartHome_UART();
        } else if (rfs.GET_Type() == RequestType::MOD_ON) {
          smart_home.SET_on_mod(rfs.GET_relay(), rfs.GET_mod());
          SmartHome_UART();
        } else if (rfs.GET_Type() == RequestType::MOD_OFF) {
          smart_home.SET_off_mod(rfs.GET_relay(), rfs.GET_mod());
          SmartHome_UART();
        } else if (rfs.GET_Type() == RequestType::MOD_T) {
          smart_home.SET_minut_on(rfs.GET_relay(), rfs.GET_mod(),
                                  rfs.GET_hour_on() * MINUTES_IN_HOUR + rfs.GET_min_on());
          smart_home.SET_minut_off(rfs.GET_relay(), rfs.GET_mod(),
                                  rfs.GET_hour_off() * MINUTES_IN_HOUR + rfs.GET_min_off());
          SmartHome_UART();
        } else if (rfs.GET_Type() == RequestType::TIME) {
          rtc.setTime(0, rfs.GET_min_on(), rfs.GET_hour_on(), rfs.GET_day(),
                      rfs.GET_month(), rfs.GET_year() + 2000);
          smart_home.SET_time(rtc.getTime());
          SmartHome_UART();
        }
      }
    }

    smart_home.SET_time(rtc.getTime());
    delay(10);
    minutes = smart_home.GET_time().hour * 60 + smart_home.GET_time().minute;
    if (smart_home.Update(minutes)) {
      EEPROM.put(20, smart_home);
    }
    for (uint8_t z = 0; z < 3; ++z) {
      digitalWriteFast(smart_home.GET_pin(z), smart_home.GET_status_relay(z));
    }
}

void SmartHome_UART() {
  smart_home.serialize(dataOut, MAX_BUF_out - static_cast<uint8_t>(1));
  dataOut[MAX_BUF_out - static_cast<uint8_t>(1)] = calculate_checksum(dataOut, MAX_BUF_out - static_cast<uint8_t>(1));
  digitalWriteFast(PIN_DIR,  HIGH);
  Serial.write(dataOut, MAX_BUF_out); // Отправляем данные
  delay(30);
  digitalWriteFast(13,  LOW);
  digitalWriteFast(PIN_DIR,  LOW);
  delay(10);
}

void clearEEPROM() {
    for (uint8_t y = 0; y < EEPROM.length(); y++) {
        EEPROM.write(y, 0); // Записываем 0 в каждый адрес EEPROM
    }
}

bool receiveBytesFromUART() { // получение данных
  uint8_t byteCount = 0;
 
  if (Serial.available() > 0) {
    delay(10);
    while (byteCount < MAX_BUF_in && Serial.available() > 0) {
      int bytesRead = Serial.readBytes(dataIn + byteCount, MAX_BUF_in - byteCount);
      byteCount += bytesRead;
    }
    if (byteCount == MAX_BUF_in) {
      if (verify_checksum(dataIn, MAX_BUF_in)) {
        digitalWriteFast(13, HIGH);
        return true; // Успешно получили и проверили данные
      }
    }
  }
  return false;
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

uint8_t calculate_checksum(const uint8_t *data, uint8_t size) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < size; ++i) {
        checksum ^= data[i]; // Используем XOR для контроля
    }
    return checksum;
}

bool verify_checksum(const uint8_t *data, uint8_t size) {
    if (size < 1) return false; // Если данных недостаточно, контрольная сумма недоступна
    uint8_t received_checksum = data[size - 1]; // Предполагаем, что последний байт — это контрольная сумма
    return received_checksum == calculate_checksum(data, size - 1);
}