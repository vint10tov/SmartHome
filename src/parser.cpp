#include <Arduino.h>
#include "parser.h"

Parser::Parser(char * buf, uint8_t sizebuf) {
    while (i < sizebuf && buf[i] != '/' && buf[i] != '=' && error) {
        iterat_buf(buf, sizebuf);
        TypeDefinition();
    }
    if (buf[i] == '/' && error) {
        return;
    } else if (buf[i] == '=' && error) {
        ++i;
        while (i < sizebuf && buf[i] != '/' && error) {
            iterat_buf(buf, sizebuf);
            ValueUint();
        }
    }
    if (buf[i] != '/')
        error = false;
}

void Parser::iterat_buf(char * b, uint8_t s) {
    uint8_t z = 0;
    while (i < s && b[i] != '/' && z < SIZE_TEMP_BUF - 1) {
        temp[z] = b[i];
        ++i;
                ++z;
    }
    temp[z] = '\0';
    if (b[i] != '/' || z == 0)
        error = false;
    ++i;
}

void Parser::ValueUint() {
    if (value_counter < SIZE_VALUE) {
        value[value_counter] = atoi(temp);
        ++value_counter;
    } else {
        error = false;
    }
}

void Parser::TypeDefinition () {
    if (strcmp("gt", temp) == 0) {
        type = Parser::Type::GET;
    } else if (strcmp("pt", temp) == 0) {
        type = Parser::Type::POST;
    } else if (strcmp("pg", temp) == 0) {
        type = Parser::Type::PING;
    } else if (strcmp("tm", temp) == 0) {
        chapter = Parser::Type::TIME;
    } else if (strcmp("rl", temp) == 0) {
        chapter = Parser::Type::RELAY;
    } else if (strcmp("pn", temp) == 0) {
        subsection_1 = Parser::Type::PIN;
    } else if (strcmp("md", temp) == 0) {
        subsection_1 = Parser::Type::MOD;
    } else if (strcmp("on", temp) == 0) {
        subsection_2 = Parser::Type::ON;
    } else if (strcmp("of", temp) == 0) {
        subsection_2 = Parser::Type::OFF;
    } else if (strcmp("rt", temp) == 0) {
        subsection_2 = Parser::Type::REPIAT;
    } else if (strcmp("ac", temp) == 0) {
        subsection_2 = Parser::Type::ACTIV;
    } else 
        error = false;
}

