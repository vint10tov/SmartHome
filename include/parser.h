#pragma once

#include <string.h>
#include <stdlib.h>

const uint8_t SIZE_TEMP_BUF = 8;
const uint8_t SIZE_VALUE = 6;

class Parser {
    public:

        enum class Type {GET, POST, PING, TIME, RELAY, PIN, MOD, ON, OFF, REPIAT, ACTIV, ERROR};

    private:

        bool error = true; // ошибки при парсинге - false    
        char temp[SIZE_TEMP_BUF]; // буфер для разбиения поступившего массива
        uint8_t i = 0; // позиция в массиве
        uint8_t value_counter = 0; // количество значений VALUE

        uint8_t value[SIZE_VALUE] {0};
        Parser::Type type = Parser::Type::ERROR;
        Parser::Type chapter = Parser::Type::ERROR;
        Parser::Type subsection_1 = Parser::Type::ERROR;
        Parser::Type subsection_2 = Parser::Type::ERROR;

        // разбиение массива на строки, возвращает false если есть ощибки
        void iterat_buf(char * b, uint8_t s);

        // 
        void ValueUint();
        void TypeDefinition ();

    public:
        
        Parser(char * buf, uint8_t sizebuf);

        bool GET_error() const {return error;}
        Parser::Type GET_type() const {return type;}
        Parser::Type GET_chapter() {return chapter;}
        Parser::Type GET_subsection_1() {return subsection_1;}
        Parser::Type GET_subsection_2() {return subsection_2;}
        uint8_t GET_value(uint8_t i) {return value[i];}
};
