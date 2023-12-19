#ifndef my_func_h
#define my_func_h

#include "stm32f4xx_hal.h"

#include <iostream>
#include <string>
#include <string.h>
#include <vector>


using namespace std;

void I2C_send(uint8_t data, uint8_t flags);
void LCD_SendString(char *str);

vector<string> parser(std::string pre_parser);

void print_str(vector<string> parsered_data);
void print_arr(vector<string> parsered_data, double memory_array[]);
double* sum(vector<string> parsered_data, double memory_array[]);
double* diff(vector<string> parsered_data, double memory_array[]);

void read_epr(double memory_array[]);
void save_epr(double memory_array[]);
void read_sd(double memory_array[]);
void save_sd(double memory_array[]);

#endif /* my_func_h */
