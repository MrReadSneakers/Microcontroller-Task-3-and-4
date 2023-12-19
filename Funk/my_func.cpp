#include "my_func.h"

#define LCD_ADDR (0x27 << 1)       // адрес дисплея, сдвинутый на 1 бит влево (HAL работает с I2C-адресами, сдвинутыми на 1 бит влево)
#define EEPROM_ADDR (0x50 << 1)	   // адрес EEPROM памяти, сдвинутый на 1 бит влево (HAL работает с I2C-адресами, сдвинутыми на 1 бит влево)
#define PIN_RS    (1 << 0)         // если на ножке 0, данные воспринимаются как команда, если 1 - как символы для вывода
#define PIN_EN    (1 << 2)         // бит, по изменению сост. которого считывается информация
#define BACKLIGHT (1 << 3)         // управление подсветкой
#define LCD_DELAY_MS 5             // пауза перед высвечиванием символа


//extern double memory_array[];
extern I2C_HandleTypeDef hi2c1;

// функция для отправки данных, data - сами данные, flags - 1 (отправка данных) или 0 (отправка команд)
void I2C_send(uint8_t data, uint8_t flags)
{
	HAL_StatusTypeDef res;
	    for(;;) {                                                                     // бесконечный цикл
	        res = HAL_I2C_IsDeviceReady(&hi2c1, LCD_ADDR, 1, HAL_MAX_DELAY);          // проверяем, готово ли устройство по адресу lcd_addr для связи
	        if(res == HAL_OK) break;                                                  // если да, то выходим из бесконечного цикла
	    }

	uint8_t up = data & 0xF0;                 // операция И с 1111 0000, приводит к обнулению последних бит с 0 по 3, остаются биты с 4 по 7
	uint8_t lo = (data << 4) & 0xF0;          // тоже самое, но data сдвигается на 4 бита влево, т.е. в этой
	                                           // переменной остаются  биты с 0 по 3
	uint8_t data_arr[4];
	data_arr[0] = up|flags|BACKLIGHT|PIN_EN;  // 4-7 биты содержат информацию, биты 0-3 конфигурируют работу
	data_arr[1] = up|flags|BACKLIGHT;         // ублирование сигнала, на выводе Е в этот раз 0
	data_arr[2] = lo|flags|BACKLIGHT|PIN_EN;
	data_arr[3] = lo|flags|BACKLIGHT;

	HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data_arr, sizeof(data_arr), HAL_MAX_DELAY);
	HAL_Delay(LCD_DELAY_MS);
}


void LCD_SendString(char *str)
{
    // *char по сути является строкой
	while(*str) {                                   // пока строчка не закончится
		I2C_send((uint8_t)(*str), 1);               // передача первого символа строки
        str++;                                      // сдвиг строки налево на 1 символ
    }
}


//парсер. Пока void, но позже над этим подумаю
vector<string> parser(std::string pre_parser)
{
	vector<string> after_parser;

	//поиск команды в введенной строке (че надо сделать: сложить, вычесть, напечатать)
	for (int i = 0; i < pre_parser.size(); i++)
	{
		if ( pre_parser[i] == '(')
		{
			after_parser.push_back( pre_parser.substr(0, i) );

			string source = pre_parser.substr(i+1, pre_parser.size() - (i+2));
			char *s = new char[source.size()+1];
			strcpy(s, source.c_str());
			char *splited_data = strtok(s, ", ");
			while (splited_data != NULL)
			{
				after_parser.push_back(splited_data);
				splited_data = strtok(NULL, ", ");
			}
			delete[] s;

			break;
		}
	}

return after_parser;

}

