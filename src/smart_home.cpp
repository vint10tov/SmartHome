#include "smart_home.hpp"

SmartHome::SmartHome(uint8_t pin_0, uint8_t pin_1, uint8_t pin_2) {
    relay_0 = pin_0;
    relay_1 = pin_1;
    relay_2 = pin_2;
}

// Запрос статуса реле
bool SmartHome::GET_status_relay(uint8_t relay) const {
    if (relay == 0)
        return (flag & BIT_6);
    else if (relay == 1)
        return (flag & BIT_5);
    else if (relay == 2)
        return (flag & BIT_4);
    else
        return false;
}

// Запрос наличия изменений
bool SmartHome::GET_status_changes() const {
    return (flag & BIT_7);
}

// Запрос статуса мод
bool SmartHome::GET_status_mod(uint8_t relay, uint8_t mod) const {
    if (relay == 1)
        return (flag & BIT_3);
    else if (relay == 2 && mod == 0)
        return (flag & BIT_2);
    else if (relay == 2 && mod == 1)
        return (flag & BIT_1);
    else
        return false;
}

// Активация реле
void SmartHome::SET_on_relay(uint8_t relay) {
    if (!GET_status_relay(relay)){
        if (relay == 0) {
            flag |= BIT_6; // Активируем реле 0
            flag |= BIT_7; // Изменения в классе
        } else if (relay == 1) {
            flag |= BIT_5; // Активируем реле 1
            flag |= BIT_7; // Изменения в классе
        } else if (relay == 2) {
            flag |= BIT_4; // Активируем реле 2
            flag |= BIT_7; // Изменения в классе
        }
    }
}

// Деактивация реле
void SmartHome::SET_off_relay(uint8_t relay) {
    if (GET_status_relay(relay)){
        if (relay == 0) {
            flag &= ~BIT_6; // Деактивируем реле 0
            flag |= BIT_7;  // Изменения в классе
        } else if (relay == 1) {
            flag &= ~BIT_5; // Деактивируем реле 1
            flag |= BIT_7;  // Изменения в классе
        } else if (relay == 2) {
            flag &= ~BIT_4; // Деактивируем реле 2
            flag |= BIT_7;  // Изменения в классе
        }
    }
}

// Активация мод
void SmartHome::SET_on_mod(uint8_t relay, uint8_t mod) {
    if (!GET_status_mod(relay, mod)) {
        if (relay == 1) {
            flag |= BIT_3; // Активируем реле 1 mod
            flag |= BIT_7; // Изменения в классе
        }
        else if (relay == 2 && mod == 0) {
            flag |= BIT_2; // Активируем реле 2 mod 0
            flag |= BIT_7; // Изменения в классе
        }
        else if (relay == 2 && mod == 1) {
            flag |= BIT_1; // Активируем реле 2 mod 1
            flag |= BIT_7; // Изменения в классе
        }
    }
}

// Деактивация мод
void SmartHome::SET_off_mod(uint8_t relay, uint8_t mod) {
    if (GET_status_mod(relay, mod)) {
        if (relay == 1) {
            flag &= ~BIT_3; // Деактивируем реле 1 mod
            flag |= BIT_7;  // Изменения в классе
        }
        else if (relay == 2 && mod == 0) {
            flag &= ~BIT_2; // Деактивируем реле 2 mod 0
            flag |= BIT_7;  // Изменения в классе
        }
        else if (relay == 2 && mod == 1) {
            flag &= ~BIT_1; // Деактивируем реле 2 mod 1
            flag |= BIT_7;  // Изменения в классе
        }
    }
}

// Запрос пина
char SmartHome::GET_pin(uint8_t relay) const {
    if (relay == 0)
        return relay_0;
    else if (relay == 1)
        return relay_1;
    else if (relay == 2)
        return relay_2;
    else
        return 0;
}

// Запрос времени <on>
uint16_t SmartHome::GET_minut_on(uint8_t relay, uint8_t mod) const {
    if (relay == 2 && mod == 0) {
        return minute_on_relay_2_mod_0;
    }
    else if (relay == 2 && mod == 1) {
        return minute_on_relay_2_mod_1;
    } else {
        return 0;
    }
}

// Запрос времени <off>
uint16_t SmartHome::GET_minut_off(uint8_t relay, uint8_t mod) const {
    if (relay == 1) {
        return minute_off_relay_1;
    }
    else if (relay == 2 && mod == 0) {
        return minute_off_relay_2_mod_0;
    }
    else if (relay == 2 && mod == 1) {
        return minute_off_relay_2_mod_1;
    } else {
        return 0;
    }
}

