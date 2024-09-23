#include <stdlib.h>
#include <memory.h>
#include "JZModbusDefine.h"

//JZModbusMapping
JZModbusMapping::JZModbusMapping()
{
    memset(this, 0, sizeof(JZModbusMapping));
}

JZModbusMapping::~JZModbusMapping()
{
    clear();
}

void JZModbusMapping::init(int bits_count, int input_bits_count,
    int registers_count, int input_registers_count)
{
    clear();

    /* 0X */
    nb_bits = bits_count;
    start_bits = 0;
    if (nb_bits != 0) {
        tab_bits = (uint8_t *)malloc(nb_bits * sizeof(uint8_t));
        memset(tab_bits, 0, nb_bits * sizeof(uint8_t));
    }

    /* 1X */
    nb_input_bits = input_bits_count;
    start_input_bits = 0;
    if (nb_input_bits != 0) {

        tab_input_bits =
            (uint8_t *)malloc(nb_input_bits * sizeof(uint8_t));
        memset(tab_input_bits, 0, nb_input_bits * sizeof(uint8_t));
    }

    /* 4X */
    nb_registers = registers_count;
    start_registers = start_registers;
    if (nb_registers != 0) {
        tab_registers =
            (uint16_t *)malloc(nb_registers * sizeof(uint16_t));
        memset(tab_registers, 0, nb_registers * sizeof(uint16_t));
    }

    /* 3X */
    nb_input_registers = input_registers_count;
    start_input_registers = start_input_registers;
    if (nb_input_registers != 0) {
        tab_input_registers =
            (uint16_t *)malloc(nb_input_registers * sizeof(uint16_t));
        memset(tab_input_registers, 0,
            nb_input_registers * sizeof(uint16_t));
    }
}

void JZModbusMapping::clear()
{
    if (tab_input_registers)
        free(tab_input_registers);
    if (tab_registers)
        free(tab_registers);
    if (tab_input_bits)
        free(tab_input_bits);
    if (tab_bits)
        free(tab_bits);
    memset(this, 0, sizeof(JZModbusMapping));
}