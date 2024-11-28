#include "request_from_server.hpp"

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