// Установка времени <on>
void SmartHome::SET_minut_on(uint8_t relay, uint8_t mod, uint16_t min) {
    if (relay == 2 && mod == 0) {
        minute_on_relay_2_mod_0 = min;
        flag |= BIT_7;  // Изменения в классе
    }
    else if (relay == 2 && mod == 1) {
        minute_on_relay_2_mod_1 = min;
        flag |= BIT_7;  // Изменения в классе
    }
}

// Установка времени <off>
void SmartHome::SET_minut_off(uint8_t relay, uint8_t mod, uint16_t min) {
    if (relay == 1) {
        SET_off_mod(relay, mod);
        SET_on_relay(relay);
        minute_off_relay_1 = min;
        flag |= BIT_7;  // Изменения в классе
    }
    else if (relay == 2 && mod == 0) {
        minute_off_relay_2_mod_0 = min;
        flag |= BIT_7;  // Изменения в классе
    }
    else if (relay == 2 && mod == 1) {
        minute_off_relay_2_mod_1 = min;
        flag |= BIT_7;  // Изменения в классе
    }
}

// Обновление состояния (функция принимает текущие время
// и возвращает false если изменений не было)
bool SmartHome::Update(uint16_t & current_time) {
    // Если реле 1 ативно и время совпало
    if (GET_status_mod(1, 0) && minute_off_relay_1 == current_time) {
        SET_off_relay(1);
        SET_off_mod(1, 0);
    } else if (GET_status_mod(2, 0)) {
        if (minute_on_relay_2_mod_0 == current_time) {
            SET_on_relay(2);
        } else if (minute_off_relay_2_mod_0 == current_time) {
            SET_off_relay(2);
        }
    } else if (GET_status_mod(2, 1)) {
        if (minute_on_relay_2_mod_1 == current_time) {
            SET_on_relay(2);
        } else if (minute_off_relay_2_mod_1 == current_time) {
            SET_off_relay(2);
        }
    }
    bool ch = GET_status_changes();
    flag &= ~BIT_6; // изменений больше нет
    return ch;
}

bool SmartHome::serialize(uint8_t *buffer, uint8_t size_buffer) const {
    if (size_buffer < 20) { // Минимальный размер буфера для сериализации всех полей
        return false;       // Невозможно сериализовать данные с недостаточным размером буфера
    }
    buffer[0] = flag;
    buffer[1] = relay_0;
    buffer[2] = relay_1;
    buffer[3] = relay_2;
    buffer[4] = second;
    buffer[5] = minute;
    buffer[6] = hour;
    buffer[7] = date;
    buffer[8] = month;
    buffer[9] = year;
    buffer[10] = minute_off_relay_1 & 0xFF;              // младший байт
    buffer[11] = (minute_off_relay_1 >> 8) & 0xFF;       // старший байт
    buffer[12] = minute_on_relay_2_mod_0 & 0xFF;         // младший байт
    buffer[13] = (minute_on_relay_2_mod_0 >> 8) & 0xFF;  // старший байт
    buffer[14] = minute_off_relay_2_mod_0 & 0xFF;        // младший байт
    buffer[15] = (minute_off_relay_2_mod_0 >> 8) & 0xFF; // старший байт
    buffer[16] = minute_on_relay_2_mod_1 & 0xFF;         // младший байт
    buffer[17] = (minute_on_relay_2_mod_1 >> 8) & 0xFF;  // старший байт
    buffer[18] = minute_off_relay_2_mod_1 & 0xFF;        // младший байт
    buffer[19] = (minute_off_relay_2_mod_1 >> 8) & 0xFF; // старший байт
    return true;
}

void SmartHome::SET_time(const DateTime& dateTime) {
    second = dateTime.second;
    minute = dateTime.minute;
    hour   = dateTime.hour;
    date   = dateTime.date;
    month  = dateTime.month;
    year   = dateTime.year - 2000; // без 2000
}

DateTime SmartHome::GET_time() const {
    DateTime dateTime;
    dateTime.second = second;
    dateTime.minute = minute;
    dateTime.hour   = hour;
    dateTime.date   = date;
    dateTime.month  = month;
    dateTime.year   = year + 2000;
    return dateTime;
}