//функция, выводящая на дисплей строку
void print_str(vector<string> parsered_data)
{
    I2C_send(0b00110000,0);   // 8ми битный интерфейс
    I2C_send(0b00000010,0);   // установка курсора в начале строки
    I2C_send(0b00001100,0);   // нормальный режим работы
    I2C_send(0b00000001,0);   // очистка дисплея

    char *s = new char[parsered_data[2].size() + 1];
	strcpy(s, parsered_data[2].c_str());

	switch (stoi(parsered_data[1]))
	{
		case 1:
			I2C_send(0b10000000,0);   // переход на 1 строку
			LCD_SendString(s);
			break;
		case 2:
		    I2C_send(0b11000000,0);   // переход на 2 строку
			LCD_SendString(s);
			break;
		case 3:
		    I2C_send(0b10010100,0);   // переход на 3 строку
			LCD_SendString(s);
			break;
		case 4:
		    I2C_send(0b11010100,0);   // переход на 4 строку
			LCD_SendString(s);
			break;
	}
}

//функция, выводящая число, записанное в указанной ячейке массива
void print_arr(vector<string> parsered_data, double memory_array[])
{

	I2C_send(0b00110000,0);   // 8ми битный интерфейс
	I2C_send(0b00000010,0);   // установка курсора в начале строки
	I2C_send(0b00001100,0);   // нормальный режим работы
	I2C_send(0b00000001,0);   // очистка дисплея


	char outString[100];
	memset(outString, 0, sizeof(outString));
	sprintf(outString, "arr[%d] = %f", stoi(parsered_data[2]), memory_array[stoi(parsered_data[2])]);

	switch (stoi(parsered_data[1]))
	{
		case 1:
			I2C_send(0b10000000,0);   // переход на 1 строку
			LCD_SendString(outString);
			break;
		case 2:
		    I2C_send(0b11000000,0);   // переход на 2 строку
			LCD_SendString(outString);
			break;
		case 3:
		    I2C_send(0b10010100,0);   // переход на 3 строку
			LCD_SendString(outString);
			break;
		case 4:
		    I2C_send(0b11010100,0);   // переход на 4 строку
			LCD_SendString(outString);
			break;
	}

}

//функция, находящая сумму и записывающая ее в массив
double* sum(vector<string> parsered_data, double memory_array[])
{
	memory_array[stoi(parsered_data[1])] = stod(parsered_data[2]) + stod(parsered_data[3]);
	return memory_array;
}

//функция, находящая разность и записывающая ее в массив
double* diff(vector<string> parsered_data, double memory_array[])
{
	memory_array[stoi(parsered_data[1])] = stod(parsered_data[2]) - stod(parsered_data[3]);
	return memory_array;
}



void read_epr(double memory_array[])
{
	HAL_StatusTypeDef res;
	for(;;)
	{                                                                     		  // бесконечный цикл
		res = HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_ADDR, 1, HAL_MAX_DELAY);          // проверяем, готово ли устройство по адресу lcd_addr для связи
		if(res == HAL_OK) break;                                                  // если да, то выходим из бесконечного цикла
	}

	//цикл для разбиения элементов массива на по двухбитовые группы
	for (int j = 0; j < 10; j++)		//выбор эллемента массива
	{
		for (int k = 0; k < 4; k++)		//запись в двухбитовые слова, котороые потом пойдут на запись
		{
			HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, 8*j, I2C_MEMADD_SIZE_16BIT, (uint8_t *)&memory_array[j], sizeof(memory_array[j]), 100);
			HAL_Delay(10);
		}
	}
}


void save_epr(double memory_array[])
{
	HAL_StatusTypeDef res;
	for(;;)
	{                                                                     		  // бесконечный цикл
		res = HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_ADDR, 1, HAL_MAX_DELAY);          // проверяем, готово ли устройство по адресу lcd_addr для связи
		if(res == HAL_OK) break;                                                  // если да, то выходим из бесконечного цикла
	}

	//цикл для разбиения элементов массива на по двухбитовые группы
	for (int j = 0; j < 10; j++)		//выбор эллемента массива
	{
		for (int k = 0; k < 4; k++)		//запись в двухбитовые слова, котороые потом пойдут на запись
		{
			HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, 8*j, I2C_MEMADD_SIZE_16BIT, (uint8_t *)&memory_array[j], sizeof(memory_array[j]), 100);
			HAL_Delay(10);
		}
	}
}



void read_sd(double memory_array[])
{

}


void save_sd(double memory_array[])
{

}


