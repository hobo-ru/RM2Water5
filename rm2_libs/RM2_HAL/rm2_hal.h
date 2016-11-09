//
//  RM2 Hardware Abstraction Layer
//  01.07.2015 S.Omelchenko
//  somelchenko@strij.net
//

#ifndef NWAVE_COMMON_H_
#define NWAVE_COMMON_H_

#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "time.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_wdog.h"
#include "em_rmu.h"
#include "em_device.h"
#include "em_gpio.h"
#include "em_opamp.h"
#include "em_adc.h"
#include "em_cmu.h"

#include "gpiointerrupt.h"

#include "td1202.h"
#include "td_gpio.h"
#include "td_rtc.h"
#include "td_flash.h"
#include "td_measure.h"
#include "td_flash.h"

#include "rm2_settings.h"

#include "UNB.h"




enum LED_STATE
{
	OFF = 0,
	ON = 1,
	TOGGLE = 2
};

typedef struct
{
	GPIO_Port_TypeDef 	port;	// GPIO port
	unsigned int 		pin;	// GPIO pin
	bool 				event;	// Event flag
}button_t;


/** RM2 device descriptor */
typedef struct _RM2_DEVICE {
    uint32_t   ID;            	/* ID  */
    uint32_t   Key[8];			/*Key*/
    uint32_t   Reserved[7];	        /*Reserved*/
} RM2_DEVICE;


#define RM2_TRANSFER    (0x8000 - 256)
#define E2P_FACTORY     0x0FE00004

#define TIME_SINCE(x)	(__time32(0) - (x))
#define TIME_BEFORE(x)	((x) - __time32(0))
#define NOW()				(__time32(0))

#define BUTTON_PRESSED	0
#define BUTTON_RELEASED	1

#define BUTTON_INT_DEBOUNCE_TIMEOUT 6

#define MEASURE_PERIOD 20		// VWC measurement period in seconds
#define MEASURES_PER_PACKET 4	// Number of measurements to accumulate before transmittion

enum mode
{
	MODE_SLEEP = 0,
	MODE_MEASURING = 1
};

extern uint32_t serial;
extern uint8_t rm2_mode;

void delay(uint32_t msec);	// no sleep. to make MCU sleep in delay, use TD_RTC_Delay()

void 		RM2_Watchdog_Init();
void 		RM2_Watchdog_Feed();
uint32_t 	RM2_GetSerial();
RM2_DEVICE*     RM2_GetDevice(RM2_DEVICE* device);
void 		RM2_RTC_EverySecondEvent();
void 		RM2_RTC_Init();
void 		RM2_Tiffany_GPIO_Init();
uint8_t		RM2_ButtonInt_Init(GPIO_Port_TypeDef port, unsigned int pin);
uint8_t 	RM2_ButtonInt_GetState(uint8_t n);
uint8_t 	RM2_ButtonGetState(uint8_t n);
void 		RM2_LED(uint8_t state);
bool            RM2_Trasfer_ID();

#endif