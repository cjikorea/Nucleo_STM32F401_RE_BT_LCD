/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_bluenrg_2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
//LCD
#include "snow_tiger.h"
#include "BD663474.h"
#include "img_define.h"
#include "lcd_gfx.h"
//clock
#include "ds3231.h"
#include "fonts.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI3_Init(void);
static void MX_I2C3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define DISP_MODE_ALL 		0
#define DISP_MODE_TIME		1
#define DISP_MODE_CAR		2
#define DISP_MODE_F14		3
#define DISP_MODE_BALLOON	4
#define DISP_MODE_3D_IMG    5

//BackGrount Img
#define DISP_BG_ALL		1
#define DISP_BG_TIME	2
#define DISP_BG_CAR		3
#define DISP_BG_F14		4
#define DISP_BG_3D_IMG		5
#define DISP_BG_BALLOON		6


#define _ON_			1
#define _OFF_			0

/* colors */
#define BLACK      				0x0000
#define NAVY        			0x000F
#define DARKGREEN   			0x03E0
#define DARKCYAN    			0x03EF
#define MAROON      			0x7800
#define PURPLE      			0x780F
#define OLIVE       			0x7BE0
#define LIGHTGREY   			0xC618
#define DARKGREY    			0x7BEF
#define BLUE        			0x001F
#define GREEN       			0x07E0
#define CYAN        			0x07FF
#define RED         			0xF800
#define MAGENTA     			0xF81F
#define YELLOW      			0xFFE0
#define WHITE       			0xFFFF
#define ORANGE      			0xFD20
#define GREENYELLOW 			0xAFE5
#define PINK        			0xF81F


int m_nRedraw = 1;
int nDispMode = DISP_MODE_ALL;
int nDispModeOld = -1;
int nDelayDispCnt = 0;

int nHourOld = -1;
int nMinuteOld = 0;
int nSecondOld = -1;
int nFlicker = 0;
char cLogData[100];
int nBarColor = 1;
int nDayOld = -1;
int nBGImgTypeOld = -1;

//int setDisplayModeCmd =DISP_MODE_TIME;//DISP_MODE_ALL;
//Select main
int setDisplayModeCmd = DISP_MODE_ALL;//DISP_MODE_BALLOON;//DISP_MODE_TIME;//DISP_MODE_ALL;

int setLEDControl = 0;

#define MSG_SIZE 			1024
#define UART_RX_BUFFER_SIZE 1024
#define MAX_SPI_READ 		0x5000 // 1024�???????????????????????? data
#define FLASH_STATUS_BUSY	0x01
#define DS3231_ADDRESS 		0xD0

unsigned char buffer[5];				//for HDC1080 on the I2C
unsigned int rawTemp;
unsigned int rawHumi;
float HDC1080_Temp;
float HDC1080_Temp_Old;
float HDC1080_Humi;
float HDC1080_Humi_Old;
int nRedraw_Temp_Label = 0;
int nRedraw_Humi_Label = 0;
int temp_max = -99;
int temp_min = 99;
int humi_max = -99;
int humi_min = 99;

TIME time;
nSetTime = 0;

uint8_t decToBcd(int val)
{
	return (uint8_t)((val/10*16) + (val%10));
}
int bcdToDec(uint8_t val)
{
	return (int)((val/16*10) + (val%16));
}



void DrawMainWindow(int nType)
{

	m_nRedraw = 0;//reset

	if( nType == DISP_MODE_ALL)
	{
		 Get_Time();
		 TFT_setColor(TFT_COLOR_Orange);
		 TFT_clear();
/*
		 TFT_setCursor(5,50);
		 TFT_drawImage(65,65, IMG_Weather_Channel);


		 TFT_setCursor(80,55);
		 TFT_drawImage(64,64, IMG_C);

		 TFT_setCursor(80,135);
		 TFT_drawImage(64,64, IMG_h);

		 TFT_setCursor(5,128);
		 TFT_drawImage(65,104, IMG_f22);

		 TFT_setCursor(80,203);

		 TFT_drawImage(220,31, bitmap_3);

		 //Draw rectangle
		 TFT_drawRectangle(80, 50, 220, 150, 1, TFT_COLOR_Fuchsia);
*/
	}

	if( nType == DISP_BG_TIME)
	{
		//TFT_drawImage(320,240, Date_BG);
		//TFT_drawRoundRect(5, 5, 320-5, 240-5, 5/*round*/, 3, TFT_COLOR_Silver);
	}

	//
	if( nType == DISP_MODE_CAR)
	{
		// TFT_drawImage(320,240, IMG_BG);
	}
	//DISP_MODE_F14
	//
	if( nType == DISP_MODE_F14)
	{
		 TFT_drawImage(320,240, IMG_F14);
	}

	if( nType == DISP_MODE_BALLOON)
	{
		 TFT_drawImage(320,240, IMG_BALLOON);
	}



}



void Set_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month,uint8_t year)
{
	uint8_t set_time[7];

	set_time[0] = decToBcd(sec);
	set_time[1] = decToBcd(min);
	set_time[2] = decToBcd(hour);
	set_time[3] = decToBcd(dow);//day of week (1 ~ 7 )
	set_time[4] = decToBcd(dom);//day of month(1 ~ 31)
	set_time[5] = decToBcd(month);
	set_time[6] = decToBcd(year);

	HAL_I2C_Mem_Write(&hi2c3, DS3231_ADDRESS, 0x00,1, set_time, 7, 1000);

}
void Get_Time(void)
{
	//Time Setting
	if(nSetTime == 1 ) return;

	uint8_t get_time[7];
	HAL_I2C_Mem_Read(&hi2c3, DS3231_ADDRESS, 0x00, 1, get_time, 7,1000);

	time.seconds = bcdToDec(get_time[0]);
	time.minutes = bcdToDec(get_time[1]);
	time.hour = bcdToDec(get_time[2]);
	time.dayofweek = bcdToDec(get_time[3]);
	time.dayofmonth = bcdToDec(get_time[4]);
	time.month = bcdToDec(get_time[5]);
	time.year = bcdToDec(get_time[6]);

	//HAL_I2C_Master_Transmit(&hi2c1, 0xB0, buffer, 1, 1000);

	// HAL_Delay(20);

	//HAL_I2C_Master_Receive(&hi2c1, 0xB0, &buffer[2], 2, 100);

}

void MainBGImageDraw(int nType)
{
	if(nBGImgTypeOld != nType)
	{
		nBGImgTypeOld = nType;

		//redraw
		 nHourOld = -1;
		 nMinuteOld = -1;
		 nDayOld = -1;

	}
	else return;

	if(nType == DISP_BG_ALL)
	{
		DrawMainWindow(DISP_MODE_ALL);
	}
	else if(nType == DISP_BG_TIME)
	{
		DrawMainWindow(DISP_BG_TIME);
	}
	else if(nType == DISP_BG_CAR)
	{
		DrawMainWindow(DISP_MODE_CAR);
	}
	else if(nType == DISP_BG_F14)
	{
		DrawMainWindow(DISP_MODE_F14);
	}
	else if(nType == DISP_BG_3D_IMG)
		{
			DrawMainWindow(DISP_MODE_BALLOON);
		}
}


