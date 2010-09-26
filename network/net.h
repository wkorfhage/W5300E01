#include <sys/mman.h>

#define W5300BASE_ADDRESS 0x56000000
#define GPGCON_OFFSET (0x60/4)
#define GPGDAT_OFFSET (0x64/4)
#define GPGUP_OFFSET (0x68/4)

unsigned int *gpio_base;

unsigned int *get_gpio_base();
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
