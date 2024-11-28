#pragma once

#include <Arduino.h>

enum class RequestType {PING, PIN_ON, PIN_ON_MIN, PIN_OFF,
                        MOD_ON, MOD_OFF, MOD_T, TIME, NO};

class RequestFromServer {
    private:
        uint8_t flag;
        uint8_t relay;
        uint8_t mod;
        uint8_t min_on;
        uint8_t hour_on;
        uint8_t min_off;
        uint8_t hour_off;
        uint8_t day;
        uint8_t month;
        uint8_t year;
    public:
        RequestFromServer() {}
        RequestType GET_Type() const;
        uint8_t GET_relay() const {return relay;}
        uint8_t GET_mod() const {return mod;}
        uint8_t GET_min_on() const {return min_on;}
        uint8_t GET_hour_on() const {return hour_on;}
        uint8_t GET_min_off() const {return min_off;}
        uint8_t GET_hour_off() const {return hour_off;}
        uint8_t GET_day() const {return day;}
        uint8_t GET_month() const {return month;}
        uint8_t GET_year() const {return year;}
        bool deserialize(const uint8_t* buffer, uint8_t size_buffer);
};