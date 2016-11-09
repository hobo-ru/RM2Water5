//
//  RM2 Hardware Abstraction Layer
//  01.07.2015 S.Omelchenko
//  somelchenko@strij.net
//

#include "rm2_hal.h"


/* Defining the watchdog initialization data */
WDOG_Init_TypeDef init =
{
  .enable     = true,               /* Start watchdog when init done */
  .debugRun   = false,              /* WDOG not counting during debug halt */
  .em2Run     = true,               /* WDOG counting when in EM2 */
  .em3Run     = true,               /* WDOG counting when in EM3 */
  .em4Block   = false,              /* EM4 can be entered */
  .swoscBlock = false,              /* Do not block disabling LFRCO/LFXO in CMU */
  .lock       = false,              /* Do not lock WDOG configuration (if locked, reset needed to unlock) */
  .clkSel     = wdogClkSelULFRCO,   /* Select 1kHZ WDOG oscillator */
  .perSel     = wdogPeriod_16k,     /* Set the watchdog period to  16385 clock periods (ie ~16 seconds)*/
};

uint8_t button_int = 0;

uint8_t button_num;
button_t buttons[N_OF_BUTTONS];

// Initialize watchdog. Don't forget to feed it regulary
void RM2_Watchdog_Init()
{
	WDOG_Init(&init);
}

void RM2_Watchdog_Feed()
{
	WDOG_Feed();
}

void delay(uint32_t msec)
{
	msec = msec * 1024 / 1000;

	uint32_t target_time = RTC->CNT + msec;
	while(RTC->CNT != target_time);

	//TD_RTC_Delay(msec);
}

__weak void RM2_RTC_EverySecondEvent()
{
	return;
}

void RM2_RTC_Init()
{
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_CORELE, true);

	TD_RTC_Init(0);
	TD_RTC_SetUserHandler(RM2_RTC_EverySecondEvent);
}

uint32_t RM2_GetSerial()
{
        
        RM2_DEVICE        device;
   
        if(!TD_FLASH_ReadRegion(E2P_FACTORY, &device, sizeof(RM2_DEVICE))) return 0xffffffff; //default ID

        return device.ID;
        
        
}

RM2_DEVICE* RM2_GetDevice(RM2_DEVICE* device)
{

              
        if(!TD_FLASH_ReadRegion(E2P_FACTORY, device, sizeof(RM2_DEVICE))) return 0; 

                 
        return device;
  
}


bool RM2_Trasfer_ID()
{
        
    RM2_DEVICE        device;
    
    if(!TD_FLASH_ReadRegion(RM2_TRANSFER, &device, sizeof(RM2_DEVICE))) return false;
       
    if(device.ID != 0) TD_FLASH_WriteRegion(E2P_FACTORY, &device, sizeof(RM2_DEVICE));
    
    memset((void*)&device, 0xff , sizeof(RM2_DEVICE));
    
    TD_FLASH_ErasePage(RM2_TRANSFER);
    
        
    return true;
     
}


void RM2_ButtonInt_Callback(uint8_t pin)
{
	static uint32_t last_int_time = 0;

	if(TIME_SINCE(last_int_time) < BUTTON_INT_DEBOUNCE_TIMEOUT)
	{
		return;
	}

	for(int i=0; i<button_num; i++)
	{
		if(pin == buttons[i].pin)
		{
			RM2_LED(ON);
			buttons[i].event++;
		}
	}
	last_int_time = __time32(0);
}

uint8_t RM2_ButtonInt_GetState(uint8_t n)
{
	uint16_t events = buttons[n].event;
	buttons[n].event = 0;	// clear event counter
	return events;
}

uint8_t RM2_ButtonGetState(uint8_t n)
{
	return GPIO_PinInGet(buttons[n].port, buttons[n].pin);
}

void RM2_Tiffany_GPIO_Init()
{
	CMU_ClockEnable(cmuClock_GPIO, true);

	GPIO_PinModeSet(LED_PORT,		LED_PIN,		gpioModePushPull, 0);//led on tiffany
	GPIO_PinModeSet(JP2_PORT,	JP2_PIN,	gpioModeInputPull , 1);//JP2 
        GPIO_PinModeSet(JP2_GND_PORT,	JP2_GND_PIN,	gpioModePushPull, 0);//JP2 gnd
        GPIO_PinModeSet(SB1_PORT,	SB1_PIN,	gpioModeInputPull , 1);//SB1        
}

void RM2_Tiffany_GPIO_Deinit()
{
	GPIO_PinModeSet(LED_PORT,		LED_PIN,	gpioModeDisabled, 0);//led on tiffany
}

uint8_t RM2_ButtonInt_Init(GPIO_Port_TypeDef port, unsigned int pin)
{
	if(button_num >= N_OF_BUTTONS)	return 0;

	buttons[button_num].port = port;
	buttons[button_num].pin = pin;
	buttons[button_num].event = 0;

	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIOINT_Init();
	GPIO_PinModeSet(buttons[button_num].port, buttons[button_num].pin, gpioModeInputPullFilter, 1);//reed switch tiffany
	GPIOINT_CallbackRegister(buttons[button_num].pin, RM2_ButtonInt_Callback);
	GPIO_IntConfig(buttons[button_num].port, buttons[button_num].pin, false, true, true);

	return (button_num++);
}

void RM2_LED(uint8_t state)
{
	switch(state)
	{
	case ON:
		GPIO_PinOutSet(LED_PORT, LED_PIN);
		break;
	case OFF:
		GPIO_PinOutClear(LED_PORT, LED_PIN);
		break;
	case TOGGLE:
		GPIO_PinOutToggle(LED_PORT, LED_PIN);
		break;
	default:
		break;
	}
}


