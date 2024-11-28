#pragma once

#include <Arduino.h>
#include <microDS3231.h>

const uint8_t BIT_7 = 128;    // 0b1000 0000
const uint8_t BIT_6 = 64;     // 0b0100 0000
const uint8_t BIT_5 = 32;     // 0b0010 0000
const uint8_t BIT_4 = 16;     // 0b0001 0000
const uint8_t BIT_3 = 8;      // 0b0000 1000
const uint8_t BIT_2 = 4;      // 0b0000 0100
const uint8_t BIT_1 = 2;      // 0b0000 0010

class SmartHome {
    private:
        // флаги
        // [7] изменения в классе
        // [6] статус relay 0
        // [5] статус relay 1
        // [4] статус relay 2
        // [3] статус relay 1 мод
        // [2] статус relay 2 мод 0
        // [1] статус relay 2 мод 1
        // [0]
        uint8_t flag = 0;

        uint8_t relay_0;
        uint8_t relay_1;
        uint8_t relay_2;

        // время на плате
        uint8_t second = 0; 
        uint8_t minute = 0;
        uint8_t hour   = 0;
        uint8_t date   = 0;
        uint8_t month  = 0;
        uint8_t year   = 0; // без 2000

        // relay 1
        uint16_t minute_off_relay_1 = 0;

        // relay 2
        uint16_t minute_on_relay_2_mod_0 = 0;
        uint16_t minute_off_relay_2_mod_0 = 0;
        uint16_t minute_on_relay_2_mod_1 = 0;
        uint16_t minute_off_relay_2_mod_1 = 0;
    public:
        SmartHome(uint8_t pin_0, uint8_t pin_1, uint8_t pin_2);
        // Запрос статуса реле
        bool GET_status_relay(uint8_t relay) const;
        // Запрос наличия изменений
        bool GET_status_changes() const;
        // Запрос статуса мод
        bool GET_status_mod(uint8_t relay, uint8_t mod) const;
        // Активация реле
        void SET_on_relay(uint8_t relay);
        // Деактивация реле
        void SET_off_relay(uint8_t relay);
        // Активация мод
        void SET_on_mod(uint8_t relay, uint8_t mod);
        // Деактивация мод
        void SET_off_mod(uint8_t relay, uint8_t mod);
        // Запрос пина
        char GET_pin(uint8_t relay) const;
        // Запрос времени <on>
        uint16_t GET_minut_on(uint8_t relay, uint8_t mod) const;
        // Запрос времени <off>
        uint16_t GET_minut_off(uint8_t relay, uint8_t mod) const;
        // Установка времени <on>
        void SET_minut_on(uint8_t relay, uint8_t mod, uint16_t min);
        // Установка времени <off>
        void SET_minut_off(uint8_t relay, uint8_t mod, uint16_t min);
        // Обновление состояния (функция принимает текущие время
        // и возвращает false если изменений не было)
        bool Update(uint16_t & current_time);
        // Сериализация в бинарный формат
        bool serialize(uint8_t * buffer, uint8_t size_buffer) const;
        void SET_time(const DateTime& date_time);
        DateTime GET_time() const;
};