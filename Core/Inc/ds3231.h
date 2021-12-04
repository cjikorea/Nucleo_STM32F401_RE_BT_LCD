#ifndef DS3231_H

#define DS3231_H



#include "stm32f4xx_hal.h"



#if 1



#define DS3231_addr    0XAE// 0xD0 // I2C 7-bit slave address shifted for 1 bit to the left

#define DS3231_seconds  0x00 // DS3231 seconds address

#define DS3231_control  0x0E // DS3231 control register address

#define DS3231_tmp_MSB  0x11 // DS3231 temperature MSB



// All DS3231 registers

typedef struct {

	uint8_t seconds;

	uint8_t minutes;

	uint8_t hours;

	uint8_t day;

	uint8_t date;

	uint8_t month;

	uint8_t year;

	uint8_t alarm1_secconds;

	uint8_t alarm1_minutes;

	uint8_t alarm1_hours;

	uint8_t alarm1_day;

	uint8_t alarm1_date;

	uint8_t alarm2_minutes;

	uint8_t alarm2_hours;

	uint8_t alarm2_day;

	uint8_t alarm2_date;

	uint8_t control;

	uint8_t status;

	uint8_t aging;

	uint8_t msb_temp;

	uint8_t lsb_temp;

} DS3231_registers_TypeDef;



// DS3231 date

typedef struct {

	uint8_t seconds;

	uint8_t minutes;

	uint8_t hours;

	uint8_t day_of_week;

	uint8_t day;

	uint8_t month;

	uint8_t year;

} DS3231_date_TypeDef;



// Human Readable Format date

typedef struct {

	uint8_t  Seconds;

	uint8_t  Minutes;

	uint8_t  Hours;

	uint8_t  Day;

	uint8_t  Month;

	uint16_t Year;

	uint8_t  DOW;

} HRF_date_TypeDef;



void DisplayRTCTime(void);

#endif



#define DS3231_ADDR			0xAE//0x57//0b11010000

enum {

	DS3231_SEC = 0x00,

	DS3231_MIN,

	DS3231_HOUR,

	DS3231_WDAY,

	DS3231_DATE,

	DS3231_MONTH,

	DS3231_YEAR

};



void ds3231ReadTime(void);

uint8_t ds3231GetTime(uint8_t tm);

void WriteRTCTime(struct tm timeinfo);

time_t ReadRTCTimeT(void);

void RTC_GetTime(unsigned char *buf);



#endif