//Main LCD DISPLAY SETTING By Mode
void MainLCDDisplayMode(int nDispMode)
{
	   if(nDispMode != nDispModeOld)
	   {
		   //redraw
		   nHourOld = -1.0;
		   nMinuteOld = -1.0;
		   HDC1080_Temp_Old = -1.0;
		   HDC1080_Humi_Old = -1.0;

		   nDispModeOld = nDispMode;
		   TFT_clear();

		 if(nDispMode == DISP_MODE_ALL )
		 {
			 MainBGImageDraw(DISP_BG_ALL);
			 DrawMainWindow(DISP_MODE_ALL);
		 }

		 if(nDispMode == DISP_MODE_TIME )
		 {
			 //MainBGImageDraw(DISP_BG_TIME);
		 }

		 if(nDispMode == DISP_MODE_CAR )
		 {
			 MainBGImageDraw(DISP_BG_CAR);
		 }

		 if(nDispMode == DISP_MODE_F14 )
		 {
			 MainBGImageDraw(DISP_BG_F14);
		 }

		 if(nDispMode == DISP_MODE_BALLOON )
		 {
			 MainBGImageDraw(DISP_BG_3D_IMG);
		 }

	   }

	    // Temp + Humi + Time
		if( nDispMode == DISP_MODE_ALL)
		{
			 MeasureTempHumi();

			 //get current time / day
			 Get_Time();

			 TFT_setTextBackColor(TFT_COLOR_Black);
			 TFT_setColor(TFT_COLOR_Orange);

			 //TFT_setFontSize(2);
			 //TFT_print(5,10, "DEV :");
			 TFT_setFontSize(2);
			 sprintf(cLogData, "%02d/%02d/%02d", time.year,time.month,time.dayofmonth);
			 //sprintf(cLogData, "%04d/%02d/%02d", 2021,2,24,13,23);
			 TFT_print(20,20, cLogData);

			 TFT_setFontSize(3);
			 TFT_setColor(TFT_COLOR_White);
			 //TFT_setTextBackColor(TFT_COLOR_Teal);

			 int hour12 = (int)time.hour;
			 if(hour12 > 12 )
			 {
				 hour12 = hour12 - 12;
			 }

			 sprintf(cLogData, "%02d:%02d:%02d", hour12, time.minutes, time.seconds);
			 //sprintf(cLogData, "%02d:%02d:%02d", 13,23,57);
			 TFT_print(150,15, cLogData);

			 ///BIG HOUR================================================================
			 //TFT_setFontSize(8);
			 //sprintf(cLogData, "%02d", hour12);
			 //TFT_print(20,70, cLogData);

			 //int hour12 = (int)time.hour;
			 if(hour12 >= 12 )
			 {
				 hour12 = hour12 - 12;
			 }

			 if(hour12 == 0) hour12 =12;

			 //int nHourOld = 0;
			 //int nMinuteOld = 0;

			 if(hour12 != nHourOld )
			 {
				 nHourOld = hour12;

				 sprintf(cLogData, "%02d", hour12);
				 //TFT_print(pos_x+20, pos_y,cLogData);
				 LCD_DrawText(cLogData, FONT9, 15, 80, TFT_COLOR_White, TFT_COLOR_clear);

				 /////////////////////////////////////////////////////////////////////////////
				 TFT_drawLineHorizontal(20,140,80,1,TFT_COLOR_Silver);
				 TFT_drawLine(100,140,115,115,1,TFT_COLOR_Silver);
				 /////////////////////////////////////////////////////////////////////////////

				 /////////////////////////////////////////////////////////////////////////////

				 TFT_fillRectangle(99,95, 35,30, TFT_COLOR_Gray); //fill rect bg

				 TFT_drawLine(110,100, 105,115,2,TFT_COLOR_Silver);  // / \ l
				 TFT_drawLine(110,100, 115,115,2,TFT_COLOR_Silver);  // / \ l
				 TFT_drawLine(125,100, 125,115,2,TFT_COLOR_Silver);  // / \ l

				 //////////////////////////////////////////////////////////////////////////////////

				 /////////////////////////////////////////////////////////////////////////////
				 TFT_drawLineHorizontal(20,220,80,1,TFT_COLOR_Silver);
				 TFT_drawLine(100,220,115,195,1,TFT_COLOR_Silver);
				 /////////////////////////////////////////////////////////////////////////////

				 TFT_fillRectangle(99,175, 35,32, TFT_COLOR_Gray); //fill rect bg

				 TFT_drawLine(105,180, 105,190,2,TFT_COLOR_Silver);  // / l
				 TFT_drawLine(125,180, 125,190,2,TFT_COLOR_Silver);  // / l
				 TFT_drawLine(105,185, 125,185,2,TFT_COLOR_Silver);  // / l
				 TFT_drawLine(105,190, 125,190,2,TFT_COLOR_Silver);  // / l

				 TFT_drawLine(105,193, 125,193,2,TFT_COLOR_Silver);  // / l

				 TFT_drawLine(115,195, 115,198,2,TFT_COLOR_Silver);  // / l

				 TFT_drawLine(105,197, 105,202,2,TFT_COLOR_Silver);  // / l
				 TFT_drawLine(105,202, 125,202,2,TFT_COLOR_Silver);  // / l

				 //sprintf(cLogData, "-");
				 //TFT_print(pos_x+20, pos_y,cLogData);
				 //LCD_DrawText(cLogData, FONT9, 20, 130, TFT_COLOR_White, TFT_COLOR_clear);
				 //LCD_DrawText(cLogData, FONT9, 40, 130, TFT_COLOR_White, TFT_COLOR_clear);

				 TFT_setColor(TFT_COLOR_Silver);
				 TFT_setTextBackColor(TFT_COLOR_Gray);
				 TFT_setFontSize(2);
				 TFT_print(22,50, "Time");

			 }
			 //========================================================================

			 ///BIG MINUTE
			 //TFT_setFontSize(8);
			 //sprintf(cLogData, "%02d", time.minutes);
			 //TFT_print(20,150, cLogData);
			 if(time.minutes != nMinuteOld)
			 {
				 //nHourOld = hour12;
				 nMinuteOld = time.minutes;

				 sprintf(cLogData, "%02d", (int)time.minutes);
				 //TFT_print(pos_x+140, pos_y,cLogData);
				LCD_DrawText(cLogData, FONT9, 15, 160, TFT_COLOR_White, TFT_COLOR_clear);
			 }

			 //========================================================================

			 TFT_setColor(TFT_COLOR_Green_Light);

			  TFT_setFontSize(2);
			  //sprintf(cLogData, "??");//??  =>  °
			  //TFT_print(247,95, cLogData);
			  TFT_setFontSize(3);
			  sprintf(cLogData, "C");
			  TFT_print(255,90, cLogData);

			  TFT_setFontSize(4);

			  sprintf(cLogData, "%.1f", (float)HDC1080_Temp);
			  TFT_setTextBackColor(TFT_COLOR_Gray);
			  //TFT_print(100,160, "T"); //TFT_print(10,30, "Temp:");

			  TFT_setColor(TFT_COLOR_Green_Light);
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  TFT_print(150,80, cLogData);

			  TFT_setColor(TFT_COLOR_Aqua);
			  sprintf(cLogData, "%.1f", (float)HDC1080_Humi);
			  TFT_setTextBackColor(TFT_COLOR_Gray);
			 // TFT_print(100,160, "H:");//TFT_print(10,60, "Humi:");

			  TFT_setColor(TFT_COLOR_Aqua);
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  TFT_print(150,160, cLogData);

			  TFT_setFontSize(3);
			  sprintf(cLogData, "%%");
			  TFT_print(255,165, cLogData);

			  //
			  TFT_setColor(TFT_COLOR_Silver);
			  TFT_setTextBackColor(TFT_COLOR_Gray);
			  TFT_setFontSize(2);
			  TFT_print(150,50, "Temp");
			  TFT_print(150,135, "Humi");

			  TFT_setTextBackColor(TFT_COLOR_Black);

			 //TEST CODE
			 TFT_setCursor(140,155);

		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if( nDispMode == DISP_MODE_TIME)
		{
			//delay time for redraw
			//if(nDelayDispCnt++ < 5) return;
			//else  nDelayDispCnt = 0;

			Get_Time();

			 TFT_setColor(TFT_COLOR_Orange);
			 // TFT_clear();

			 //TFT_setFontSize(2);
			 //TFT_print(5,10, "DEV :");
			 TFT_setFontSize(3);

			 int year = (int)time.year;
			 year = year;
			// year = 2000 + year;
			 if(time.dayofmonth != nDayOld)
			 {
				 nDayOld = time.dayofmonth;
				 //sprintf(cLogData, "%02d-%02d",time.month, time.dayofmonth);
				 sprintf(cLogData, "%02d.%02d.%02d", year, time.month, time.dayofmonth);
				 //sprintf(cLogData, "%04d.%02d.%02d", year, time.month, time.dayofmonth);

				 //LCD_DrawText(cLogData, Digital, 60, 135, TFT_COLOR_Orange, TFT_COLOR_Black);
				 //LCD_DrawText(cLogData, FONT10, 60, 143, TFT_COLOR_Orange, TFT_COLOR_Gray);
				 LCD_DrawText(cLogData, FONT6, 60, 13, TFT_COLOR_Silver, TFT_COLOR_none);

			 }

			 //TFT_print(30,35, cLogData);

			TFT_setFontSize(7);
			TFT_setColor(TFT_COLOR_White);
			 //TFT_setTextBackColor(TFT_COLOR_Teal);

			//sprintf(cLogData, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);

			 int hour12 = (int)time.hour;
			 if(hour12 >= 12 )
			 {
				 hour12 = hour12 - 12;
			 }

			 if(hour12 == 0) hour12 =12;

			 //int nHourOld = 0;
			 //int nMinuteOld = 0;

			 int nHour_Px = 50;
			 int nHour_py = 52;

			 int Minute_Px = 170;
			 int nMinute_py = 52;

			 if(hour12 != nHourOld )
			 {
				 nHourOld = hour12;

				 sprintf(cLogData, "%02d", hour12);
				 //LCD_DrawText(cLogData, Digital, 30, 60, TFT_COLOR_White, TFT_COLOR_Black);
				 LCD_DrawText(cLogData, FONT9, nHour_Px, nHour_py, TFT_COLOR_Silver, TFT_COLOR_Black);

				 sprintf(cLogData, ":");
				 //LCD_DrawText(cLogData, Digital, 108, 60, TFT_COLOR_White, TFT_COLOR_Black);
				 LCD_DrawText(cLogData, FONT9, 130, 60, TFT_COLOR_Silver, TFT_COLOR_Black);

				 TFT_setFontSize(2);
				 if(time.hour < 12)
				 {
					 sprintf(cLogData, "AM");
					 LCD_DrawText(cLogData, FONT6, 255, 74, TFT_COLOR_Silver, TFT_COLOR_none);

				 }

				 else
				 {
					 sprintf(cLogData, "PM");
					LCD_DrawText(cLogData, FONT6, 255, 74, TFT_COLOR_Silver, TFT_COLOR_none);

				 }

			 }

			 if(time.minutes != nMinuteOld)
			 {
				 //nHourOld = hour12;
				 nMinuteOld = time.minutes;

				 sprintf(cLogData, "%02d", time.minutes);
				 //LCD_DrawText(cLogData, Digital, 150, 60, TFT_COLOR_White, TFT_COLOR_Black);
				 LCD_DrawText(cLogData, FONT9, Minute_Px, nMinute_py, TFT_COLOR_Silver, TFT_COLOR_Black);

				 if(nBarColor == 1)  nBarColor = 2;
				 else if(nBarColor == 2)  nBarColor = 3;
				 else if(nBarColor == 3)  nBarColor = 1;
				 //else if(nBarColor == 3)  nBarColor = 0;

				 //TFT_fillRectangle(10,220,310,10,TFT_RGB(0,0,0));

			 }
			 if(time.seconds == 0 )
			 {
				 int x_s = 0;
				 int x_e = 60;
				 int x_p_s = 20+(x_s*4);
				 int x_p_e = 20+(x_e*4);
				 int y_p = (240/2) - 20; //13

				 //Clear Whole 60sec Bar of progress
				 TFT_fillRectangle(
						 x_p_s,/*x*/
						 y_p, //(240/2) - 5 - 1,//221,/*y*/
						 x_p_e-x_p_s+50,/*width*/
						 15,//65,/*height */
						 TFT_RGB(0,0,0));  /* color */
			 }

			 //TFT_print(30,100, cLogData);

			 TFT_setFontSize(2);
			 // sprintf(cLogData, "%02d", time.seconds);
			 //TFT_print(250,135, cLogData);

			 int R = 0;// (int)4*nSecondOld;
			 int G = 0; // (int)4*nSecondOld;
			 int B = 0; // (int)4*nSecondOld;
			 int RB,GB,BB;

			 if( nSecondOld != time.seconds)// && time.seconds != 0)
			 {
				 nFlicker = !nFlicker;

				 if(1)//nFlicker == 1)
					 sprintf(cLogData, ":");
				 else
					 sprintf(cLogData, " ");

				 //LCD_DrawText(cLogData, FONT9, 120, 115, TFT_COLOR_White, TFT_COLOR_Black);

				 //nSecondOld = time.seconds;
				 TFT_setFontSize(8);
				 //sprintf(cLogData, "a", time.seconds);
				 //sprintf(cLogData, "%02d ", time.seconds);
				 //LCD_DrawText(cLogData, FONT9, 50, 150, TFT_COLOR_White, TFT_COLOR_Black);
				 //draw_string("H", 100, 160, FONT3, 54, 64, 32, 127, 0, 0, 1);

				 //
				 sprintf(cLogData, "|");

				 //if(time.seconds == -1) nSecondOld = 0;
				 if(time.seconds == 0) nSecondOld = 0;

				 if(0)//nBarColor == 0)
				 {

					 R = (int)4*nSecondOld;
					 G =  (int)4*nSecondOld;
					 B =  (int)4*nSecondOld;
				 }

				 if(nBarColor == 1)
				 {
					 R = (int)4*nSecondOld;
					 G = 0; // (int)4*nSecondOld;
					 B = 0; // (int)4*nSecondOld;
					 if(time.seconds == 1)  R = 250;
				 }

				 if(nBarColor == 2)
				 {
					 R = 0;//(int)4*nSecondOld;
					 G = (int)4*nSecondOld;
					 B = 0; // (int)4*nSecondOld;
					 if(time.seconds == 1) G = 250;
				 }
				 if(nBarColor == 3)
				 {
					 R = 0;//(int)4*nSecondOld;
					 G = 0;//(int)4*nSecondOld;
					 B =  (int)4*nSecondOld;
					 if(time.seconds == 1) B = 250;
				 }
				 if(time.seconds == 1 )
				 {

					 //R = 250;
					 //G = 250;
					 //B = 250;
					 int hold = 0;
				 }

				 int color = 0;
				 if(time.seconds != 0 )
				 {
					 color = nSecondOld%7;
				 }
				 else
				 {
					 color = 0;
					 //R = 0;//(int)4*nSecondOld;
					 //G = 0;//(int)4*nSecondOld;
					 //B = 0;//  (int)4*nSecondOld;
				 }
				 //void TFT_drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint16_t color)
				 //TFT_fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {


				 if( time.seconds != 0) //nSecondOld != 0)
				 {
					 if(R != 0 )RB = 250;
					 else RB = 0;

					 if(G != 0 )GB = 250;
					 else GB = 0;

					 if(B != 0 )BB = 250;
					 else BB = 0;
				 }

				 //TFT_drawRectangle(20,220,260,12,1,TFT_RGB(RB,GB,BB));

				 //bottom bar pre fill draw
				 if(0)// nSecondOld == -1 )
				 {

					 TFT_fillRectangle(
							 20+(time.seconds*4)/*x*/,
							 221/*y*/,
							 15 /*width*/,
							 65,/*height */
							 TFT_RGB(R,G,B));  /* color */

				 }

				 if( time.seconds != 0)
				 {

					 ////////////////////////////////
					 //// 60sec bar
					 ////////////////////////////////
					 int x_p = 20+(time.seconds*4);
					 int y_p = (240/2) - 13;

					 if(time.seconds != 1)
					 {
						 TFT_fillRectangle(
							 x_p,/*x*/
							 y_p,//221,/*y*/
							 15,/*width*/
							 8,//65,/*height */
							 TFT_RGB(R,G,B));  /* color */
					 }
					 //LCD_DrawText(cLogData, FONT6, 0+(nSecondOld*5), 190, TFT_RGB(R,G,B) , TFT_COLOR_Black);
					 TFT_setFontSize(2);
					 int nColor = TFT_RGB(RB,GB,BB);
					 TFT_setColor(nColor);
					 sprintf(cLogData, " %02d", time.seconds);
					 TFT_print(x_p + 15, y_p - 6, cLogData);
					// LCD_DrawText(cLogData, FONT6, x_p + 5, y_p, TFT_COLOR_Silver, TFT_COLOR_none);

				 }

				 if(time.seconds == 0)
				 {
					 nSecondOld = 0; //
				 }
				 else
				 {
					 nSecondOld = time.seconds;
				 }
			 }

			  TFT_setFontSize(2);

			  int nTemp;

			  nTemp = (int)HDC1080_Temp;

			  if(nTemp != HDC1080_Temp_Old)
			  {
				  nRedraw_Temp_Label = 1;

				  if(nTemp<0 )nTemp = 0;

				  HDC1080_Temp_Old = nTemp;
				  sprintf(cLogData, "%2d", nTemp);

				  if(nTemp >= 20 && nTemp <= 26  )
					  LCD_DrawText(cLogData, FONT9, 38, 155, TFT_COLOR_Green_Light, TFT_COLOR_none);
				  if(nTemp > 26)
				  	  LCD_DrawText(cLogData, FONT9, 38, 155, TFT_COLOR_Red, TFT_COLOR_none);
				  if(nTemp < 20)
				  	   LCD_DrawText(cLogData, FONT9, 38, 155, TFT_COLOR_Blue, TFT_COLOR_none);

				  sprintf(cLogData, "C");
				  LCD_DrawText(cLogData, FONT6, 123, 175, TFT_COLOR_Silver, TFT_COLOR_none);

			  }else nRedraw_Temp_Label = 0;

			  //TFT_setTextBackColor(TFT_COLOR_Gray);
			  //TFT_print(100,160, "T"); //TFT_print(10,30, "Temp:");


			  TFT_setColor(TFT_COLOR_Green_Light);
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  //TFT_print(62,190, cLogData);

			  TFT_setColor(TFT_COLOR_Silver);
			  sprintf(cLogData, "/");
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  //TFT_print(145,190, cLogData);
			 // TFT_print(100,160, "H:");//TFT_print(10,60, "Humi:");


			  TFT_setColor(TFT_COLOR_Aqua);
			  int n_humi = (int)HDC1080_Humi;

			  if(n_humi != (int)HDC1080_Humi_Old)
			  {
				  nRedraw_Humi_Label = 1;
				  HDC1080_Humi_Old = n_humi;
				  sprintf(cLogData, "%2d", (int)HDC1080_Humi);

				  if(n_humi >= 30 && n_humi <= 70  )
					  LCD_DrawText(cLogData, FONT9, 190, 155, TFT_COLOR_Green_Light, TFT_COLOR_none);
				  if (n_humi > 70)
				  	  LCD_DrawText(cLogData, FONT9, 190, 155, TFT_COLOR_Red, TFT_COLOR_none);
				  if (n_humi < 30)
				  	 LCD_DrawText(cLogData, FONT9, 190, 155, TFT_COLOR_Blue, TFT_COLOR_none);

				  sprintf(cLogData, "%%");
				  LCD_DrawText(cLogData, FONT6, 275, 175, TFT_COLOR_Silver, TFT_COLOR_none);
			  }
			  else nRedraw_Humi_Label = 0;

			  TFT_setTextBackColor(TFT_COLOR_Gray);
			 // TFT_print(100,160, "H:");//TFT_print(10,60, "Humi:");

			  TFT_setColor(TFT_COLOR_Aqua);
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  //TFT_print(165,190, cLogData);

			  int offset = 5;
			  //  ----
			  //TFT_drawLine(offset, 240/2, (320-offset)+3, (240/2), 1, TFT_COLOR_Silver);

			  //  T
			  //TFT_drawLine((320-offset)/2, 240/2, (320-offset)/2, (240-2), 1, TFT_COLOR_Silver);

			  //[  ] up
			  TFT_drawRoundRect(offset, offset, 320-offset, (240/2)-offset, 9/*corner round*/, 3, TFT_COLOR_Silver);

			  // [  ] screen
			  //TFT_drawRoundRect(offset, offset, 320-offset, 240-offset, 10/*corner round*/, 1, TFT_COLOR_Silver);



			  //-------------------------------------------------------------------------------------
			  if(nRedraw_Temp_Label == 1)
			  {
				  //temp - axies
				  TFT_drawLine(12, 125, 12, 170, 3, TFT_RGB(200,20,20));
				  TFT_drawLine(12, 170, 12, 200, 3, TFT_COLOR_Green_Light1);//20~26
				  TFT_drawLine(12, 200, 12, 225, 3, TFT_RGB(20,20,200)); //

				  int temp_y = (int)HDC1080_Temp;
				  //int temp_y_old = (int)HDC1080_Temp_Old;

				  int nToValueBar =0;

				  if(temp_y >= 20) nToValueBar= (200) - ((temp_y - 20) * 5);
				  else nToValueBar= (200) + ((20 - temp_y) * 5);

				  if(nToValueBar < 125 ) nToValueBar = 125;//min
				  if(nToValueBar > 225 ) nToValueBar = 225;//max

				  //clear old line
				  TFT_fillRectangle(17,121,20,113,TFT_COLOR_Black);
				  //TFT_drawLine(10, 225-temp_y_old, 10+10, 225-temp_y_old, 1, TFT_COLOR_none);

				  //label axis 15
				  TFT_setCursor(16,122);
				  //TFT_drawImage(23,15, IMG_num_35);
				  //label axis 35
				  TFT_setCursor(16,216);
				 // TFT_drawImage(23,15, IMG_num_15);

				  TFT_drawTriangle(17, nToValueBar, 17+10,nToValueBar+5, 17+10,nToValueBar-5,2,TFT_COLOR_White);
				  //TFT_drawLine(14, nToValueBar, 10+10, nToValueBar, 1, TFT_COLOR_White);

				  TFT_setFontSize(1);
				 if(temp_max < temp_y)
					 temp_max = temp_y;
				 if(temp_min > temp_y)
					 temp_min = temp_y;

				 if( nRedraw_Temp_Label == 1)
				 {
					 sprintf(cLogData, "Min:");
					 TFT_print(38,225, cLogData);

					 TFT_setFontSize(2);
					 sprintf(cLogData, "%d", temp_min);
					 TFT_print(65,220, cLogData);

					 TFT_setFontSize(1);
					 sprintf(cLogData, "Max:");
					 TFT_print(95,225, cLogData);

					 TFT_setFontSize(2);
					 sprintf(cLogData, "%d", temp_max);
					 TFT_print(122,220, cLogData);
				 }


			  }

			  //-------------------------------------------------------------------------------------

			  if(nRedraw_Humi_Label == 1)
			  {
				  //humi - axies
				  TFT_drawLine(164, 125, 164, 155, 3, TFT_RGB(200,20,20));
				  TFT_drawLine(164, 155, 164, 195, 3, TFT_COLOR_Green_Light1);	//30~70%
				  //TFT_drawLine(164, 175, 164, 195, 3, TFT_COLOR_Green_Light1);//
				  TFT_drawLine(164, 195, 164, 225, 3, TFT_RGB(20,20,200));//

				  int humi_y = (int)HDC1080_Humi;
				  //int temp_y_old = (int)HDC1080_Temp_Old;

				  int nToValueBar2 =0;

				  nToValueBar2 = 225 - humi_y;

				  //clear old line
				  TFT_fillRectangle(167,121,20,113,TFT_COLOR_Black);
				  //TFT_drawLine(10, 225-temp_y_old, 10+10, 225-temp_y_old, 1, TFT_COLOR_none);

				  //label axis 100
				  TFT_setCursor(168,122);
				 // TFT_drawImage(23,15, IMG_num_100);
				  //label axis 0
				  TFT_setCursor(168,216);
				 // TFT_drawImage(23,15, IMG_num_0);

				  TFT_drawTriangle(168, nToValueBar2, 168+10,nToValueBar2+5, 168+10,nToValueBar2-5,2,TFT_COLOR_White);
				  //TFT_drawLine(14, nToValueBar, 10+10, nToValueBar, 1, TFT_COLOR_White);

				  TFT_setFontSize(1);
				 if(humi_max < humi_y)
					 humi_max = humi_y;
				 if(humi_min > humi_y)
					 humi_min = humi_y;

				 int minmax_offset = 145;

				 if( nRedraw_Humi_Label == 1)
				 {
					 sprintf(cLogData, "Min:");
					 TFT_print(40 + minmax_offset,225, cLogData);

					 TFT_setFontSize(2);
					 sprintf(cLogData, "%d", humi_min);
					 TFT_print(67 + minmax_offset,220, cLogData);

					 TFT_setFontSize(1);
					 sprintf(cLogData, "Max:");
					 TFT_print(100 + minmax_offset,225, cLogData);

					 TFT_setFontSize(2);
					 sprintf(cLogData, "%d", humi_max);
					 TFT_print(130 + minmax_offset,220, cLogData);
				 }

			  }
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if( nDispMode == DISP_MODE_CAR ||
				nDispMode == DISP_MODE_F14)
		{
			 Get_Time();

			 TFT_setColor(TFT_COLOR_Orange);

			 int pos_x = 0;
			 int pos_y = 150;//175;

			 TFT_setFontSize(2);
/*
			 int year = (int)time.year;
			 year = 2000 + year;
			 if(time.dayofmonth != nDayOld)
			 {
				 nDayOld = time.dayofmonth;
				 sprintf(cLogData, "%04d.%02d.%02d", year, time.month, time.dayofmonth);
				 LCD_DrawText(cLogData, FONT2, 200, 50, TFT_COLOR_Orange, TFT_COLOR_Black);
			 }

*/
			 //TFT_print(30,35, cLogData);

			TFT_setFontSize(7);
			TFT_setColor(TFT_COLOR_White);
			 //TFT_setTextBackColor(TFT_COLOR_Teal);

			//sprintf(cLogData, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);

			 int hour12 = (int)time.hour;
			 if(hour12 >= 12 )
			 {
				 hour12 = hour12 - 12;
			 }

			 if(hour12 == 0) hour12 =12;

			 //int nHourOld = 0;
			 //int nMinuteOld = 0;

			 if(hour12 != nHourOld )
			 {
				 nHourOld = hour12;

				 sprintf(cLogData, "%02d", hour12);
				 //LCD_DrawText(cLogData, Digital, pos_x+20, pos_y, TFT_COLOR_White, TFT_COLOR_Black);
				 LCD_DrawText(cLogData, FONT9, pos_x+20, pos_y, TFT_COLOR_White, TFT_COLOR_none);

				 sprintf(cLogData, ":");
				 //LCD_DrawText(cLogData, Digital, pos_x+98, pos_y, TFT_COLOR_White, TFT_COLOR_Black);
				 LCD_DrawText(cLogData, FONT9, pos_x+100, pos_y+5, TFT_COLOR_White, TFT_COLOR_none);

				 TFT_setFontSize(3);
				 if(time.hour < 12)
					 TFT_print(235,pos_y+28, "AM");//sprintf(cLogData, "AM");
				 else TFT_print(235,pos_y+28, "PM");//sprintf(cLogData, "PM");

				 //LCD_DrawText(cLogData, FONT6, 240, 125, TFT_COLOR_White, TFT_COLOR_Black);

			 }

			 if(time.minutes != nMinuteOld)
			 {
				 //nHourOld = hour12;
				 nMinuteOld = time.minutes;

				 sprintf(cLogData, "%02d", time.minutes);
				// LCD_DrawText(cLogData, FONT9, 10, 100 , TFT_COLOR_White, TFT_COLOR_Black);
				// LCD_DrawText(cLogData, Digital, pos_x+135, pos_y , TFT_COLOR_White, TFT_COLOR_Black);
				LCD_DrawText(cLogData, FONT9, pos_x+140, pos_y , TFT_COLOR_White, TFT_COLOR_none);

				 if(nBarColor == 0)  nBarColor = 1;
				 else if(nBarColor == 1)  nBarColor = 2;
				 else if(nBarColor == 2)  nBarColor = 3;
				 else if(nBarColor == 3)  nBarColor = 0;

				 //TFT_fillRectangle(20,220,260,11,TFT_RGB(0,0,0));
				 //TFT_fillRectangle(10,220,310,10,TFT_RGB(0,0,0));

			 }

			 //TFT_print(30,100, cLogData);

			 TFT_setFontSize(2);
			 // sprintf(cLogData, "%02d", time.seconds);
			 //TFT_print(250,135, cLogData);
			 if( nSecondOld != time.seconds)
			 {
					nFlicker = !nFlicker;

					if(1)//nFlicker == 1)
						sprintf(cLogData, ":");
					else
						sprintf(cLogData, " ");

				  //LCD_DrawText(cLogData, FONT9, 120, 115, TFT_COLOR_White, TFT_COLOR_Black);
/*
				  nSecondOld = time.seconds;
				  TFT_setFontSize(2);

				  sprintf(cLogData, "%.1f C", (float)HDC1080_Temp);
				  TFT_setTextBackColor(TFT_COLOR_Gray);
				  //TFT_print(100,160, "T"); //TFT_print(10,30, "Temp:");

				  TFT_setColor(TFT_COLOR_Green_Light);
				  TFT_setTextBackColor(TFT_COLOR_Black);
				  TFT_print(240,10, cLogData);

				  TFT_setColor(TFT_COLOR_Aqua);
				  sprintf(cLogData, "%.1f %%", (float)HDC1080_Humi);
				  TFT_setTextBackColor(TFT_COLOR_Gray);
				 // TFT_print(100,160, "H:");//TFT_print(10,60, "Humi:");

				  TFT_setColor(TFT_COLOR_Aqua);
				  TFT_setTextBackColor(TFT_COLOR_Black);
				  TFT_print(240,30, cLogData);
*/
			 }



		}

		if( nDispMode == DISP_MODE_3D_IMG)
		{
			 Get_Time();

				 TFT_setColor(TFT_COLOR_Orange);

				 int pos_x = 0;
				 int pos_y = 95;//175;

				 TFT_setFontSize(2);
	/*
				 int year = (int)time.year;
				 year = 2000 + year;
				 if(time.dayofmonth != nDayOld)
				 {
					 nDayOld = time.dayofmonth;
					 sprintf(cLogData, "%04d.%02d.%02d", year, time.month, time.dayofmonth);
					 LCD_DrawText(cLogData, FONT2, 200, 50, TFT_COLOR_Orange, TFT_COLOR_Black);
				 }

	*/
				 //TFT_print(30,35, cLogData);

				TFT_setFontSize(7);
				TFT_setColor(TFT_COLOR_White);
				 //TFT_setTextBackColor(TFT_COLOR_Teal);

				//sprintf(cLogData, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);

				 int hour12 = (int)time.hour;
				 if(hour12 >= 12 )
				 {
					 hour12 = hour12 - 12;
				 }

				 if(hour12 == 0) hour12 =12;

				 //int nHourOld = 0;
				 //int nMinuteOld = 0;

				 if(hour12 != nHourOld )
				 {
					 nHourOld = hour12;

					 sprintf(cLogData, "%02d", hour12);
					 //TFT_print(pos_x+20, pos_y,cLogData);
					 LCD_DrawText(cLogData, FONT6, pos_x+20, pos_y, TFT_COLOR_Black, TFT_COLOR_Yellow);

					 sprintf(cLogData, ":");
					// TFT_print(pos_x+100, pos_y+5,cLogData);
					 //LCD_DrawText(cLogData, FONT9, pos_x+100, pos_y+5, TFT_COLOR_White, TFT_COLOR_none);

					 TFT_setFontSize(3);
					 if(time.hour < 12)  LCD_DrawText("AM", FONT10, 195, pos_y+40, TFT_COLOR_White, TFT_COLOR_Orange);
						 //TFT_print(235,pos_y+28, "AM");//sprintf(cLogData, "AM");
					 else  LCD_DrawText("PM", FONT10, 195, pos_y+40, TFT_COLOR_White, TFT_COLOR_Orange);
						 //TFT_print(235,pos_y+28, "PM");//sprintf(cLogData, "PM");

					 //LCD_DrawText(cLogData, FONT6, 240, 125, TFT_COLOR_White, TFT_COLOR_Black);

				 }

				 TFT_setFontSize(7);

				 if(time.minutes != nMinuteOld)
				 {
					 //nHourOld = hour12;
					 nMinuteOld = time.minutes;

					 sprintf(cLogData, "%02d", time.minutes);
					 //TFT_print(pos_x+140, pos_y,cLogData);
					LCD_DrawText(cLogData, FONT6, pos_x+95, pos_y+45 , TFT_COLOR_Black, TFT_COLOR_Lime);

					 if(nBarColor == 0)  nBarColor = 1;
					 else if(nBarColor == 1)  nBarColor = 2;
					 else if(nBarColor == 2)  nBarColor = 3;
					 else if(nBarColor == 3)  nBarColor = 0;

					 //TFT_fillRectangle(20,220,260,11,TFT_RGB(0,0,0));
					 //TFT_fillRectangle(10,220,310,10,TFT_RGB(0,0,0));

				 }

				 //TFT_print(30,100, cLogData);

				 TFT_setFontSize(2);
				 // sprintf(cLogData, "%02d", time.seconds);
				 //TFT_print(250,135, cLogData);
				 if( nSecondOld != time.seconds)
				 {
						nFlicker = !nFlicker;

						if(1)//nFlicker == 1)
							sprintf(cLogData, ":");
						else
							sprintf(cLogData, " ");

				 }
		 }

		if( nDispMode == DISP_MODE_BALLOON)
				{
					 Get_Time();

						 TFT_setColor(TFT_COLOR_Orange);

						 int pos_x = 0;
						 int pos_y = 170;//175;

						 TFT_setFontSize(2);
			/*
						 int year = (int)time.year;
						 year = 2000 + year;
						 if(time.dayofmonth != nDayOld)
						 {
							 nDayOld = time.dayofmonth;
							 sprintf(cLogData, "%04d.%02d.%02d", year, time.month, time.dayofmonth);
							 LCD_DrawText(cLogData, FONT2, 200, 50, TFT_COLOR_Orange, TFT_COLOR_Black);
						 }

			*/
						 //TFT_print(30,35, cLogData);

						TFT_setFontSize(7);
						TFT_setColor(TFT_COLOR_White);
						 //TFT_setTextBackColor(TFT_COLOR_Teal);

						//sprintf(cLogData, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);

						 int hour12 = (int)time.hour;
						 if(hour12 >= 12 )
						 {
							 hour12 = hour12 - 12;
						 }

						 if(hour12 == 0) hour12 =12;

						 //int nHourOld = 0;
						 //int nMinuteOld = 0;

						 if(hour12 != nHourOld )
						 {
							 nHourOld = hour12;

							 sprintf(cLogData, "%02d", hour12);
							 //TFT_print(pos_x+20, pos_y,cLogData);
							 LCD_DrawText(cLogData, FONT9, pos_x+10, pos_y, TFT_COLOR_White, TFT_COLOR_Purple);

							 sprintf(cLogData, ":");
							// TFT_print(pos_x+100, pos_y+5,cLogData);
							// LCD_DrawText(cLogData, FONT10, pos_x+100, pos_y-10, TFT_COLOR_White, TFT_COLOR_Gray);

							 TFT_setFontSize(3);
							 if(time.hour < 12)  LCD_DrawText("AM", FONT10, 250, pos_y - 30, TFT_COLOR_White, TFT_COLOR_Orange);
								 //TFT_print(235,pos_y+28, "AM");//sprintf(cLogData, "AM");
							 else  LCD_DrawText("PM", FONT10, 250, pos_y - 30, TFT_COLOR_White, TFT_COLOR_Orange);
								 //TFT_print(235,pos_y+28, "PM");//sprintf(cLogData, "PM");

							 //LCD_DrawText(cLogData, FONT6, 240, 125, TFT_COLOR_White, TFT_COLOR_Black);

						 }

						 TFT_setFontSize(7);

						 if(time.minutes != nMinuteOld)
						 {
							 //nHourOld = hour12;
							 nMinuteOld = time.minutes;

							 sprintf(cLogData, "%02d", (int)time.minutes);
							 //TFT_print(pos_x+140, pos_y,cLogData);
							LCD_DrawText(cLogData, FONT9, pos_x+140, pos_y-20 , TFT_COLOR_White, TFT_COLOR_Blue);

							 if(nBarColor == 0)  nBarColor = 1;
							 else if(nBarColor == 1)  nBarColor = 2;
							 else if(nBarColor == 2)  nBarColor = 3;
							 else if(nBarColor == 3)  nBarColor = 0;

							 //TFT_fillRectangle(20,220,260,11,TFT_RGB(0,0,0));
							 //TFT_fillRectangle(10,220,310,10,TFT_RGB(0,0,0));

						 }

						 //TFT_print(30,100, cLogData);

						 TFT_setFontSize(2);
						 // sprintf(cLogData, "%02d", time.seconds);
						 //TFT_print(250,135, cLogData);
						 if( nSecondOld != time.seconds)
						 {
								nFlicker = !nFlicker;

								if(1)//nFlicker == 1)
									sprintf(cLogData, ":");
								else
									sprintf(cLogData, " ");

						 }
				 }

}

void MeasureTempHumi()
{

//Device : HDC1080
//SCL : D15
//SDA : D14

	if (1)//nRedraw == 0 && nDispMode == DISP_MODE_ALL)
	{
	  buffer[0] = 0x00;

	 //trigger temperature measurment
	  HAL_I2C_Master_Transmit(&hi2c1, 0x40 << 1, buffer, 1, 100);
	  HAL_Delay(20);
	  HAL_I2C_Master_Receive(&hi2c1, 0x40 << 1, buffer, 2, 100);


	  rawTemp = buffer[0] << 8 | buffer[1];		//combine 8-bit of 2ea into 1 16-bit
	  HDC1080_Temp = (( (float)rawTemp / 65536 ) * 165.0 - 40 );

	  HAL_Delay(10);

	  //trigger humidity measurment
	  buffer[0] = 0x01;
	  HAL_I2C_Master_Transmit(&hi2c1, 0x40 << 1, buffer, 1, 100);
	  HAL_Delay(20);
	  HAL_I2C_Master_Receive(&hi2c1, 0x40 << 1, buffer, 2, 100);

	  rawHumi = buffer[0] << 8 | buffer[1];		//combine 8-bit of 2ea into 1 16-bit
	  HDC1080_Humi = (( (float)rawHumi / 65536 ) * 100.0 );

	  HAL_Delay(10);
/*
	  if(0)// nDispMode == DISP_MODE_TIME)
		{
			//test

			//get current time / day
			 Get_Time();

			 TFT_setColor(TFT_COLOR_Orange);
			 // TFT_clear();

			 //TFT_setFontSize(2);
			 //TFT_print(5,10, "DEV :");
			 TFT_setFontSize(3);

			 int year = (int)time.year;
			 year = 2000 + year;
			 if(time.dayofmonth != nDayOld)
			 {
				 nDayOld = time.dayofmonth;
				 sprintf(cLogData, "%04d.%02d.%02d", year, time.month, time.dayofmonth);
				 LCD_DrawText(cLogData, FONT10, 60, 135, TFT_COLOR_Orange, TFT_COLOR_Black);
			 }


			 //TFT_print(30,35, cLogData);

			TFT_setFontSize(7);
			TFT_setColor(TFT_COLOR_White);
			 //TFT_setTextBackColor(TFT_COLOR_Teal);

			//sprintf(cLogData, "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);

			 int hour12 = (int)time.hour;
			 if(hour12 >= 12 )
			 {
				 hour12 = hour12 - 12;
			 }

			 if(hour12 == 0) hour12 =12;

			 //int nHourOld = 0;
			 //int nMinuteOld = 0;

			 if(hour12 != nHourOld )
			 {
				 nHourOld = hour12;

				 sprintf(cLogData, "%02d", hour12);
				 LCD_DrawText(cLogData, FONT6, 30, 60, TFT_COLOR_White, TFT_COLOR_Black);
				 //LCD_DrawText(cLogData, FONT9, 30, 60, TFT_COLOR_White, TFT_COLOR_Black);

				 sprintf(cLogData, ":");
				 LCD_DrawText(cLogData, FONT6, 110, 65, TFT_COLOR_White, TFT_COLOR_Black);
				 //LCD_DrawText(cLogData, FONT9, 110, 65, TFT_COLOR_White, TFT_COLOR_Black);

				 TFT_setFontSize(3);
				 if(time.hour < 12)
					 TFT_print(245,80, "AM");//sprintf(cLogData, "AM");
				 else TFT_print(245,80, "PM");//sprintf(cLogData, "PM");

				 //LCD_DrawText(cLogData, FONT6, 240, 125, TFT_COLOR_White, TFT_COLOR_Black);


			 }

			 if(time.minutes != nMinuteOld)
			 {
				 //nHourOld = hour12;
				 nMinuteOld = time.minutes;

				 sprintf(cLogData, "%02d", time.minutes);
				 LCD_DrawText(cLogData, FONT6, 150, 60, TFT_COLOR_White, TFT_COLOR_Black);
				 //LCD_DrawText(cLogData, FONT9, 150, 60, TFT_COLOR_White, TFT_COLOR_Black);

				 if(nBarColor == 0)  nBarColor = 1;
				 else if(nBarColor == 1)  nBarColor = 2;
				 else if(nBarColor == 2)  nBarColor = 3;
				 else if(nBarColor == 3)  nBarColor = 0;

				 TFT_fillRectangle(20,220,260,11,TFT_RGB(0,0,0));
				 //TFT_fillRectangle(10,220,310,10,TFT_RGB(0,0,0));

			 }

			 //TFT_print(30,100, cLogData);

			 TFT_setFontSize(2);
			 // sprintf(cLogData, "%02d", time.seconds);
			 //TFT_print(250,135, cLogData);
			 if( nSecondOld != time.seconds)
			 {
				 nFlicker = !nFlicker;

				 if(1)//nFlicker == 1)
					 sprintf(cLogData, ":");
				 else
					 sprintf(cLogData, " ");

				 //LCD_DrawText(cLogData, FONT9, 120, 115, TFT_COLOR_White, TFT_COLOR_Black);

				 nSecondOld = time.seconds;
				 TFT_setFontSize(8);
				 //sprintf(cLogData, "a", time.seconds);
				 //sprintf(cLogData, "%02d ", time.seconds);
				 //LCD_DrawText(cLogData, FONT9, 50, 150, TFT_COLOR_White, TFT_COLOR_Black);
				 //draw_string("H", 100, 160, FONT3, 54, 64, 32, 127, 0, 0, 1);

				 //
				 sprintf(cLogData, "|");
				 int R = 0;// (int)4*nSecondOld;
				 int G = 0; // (int)4*nSecondOld;
				 int B = 0; // (int)4*nSecondOld;

				 if(nBarColor == 0)
				 {
					 R = (int)4*nSecondOld;
					 G =  (int)4*nSecondOld;
					 B =  (int)4*nSecondOld;
				 }

				 if(nBarColor == 1)
				 {
					 R = (int)4*nSecondOld;
					 G = 0; // (int)4*nSecondOld;
					 B = 0; // (int)4*nSecondOld;
				 }

				 if(nBarColor == 2)
				 {
					 R = 0;//(int)4*nSecondOld;
					 G = (int)4*nSecondOld;
					 B = 0; // (int)4*nSecondOld;
				 }
				 if(nBarColor == 3)
				 {
					 R = 0;//(int)4*nSecondOld;
					 G = 0;//(int)4*nSecondOld;
					 B =  (int)4*nSecondOld;
				 }


				 int color = nSecondOld%7;
				 //void TFT_drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, uint16_t color)
				 //TFT_fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {

				 int RB,GB,BB;
				 if(R != 0 )RB = 50;
				 else RB = 0;

				 if(G != 0 )GB = 50;
				 else GB = 0;

				 if(B != 0 )BB = 50;
				 else BB = 0;

				 //TFT_drawRectangle(20,220,260,12,1,TFT_RGB(RB,GB,BB));
				 TFT_fillRectangle(20+(nSecondOld*4), 221,15,60,TFT_RGB(R,G,B));
				 //LCD_DrawText(cLogData, FONT6, 0+(nSecondOld*5), 190, TFT_RGB(R,G,B) , TFT_COLOR_Black);
			 }

			  TFT_setFontSize(2);

			  sprintf(cLogData, "%.1f C", (float)HDC1080_Temp);
			  TFT_setTextBackColor(TFT_COLOR_Gray);
			  //TFT_print(100,160, "T"); //TFT_print(10,30, "Temp:");

			  TFT_setColor(TFT_COLOR_Green_Light);
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  TFT_print(62,190, cLogData);

			  TFT_setColor(TFT_COLOR_Silver);
			  sprintf(cLogData, "/");
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  TFT_print(145,190, cLogData);
			 // TFT_print(100,160, "H:");//TFT_print(10,60, "Humi:");


			  TFT_setColor(TFT_COLOR_Aqua);
			  sprintf(cLogData, "%.1f %%", (float)HDC1080_Humi);
			  TFT_setTextBackColor(TFT_COLOR_Gray);
			 // TFT_print(100,160, "H:");//TFT_print(10,60, "Humi:");

			  TFT_setColor(TFT_COLOR_Aqua);
			  TFT_setTextBackColor(TFT_COLOR_Black);
			  TFT_print(165,190, cLogData);

		}
*/

	}



}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI3_Init();
  MX_I2C3_Init();
  MX_BlueNRG_2_Init();
  /* USER CODE BEGIN 2 */

  //LCD
  TFT_init(TFT_ORIENT_LANDSCAPE, &hspi3);

  // TFT_clear();

  //main window draw first
  DrawMainWindow(DISP_MODE_3D_IMG);//
  //DrawMainWindow(DISP_MODE_TIME);//DISP_MODE_3D_IMG


  //draw_string("Hello\nWorld!", 10, 50, Digital_2, 38, 71, 32, 127, 0, 0, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
	  MX_BlueNRG_2_Process();


	 // if(nRedraw == 1)
	  {
		  MeasureTempHumi();

		  MainLCDDisplayMode(setDisplayModeCmd);
	  }

	  //USER BUTTON IO push => false
	  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == false)
	  {
		  setDisplayModeCmd++;
		  if(setDisplayModeCmd > 5)setDisplayModeCmd = 0;
	  }

	  //bluetooth ble Set time cmd recieved
	  if(nSetTime == _ON_)
	  {
		  nSetTime = _OFF_;
		  Set_Time(time.seconds,time.minutes,time.hour,time.dayofweek ,time.dayofmonth,time.month,time.year );

		  /*
		  	Set_Time(
		  			50,  //    sec
		  			18,  //    min
		  			00,  //   hour day of week
		  			3,    //  day of week (1 ~ 7 ) 1= sunday, ... 7 = saturday
		  			25,   //    day of month(1 ~ 31)
		  			2,    //    month
		  			21);  //    year
		  */
	  }

#if 0
	  //USER BUTTON IO push => true
	  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == true)
	  {
		  //nRedraw = 0;
		  HAL_GPIO_WritePin(GPIOC, LED_Pin,GPIO_PIN_SET);//PC7

		 // TFT_drawImage(320,240, IMG_KF);
	  }
	  else
	  {
		  //nRedraw = 1;
		  HAL_GPIO_WritePin(GPIOC, LED_Pin,GPIO_PIN_RESET);
	  }


	  if(setLEDControl == 1)
	  {
		  HAL_GPIO_WritePin(GPIOC, LED_Pin,GPIO_PIN_SET);

		  TFT_clear();

		  //TFT_drawImage(320,240, IMG_KF);
		  TFT_drawImage(320,240, IMG_BG);

		  setLEDControl = 0;
		  nRedraw = 0;

	  }
	  else  nRedraw = 1;
#endif
	  //else HAL_GPIO_WritePin(GPIOC, LED_Pin,GPIO_PIN_RESET);



  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_LCD_GPIO_Port, CS_LCD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RS_Pin|LCD_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : CS_LCD_Pin */
  GPIO_InitStruct.Pin = CS_LCD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_LCD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RS_Pin LCD_RESET_Pin */
  GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_LED_Pin */
  GPIO_InitStruct.Pin = BLE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLE_LED_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
