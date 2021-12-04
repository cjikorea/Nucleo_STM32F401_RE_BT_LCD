/******************** (C) COPYRIGHT 2019 STMicroelectronics ********************
 * File Name          : gatt_db.h
 * Author             : SRA
 * Version            : V1.0.0
 * Date               : Oct-2019
 * Description        : Header file for gatt_db.c
 *******************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 ******************************************************************************/

#ifndef GATT_DB_H
#define GATT_DB_H

/* USER CODE BEGIN PM */
/* Includes ------------------------------------------------------------------*/
#include "hci.h"
/* USER CODE END PM */

/* Exported defines ----------------------------------------------------------*/
#define X_OFFSET 200
#define Y_OFFSET 50
#define Z_OFFSET 1000

/**
 * @brief Number of application services
 */
#define NUMBER_OF_APPLICATION_SERVICES (2)

/**
 * @brief Define How Many quaterions you want to trasmit (from 1 to 3)
 *        In this sample application use only 1
 */
#define SEND_N_QUATERNIONS 1

/* Exported typedef ----------------------------------------------------------*/
/**
 * @brief Structure containing acceleration value of each axis.
 */
typedef struct {
  int32_t AXIS_X;
  int32_t AXIS_Y;
  int32_t AXIS_Z;
} AxesRaw_t;

enum {
  ACCELERATION_SERVICE_INDEX = 0,
  ENVIRONMENTAL_SERVICE_INDEX = 1
};

/* Exported function prototypes ----------------------------------------------*/
tBleStatus Add_HWServW2ST_Service(void);
tBleStatus Add_SWServW2ST_Service(void);
tBleStatus Add_TempHumi_Service(void);
tBleStatus Add_SetTime_Service(void);

#ifdef STM32L476xx
tBleStatus Add_ConsoleW2ST_Service(void);
#endif /* STM32L476xx */
void Read_Request_CB(uint16_t handle);

/* USER CODE BEGIN PM */
void Read_TempHumi_CB(uint16_t handle);
void Attribute_Modified_Request_CB(uint16_t Connection_Handle, uint16_t attr_handle,
                                   uint16_t Offset, uint8_t data_length, uint8_t *att_data);
tBleStatus Environmental_Update(int32_t press, int16_t temp);
tBleStatus TempHumi_Update(int32_t temp, int16_t humi);
/* USER CODE END PM */

tBleStatus Acc_Update(AxesRaw_t *x_axes, AxesRaw_t *g_axes, AxesRaw_t *m_axes);
tBleStatus Quat_Update(AxesRaw_t *q_axes);

#ifdef STM32L476xx
tBleStatus Term_Update(uint8_t *data, uint8_t length);
tBleStatus Term_Update_AfterRead(void);
#endif /* STM32L476xx */

/* USER CODE BEGIN PM */
#define CONNECTED               0x01
#define SET_CONNECTABLE         0x02
#define NOTIFICATIONS_ENABLED   0x04
#define CONN_PARAM_UPD_SENT     0x08
#define L2CAP_PARAM_UPD_SENT    0x10
#define TX_BUFFER_FULL          0x20
/* Added flags for handling TX, RX characteristics discovery */
#define START_READ_TX_CHAR_HANDLE 0x100
#define END_READ_TX_CHAR_HANDLE   0x200
#define START_READ_RX_CHAR_HANDLE 0x400
#define END_READ_RX_CHAR_HANDLE   0x800
/* USER CODE END PM */

#define APP_FLAG(flag) (app_flags & flag)
#define APP_FLAG_SET(flag) (app_flags |= flag)
#define APP_FLAG_CLEAR(flag) (app_flags &= ~flag)

#endif /* GATT_DB_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
