/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
 * File Name          : sensor.c
 * Author             : AMS - VMA RF Application team
 * Version            : V1.0.0
 * Date               : 23-November-2015
 * Description        : Sensor init and sensor state machines
 *******************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sensor.h"
#include "gatt_db.h"
#include "bluenrg1_gap.h"
#include "bluenrg1_gap_aci.h"
#include "bluenrg1_hci_le.h"
#include "hci_const.h"
#include "bluenrg1_gatt_aci.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint8_t bdaddr[BDADDR_SIZE];
extern uint8_t bnrg_expansion_board;
extern uint16_t EnvironmentalCharHandle;
extern uint16_t AccGyroMagCharHandle;
extern __IO uint8_t send_temphumi;

extern int nSetTime;
extern TIME time;

__IO uint8_t set_connectable = 1;
__IO uint16_t connection_handle = 0;
__IO uint8_t  notification_enabled = FALSE;
__IO uint32_t connected = FALSE;

volatile uint8_t request_free_fall_notify = FALSE;

AxesRaw_t x_axes = {0, 0, 0};
AxesRaw_t g_axes = {0, 0, 0};
AxesRaw_t m_axes = {0, 0, 0};
AxesRaw_t q_axes[SEND_N_QUATERNIONS] = {{0, 0, 0}};

/* Private function prototypes -----------------------------------------------*/
void GAP_DisconnectionComplete_CB(void);
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle);

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Set_DeviceConnectable
 * @note   Puts the device in connectable mode
 * @param  None
 * @retval None
 */
void Set_DeviceConnectable(void)
{
  uint8_t ret;
  uint8_t local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,SENSOR_DEMO_NAME};

  uint8_t manuf_data[26] = {
    2,0x0A,0x00, /* 0 dBm */  // Trasmission Power
    8,0x09,SENSOR_DEMO_NAME,  // Complete Name
    13,0xFF,0x01, /* SKD version */
    0x02,
    0x00,
    0xF4, /* ACC+Gyro+Mag 0xE0 | 0x04 Temp | 0x10 Pressure */
    0x00, /*  */
    0x00, /*  */
    bdaddr[5], /* BLE MAC start -MSB first- */
    bdaddr[4],
    bdaddr[3],
    bdaddr[2],
    bdaddr[1],
    bdaddr[0]  /* BLE MAC stop */
  };

  manuf_data[18] |= 0x01; /* Sensor Fusion */

  hci_le_set_scan_response_data(0,NULL);

  PRINT_DBG("Set General Discoverable Mode.\r\n");

  ret = aci_gap_set_discoverable(ADV_DATA_TYPE,
                                 ADV_INTERV_MIN, ADV_INTERV_MAX,
                                 STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                                 sizeof(local_name), local_name, 0, NULL, 0, 0);

  aci_gap_update_adv_data(26, manuf_data);

  if(ret != BLE_STATUS_SUCCESS)
  {
    PRINT_DBG("aci_gap_set_discoverable() failed: 0x%02x\r\n", ret);
  }
  else
    PRINT_DBG("aci_gap_set_discoverable() --> SUCCESS\r\n");
}

/**
  * @brief  This function is called when there is a notification from the sever.
  * @param  attr_handle Handle of the attribute
  * @param  attr_len    Length of attribute value in the notification
  * @param  attr_value  Attribute value in the notification
  * @retval None
  */
void GATT_Notification_CB(uint16_t attr_handle, uint8_t attr_len, uint8_t *attr_value)
{
#if THROUGHPUT_TEST && CLIENT
    static tClockTime time, time2;
    static uint32_t packets=0;
    static uint32_t n_packet1, n_packet2, lost_packets = 0;

    if(attr_handle == tx_handle+1){
        if(packets==0){
            printf("Test start\n");
            time = Clock_Time();
            n_packet1 = LE_TO_HOST_32(attr_value+16) - 1;
        }

        //for(int i = 0; i < attr_len; i++)
            //printf("%c", attr_value[i]);
        //printf("[RX: %d]", packets);

        n_packet2 = LE_TO_HOST_32(attr_value+16);
        if(n_packet2 != n_packet1 + 1){
          lost_packets += n_packet2-(n_packet1+1);
        }
        n_packet1 = n_packet2;

        packets++;

        if(packets != 0 && packets%NUM_PACKETS == 0){
            time2 = Clock_Time();
            tClockTime diff = time2-time;
            printf("%d packets. Elapsed time: %d ms. App throughput: %.2f kbps.\n", NUM_PACKETS, diff, (float)NUM_PACKETS*20*8/diff);
            if(lost_packets){
              printf("%d lost packet(s)\n", lost_packets);
            }
            time = Clock_Time();
            lost_packets=0;
        }

    }
#elif CLIENT

    if(attr_handle == tx_handle+1){
      for(int i = 0; i < attr_len; i++)
          printf("%c", attr_value[i]);
    }
#endif
}

/**
 * @brief  Callback processing the ACI events
 * @note   Inside this function each event must be identified and correctly
 *         parsed
 * @param  void* Pointer to the ACI packet
 * @retval None
 */
