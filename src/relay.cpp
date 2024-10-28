#include <Arduino.h>

#include "relay.h"

Relay::Relay(const Relay & r) {
    pin_flag = r.pin_flag;
    for (uint8_t i = 0; i < MODS; ++i) {
        mods[i] = r.mods[i]; // Копируем режимы
    }
}

Relay & Relay::operator=(const Relay & r) {
    if (this != &r) {
        pin_flag = r.pin_flag;
        for (uint8_t i = 0; i < MODS; ++i) {
            mods[i] = r.mods[i]; // Копируем режимы
        }
    }
return *this;
}

// Запрос статуса реле
uint8_t Relay::GET_status_relay() const {
    return (pin_flag & BIT_0);
}

// Запрос наличия изменений
uint8_t Relay::GET_status_changes() const {
    return ((pin_flag & BIT_1) >> 1);
}

// Запрос статуса <repeat>
uint8_t Relay::GET_status_repeat(const uint8_t & mod) const {
    return (mods[mod].mod_flag & BIT_0);
}

// Запрос статуса <on>
uint8_t Relay::GET_status_on(const uint8_t & mod) const {
    return ((mods[mod].mod_flag & BIT_1) >> 1);
}

uint8_t Relay::GET_status_off(const uint8_t & mod) const {
    return ((mods[mod].mod_flag & BIT_2) >> 2);
}

// Активация реле
void Relay::SET_on_relay() {
    if (!GET_status_relay()){
        pin_flag |= BIT_0; // Активируем реле
        pin_flag |= BIT_1; // Изменения в классе
    }
}

void Relay::SET_off_relay() {
    if (GET_status_relay()){
        pin_flag &= ~BIT_0; // Деактивируем реле
        pin_flag |= BIT_1; // Изменения в классе
    }
}

// Активация <repeat>
void Relay::SET_on_repeat(const uint8_t & mod) {
    if (!GET_status_repeat(mod)){
        mods[mod].mod_flag |= BIT_0; // Активируем <repeat>
        pin_flag |= BIT_1;           // Изменения в классе
    }
}

void Relay::SET_off_repeat(const uint8_t & mod) {
    if (GET_status_repeat(mod)){
        mods[mod].mod_flag &= ~BIT_0; // Деактивируем <repeat>
        pin_flag |= BIT_1;            // Изменения в классе
    }
}

// Активация <on>
void Relay::SET_on_on(const uint8_t & mod) {
    if (!GET_status_on(mod)) {
        mods[mod].mod_flag |= BIT_1;  // Активируем <on>
        pin_flag |= BIT_1;            // Изменения в классе
    }
}

// Деактивация <on>
void Relay::SET_off_on(const uint8_t & mod) {
    if (GET_status_on(mod)) {
        mods[mod].mod_flag &= ~BIT_1; // Деактивируем <on>
        pin_flag |= BIT_1;           // Изменения в классе
        Serial.println("5");
    }
}

void Relay::SET_on_off(const uint8_t & mod) {
    if (!GET_status_off(mod)) {
        mods[mod].mod_flag |= BIT_2;  // Активируем <off>
        pin_flag |= BIT_1;            // Изменения в классе
    }
}

void Relay::SET_off_off(const uint8_t & mod) {
    if (GET_status_off(mod)) {
        mods[mod].mod_flag &= ~BIT_2; // Деактивируем <off>
        pin_flag |= BIT_1;           // Изменения в классе
    }
}

// Установка времени <on>
void Relay::SET_minut_on(const uint8_t & mod, uint16_t min_on) {
    if (min_on < 1440) {
        mods[mod].minut_on = min_on;
        pin_flag |= BIT_1;           // Изменения в классе
    }
}

// Установка времени <off>
void Relay::SET_minut_off(const uint8_t & mod, uint16_t min_off) {
    if (min_off < 1440) {
        mods[mod].minut_off = min_off;
        pin_flag |= BIT_1;           // Изменения в классе
    }
}

uint8_t Relay::Update(uint16_t & current_time) {
    for (uint8_t i = 0; i < MODS; ++i) {
        // Если режим <on> активен и время совпало
        if (GET_status_on(i) && mods[i].minut_on == current_time) {
            if (GET_status_repeat(i)) { // Если режим <repeat> активен
                SET_on_relay();
            } else { // Если режим <repeat> не активен
                SET_on_relay();
                SET_on_off(i);
            }
        }
        // Если режим <off> активен и время совпало
        if (GET_status_off(i) && mods[i].minut_off == current_time) {
            if (GET_status_repeat(i)) {
                SET_off_relay();
            } else { // Если режим <repeat> не активен
                SET_off_relay();
                SET_off_off(i);
            }
        }
    }
    uint8_t ch = GET_status_changes();
    pin_flag &= ~BIT_1; // изменений больше нет
    return ch;
}