
/*
 * smartio_if.c
 *
 *  Created on: Nov 17, 2018
 *      Author: carstenh
 */

#include <gpio.h>
#include <string.h>

#include "smartio_if.h"
#include "smartio_api.h"
#include "smartio_interface.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t SifQueue;  	// Second Queue for the SmartIO task for general messaging.


static void SifAppConnect(void); // Initializes App
static void SifAppDisconnect(void);	// App disconnect

static char * SIO_Version;

static void SifOnOffButtonCb( uint16_t value );
typedef enum
{
    APP_IS_OFFLINE,
    APP_CAME_ONLINE,
    APP_IS_ONLINE

} command_app_state_t;


static void SifInit(void);	// Initializes command handler

static void SifAppInit(void);  // Initialize App

static command_app_state_t AppCommandHandler(void);

static void SifSendInfoString(uint8_t * info);

static int AppSPI_State = SPI_IDLE;

void SifInit(void)
{

    SmartIO_Init( SifAppConnect, SifAppDisconnect);		// Initialize the Smart IO interface
    SIO_Version = SmartIO_GetVersion();	// Firmware version string

}

static tHandle info_hdl;


void SifAppInit(void)
{

    // Initialize UI
    SmartIO_AppTitle("B.I.G");
    tHandle p1 = SmartIO_MakePage();

    SmartIO_MakeLabel(0, 40, "Bedbug Intelligence Group" );

    tHandle h1 = SmartIO_MakeOnOffButton(0, 0, 1, SifOnOffButtonCb);
    SmartIO_AddText(h1, "PSTAT");
    SmartIO_SetSliceIcon(h1, SMARTIO_ICON_POWER);

    info_hdl = SmartIO_MakeTextBox(1, 320, 2, "Info from terminal");

    //	 tHandle h3 = SmartIO_MakeSlider(1, 0, 30, Slider1);

    //SmartIO_MakeSpacerSlice(2);
    //tHandle tl0 = SmartIO_MakeLabel(0, 1, "Test App demonstrating functionality");
    /*
	 tHandle tl1 = SmartIO_MakeLabel(0, 0, " Weekdays");
	 tHandle th1 = SmartIO_MakeTimeSelector(0, 1, "17:00", ts1);
	 SmartIO_AddText(th1, "ON at");
	 SmartIO_SetSliceIcon(th1, SMARTIO_ICON_QUERY);
	 tHandle th2 = SmartIO_MakeTimeSelector(0, 1, "12:00", ts2);
	 SmartIO_AddText(th2, "OFF at");
	 SmartIO_SetSliceIcon(th2, SMARTIO_ICON_QUERY);
	 SmartIO_GroupObjects(0, tl1, th1, th2, 0);
	 SmartIO_MakeSpacerSlice(1);

	 tHandle tl2 = SmartIO_MakeLabel(0, 0, " Weekend");
	 tHandle th3 = SmartIO_MakeTimeSelector(0, 1, "18:00", 0);
	 SmartIO_AddText(th3, "ON at");
	 SmartIO_SetSliceIcon(th3, SMARTIO_ICON_QUERY);
	 tHandle th4 = SmartIO_MakeTimeSelector(0, 1, "1:00", 0);
	 SmartIO_AddText(th4, "OFF at");
	 SmartIO_SetSliceIcon(th4, SMARTIO_ICON_QUERY);
	 SmartIO_GroupObjects(0, tl2, th3, th4, 0);
	 SmartIO_MakeSpacerSlice(3);
     */

    SmartIO_AutoBalance(p1);

}

/* Main SmartIO task */



void SifTask( void *params)
{
    //TickType_t xLastExecutionTime;

    /* Init the xLastExecutionTime variable on task entry. */
    //xLastExecutionTime = xTaskGetTickCount();

#ifndef BT_IS_HW_DEBUG
    SifInit();
#endif

    for (;;)
    {
        Message_t PortMsg;

        // Handle UI
        AppCommandHandler();

        if (xQueueReceive( SifQueue, (void*)&PortMsg, 20 ))
        {
            if (PortMsg.Type == CLI_TEXT_MESSAGE)
            {
                CliMsgContainer * msg = (CliMsgContainer*)PortMsg.data;

                if (msg != NULL)
                {
                    SifSendInfoString( (uint8_t*)msg->CLI_MESSAGE_data);

                    vPortFree(msg->CLI_MESSAGE_data);
                    vPortFree(msg);
                }
            }
        }

    }
}


static command_app_state_t AppState = APP_IS_OFFLINE;

command_app_state_t AppCommandHandler(void)
{
    if (AppState == APP_CAME_ONLINE)
    {
        // Connect. 300 ms is a empirically found to be a good number. Documentation has 10 ms but the smartio module doesn't respond.
        vTaskDelay(  pdMS_TO_TICKS(300) );

        //HAL_Delay(300);	//DelayMilliSecs(10);
        SifAppInit();
        AppState = APP_IS_ONLINE;
    }


    while (SPI_State == SPI_SMARTIO_ASYNC_REQUEST)
    {
        SmartIO_ProcessUserInput();
    }


    return(AppState);
}


void SifAppConnect(void)
{
    char * msg = "App is online\n\r";

    CliSendTextMsg(msg, CliDataQueue);

    AppState = APP_CAME_ONLINE;

}

void SifAppDisconnect(void)
{
    AppSPI_State = SPI_IDLE;

    AppState = APP_IS_OFFLINE;


    CliSendTextMsg("App is offline\n\r", CliDataQueue);

}


void SifOnOffButtonCb( uint16_t value )
{
    char msg[64];

    sprintf(msg, "App ON/OFF button pressed (value %d)\n\r", value);

    CliSendTextMsg(msg, CliDataQueue);

}



void SifSendInfoString(uint8_t * info)
{
    if (AppState == APP_IS_ONLINE)
    {
        SmartIO_ClearText(info_hdl+1);

        SmartIO_AddText(info_hdl+1, (char *)info);
    }

}



