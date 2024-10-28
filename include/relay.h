#pragma once

const uint8_t BIT_0 = 1; // 0b00000001
const uint8_t BIT_1 = 2; // 0b00000010
const uint8_t BIT_2 = 4; // 0b00000100

const uint8_t MODS = 2; // количество режимов

class Relay {
    public:
        struct  Mods {
            // [0] - флаг активации режима <repeat>
            // [1] - флаг активации режима <on>
            // [2] - флаг активации режима <off>
            uint8_t mod_flag = 0; // флаги состояния
            uint16_t minut_on = 0;  // время <on>
            uint16_t minut_off = 0; // время <off>
        };
    private: 
        // [0] - флаг активности
        // [1] - флаг изменений
        uint8_t pin_flag = 0; // флаги состояния
        uint8_t pin;      // пин
        Mods mods[MODS];  // массив режимов
    public:
        Relay(uint8_t pin_r) {pin = pin;}

        Relay(const Relay & r);
        Relay & operator=(const Relay & r);

         // Запрос статуса реле
        uint8_t GET_status_relay() const;
         // Запрос наличия изменений
        uint8_t GET_status_changes() const;
        // Запрос статуса <repeat>
        uint8_t GET_status_repeat(const uint8_t & mod) const;
         // Запрос статуса <on>
        uint8_t GET_status_on(const uint8_t & mod) const;
         // Запрос статуса <off>
        uint8_t GET_status_off(const uint8_t & mod) const;
         // Активация реле
        void SET_on_relay();
         // Деактивация реле
        void SET_off_relay();
        // Активация <repeat>
        void SET_on_repeat(const uint8_t & mod);
        // Деактивация <repeat>
        void SET_off_repeat(const uint8_t & mod);
        // Активация <on>
        void SET_on_on(const uint8_t & mod);
        // Деактивация <on>
        void SET_off_on(const uint8_t & mod);
         // Активация <off>
        void SET_on_off(const uint8_t & mod);
         // Деактивация <off>
        void SET_off_off(const uint8_t & mod);
        // Запрос пина
        uint8_t GET_pin() const {return pin;}
        // Запрос времени <on>
        uint16_t GET_minut_on(const uint8_t & mod) const {
            return mods[mod].minut_on;
        }
        // Запрос времени <off>
        uint16_t GET_minut_off(const uint8_t & mod) const {
            return mods[mod].minut_off;
        }
        // Установка времени <on>
        void SET_minut_on(const uint8_t & mod, uint16_t min_on);
        // Установка времени <off>
        void SET_minut_off(const uint8_t & mod, uint16_t min_off);
        // Обновление состояния пина и флагов (функция принимает текущие время
        // и возвращает 0 если изменений не было)
        uint8_t Update(uint16_t & current_time);
};