void APP_UserEvtRx(void *pData)
{
  uint32_t i;

  hci_spi_pckt *hci_pckt = (hci_spi_pckt *)pData;

  if(hci_pckt->type == HCI_EVENT_PKT)
  {
    hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

    if(event_pckt->evt == EVT_LE_META_EVENT)
    {
      evt_le_meta_event *evt = (void *)event_pckt->data;

      for (i = 0; i < (sizeof(hci_le_meta_events_table)/sizeof(hci_le_meta_events_table_type)); i++)
      {
        if (evt->subevent == hci_le_meta_events_table[i].evt_code)
        {
          hci_le_meta_events_table[i].process((void *)evt->data);
        }
      }
    }
    else if(event_pckt->evt == EVT_VENDOR)
    {
		  evt_blue_aci *blue_evt = (void*)event_pckt->data;

		              switch(blue_evt->ecode){

		              case EVT_BLUE_GATT_WRITE_PERMIT_REQ:
					  {
						  	  evt_gatt_write_permit_req *evt = (evt_gatt_write_permit_req *)blue_evt->data;

							  int ret = SetTime_Request_CB(evt->conn_handle,evt->attr_handle,evt->data_length, evt->data);

							  if(ret == 1)
							  {
								  uint8_t err_code = 0;
								  uint8_t write_status = err_code == 0 ? 0 : 1;

								  //reply
								  Write_Request_CB(evt->conn_handle,evt->attr_handle,evt->data_length, evt->data);

								  nSetTime = 1; //Time Set
							  }

					  }


		              case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
		                  {
		                      evt_gatt_attr_modified *evt = (evt_gatt_attr_modified *)blue_evt->data;
		                      //evt_gatt_attr_modified *evt = (evt_gatt_attr_modified*)blue_evt->data;
		                      Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);

		                      evt_att_find_information_resp *pr = (evt_att_find_information_resp*)blue_evt->data;

		                      uint8_t format_lenofuuid;
		                      format_lenofuuid = pr->format;
		                     // if(pr->event_data_length == )
		                      uint8_t * uuid_1 = pr->handle_uuid_pair;
		                      uint8_t * uuid_2 = pr->handle_uuid_pair;
		                  }
		                  break;
		              case EVT_BLUE_GATT_NOTIFICATION:
		                  {
		                      evt_gatt_attr_notification *evt = (evt_gatt_attr_notification*)blue_evt->data;
		                      GATT_Notification_CB(evt->attr_handle, evt->event_data_length - 2, evt->attr_value);
		                  }
		                  break;
		                  /*
		               case EVT_BLUE_L2CAP_CONN_UPD_RESP:
		                  {
		                      evt_l2cap_conn_upd_resp *resp = (void*)blue_evt->data;
		                      if(resp->result){
		                          PRINTF("> Connection parameters rejected.\n");
		                      }
		                      else{
		                          PRINTF("> Connection parameters accepted.\n");
		                      }
		                  }
		                  break;
		                  */
				#ifdef CLIENT
		                case EVT_BLUE_L2CAP_CONN_UPD_REQ:
		                {
		                  evt_l2cap_conn_upd_req *req = (void*)blue_evt->data;
		                  // Always accept request
		                  aci_l2cap_connection_parameter_update_response(req->conn_handle,
		                                                                 req->interval_min,req->interval_max,req->slave_latency,req->timeout_mult,
		                                                                 req->identifier,1);
		                }
		                break;

		                case EVT_BLUE_GATT_DISC_READ_CHAR_BY_UUID_RESP:
		                  {
		                      evt_gatt_disc_read_char_by_uuid_resp *resp = (void*)blue_evt->data;

		                      if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
		                      {
		                        tx_handle = resp->attr_handle;
		                        PRINTF("TX Char Handle %04X\n", tx_handle);
		                      }
		                      else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
		                      {
		                        rx_handle = resp->attr_handle;
		                        PRINTF("RX Char Handle %04X\n", rx_handle);
		                      }
		                  }
		                  break;

		                  case EVT_BLUE_GATT_PROCEDURE_COMPLETE:
		                  {
		                    /* Wait for gatt procedure complete event trigger related to Discovery Charac by UUID */
		                    //evt_gatt_procedure_complete *pr = (void*)blue_evt->data;

		                    if (APP_FLAG(START_READ_TX_CHAR_HANDLE) && !APP_FLAG(END_READ_TX_CHAR_HANDLE))
		                    {
		                      APP_FLAG_SET(END_READ_TX_CHAR_HANDLE);
		                    }
		                    else if (APP_FLAG(START_READ_RX_CHAR_HANDLE) && !APP_FLAG(END_READ_RX_CHAR_HANDLE))
		                    {
		                      APP_FLAG_SET(END_READ_RX_CHAR_HANDLE);
		                    }
		                  }
		                  break;
		  #endif
		              }



      for (i = 0; i < (sizeof(hci_vendor_specific_events_table)/sizeof(hci_vendor_specific_events_table_type)); i++)
      {
        if (blue_evt->ecode == hci_vendor_specific_events_table[i].evt_code)
        {
          //hci_vendor_specific_events_table[i].process((void *)blue_evt->data);
        }
      }
    }
    else
    {
      for (i = 0; i < (sizeof(hci_events_table)/sizeof(hci_events_table_type)); i++)
      {
        if (event_pckt->evt == hci_events_table[i].evt_code)
        {
         // hci_events_table[i].process((void *)event_pckt->data);
        }
      }
    }
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
