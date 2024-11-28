#include "request_from_server.hpp"

RequestType RequestFromServer::GET_Type() const {
    switch(flag) {
        case 0: return RequestType::PING;
        case 1: return RequestType::PIN_ON;
        case 2: return RequestType::PIN_ON_MIN;
        case 3: return RequestType::PIN_OFF;
        case 4: return RequestType::MOD_ON;
        case 5: return RequestType::MOD_OFF;
        case 6: return RequestType::MOD_T;
        case 7: return RequestType::TIME;
        default: return RequestType::NO;
    }
}

bool RequestFromServer::deserialize(const uint8_t* buffer, uint8_t size_buffer) {
    if (size_buffer < 10) {
        return false; // Невозможно десериализовать данные с недостаточным размером буфера
    }
    flag     = buffer[0];
    relay    = buffer[1];
    mod      = buffer[2];
    min_on   = buffer[3];
    hour_on  = buffer[4];
    min_off  = buffer[5];
    hour_off = buffer[6];
    day      = buffer[7];
    month    = buffer[8];
    year     = buffer[9];
    return true;
}