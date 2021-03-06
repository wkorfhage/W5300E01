#ifndef __NTKN_GPIO__
#define __NTKN_GPIO__

#include "ntkn_w5300e01.h"

vuint32 *gpio_base;

vuint32 *get_gpio_base();

int gpio_close();

// For print_control_reg
#define RAW_DISPLAY 00
#define GPG_DISPLAY 01
void print_control_reg(int register_contents, int display_type);

void print_data_reg(unsigned int register_contents);

// For set_pin_direction
#define FUNCTION_INPUT 0
#define FUNCTION_OUTPUT 1
void set_pin_direction(unsigned int *control_reg, unsigned int bit, unsigned int function);

void set_pin_data(unsigned int *data_reg, unsigned int bit, unsigned int bit_value);

#endif
