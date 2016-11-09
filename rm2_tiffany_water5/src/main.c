//
//  RM2 Tiffany water FW : main.c
//  12.07.2015 S.Omelchenko
//  somelchenko@strij.net
//

#include "rm2_hal.h"
#include "water5.h"
#include "defines.h"





uint8_t  Old_IN1, Old_IN2;



uint32_t serial;	


void battery_start_measure()
{
	//mode 0 - v


	/* Base the ADC configuration on the default setup. */
	ADC_InitSingle_TypeDef single_init = ADC_INITSINGLE_DEFAULT;
	ADC_Init_TypeDef init  = ADC_INIT_DEFAULT;

	/* Initialize timebase */
	init.timebase = ADC_TimebaseCalc(0);
	init.prescale = ADC_PrescaleCalc(40000, 0);
	CMU_ClockEnable(cmuClock_ADC0, true);
	ADC_Init(ADC0, &init);

	/* Set input to temperature sensor. Reference must be 1.25V */
	single_init.reference = adcRef1V25;
	//  single_init.resolution = adcRes8Bit;
	single_init.input   	= adcSingleInpVDDDiv3;
	ADC_InitSingle(ADC0, &single_init);

	// Start one ADC sample
	ADC_Start(ADC0, adcStartSingle);

}

uint8_t battery_read_measure()
{
	uint32_t setpoint;
	setpoint = ADC_DataSingleGet(ADC0);
	setpoint = (setpoint * (125000 * 3 / 4096));	
        if (setpoint >= 300000) {
    	  setpoint = 0x80 + ((setpoint - 300000) / 1000);        				// msb = 1 -> 3V + n*(10mv)
      } else {
    	  setpoint = 0x00 + ((setpoint - 200000) / 1000);                       // msb = 0 -> 2V + n*(10mv)
      }
	CMU_ClockEnable(cmuClock_ADC0, false);
	return setpoint;
}

void GetInputs()
{
      uint8_t led_on = 0;
#ifdef POWER_METER
        GPIO_PinModeSet(OUT1_PORT, OUT1_PIN, gpioModeInputPull, 1);
#else
        GPIO_PinOutSet(OUT1_PORT,OUT1_PIN);
#endif
        

        GPIO_PinOutSet(OUT2_PORT,OUT2_PIN);
        for(uint8_t i = 0; i < 30; i++);
        
        
        if(GPIO_PinInGet(IN1_PORT , IN1_PIN) == 0)  
        {
          if(Old_IN1 == 1)      {W5_Tags.PulseCounter0++;if(Calibrate) RM2_LED(ON); led_on = 1;} 
          Old_IN1 = 0;
          
        }
        else    {Old_IN1 = 1; if(Calibrate) RM2_LED(OFF);}
        
        if(GPIO_PinInGet(IN2_PORT , IN2_PIN) == 0)  
        {
          if(Old_IN2 == 1)      
          {
#ifdef POWER_METER           
            W5_Tags.PulseCounter0++; 
#else
            W5_Tags.PulseCounter1++; 
#endif
          if(Calibrate) RM2_LED(ON);

          }
            Old_IN2 = 0;
          
        }
        else    {Old_IN2 = 1;if(Calibrate&&!led_on) RM2_LED(OFF);}
        
#ifdef POWER_METER
        GPIO_PinModeSet(OUT1_PORT, OUT1_PIN, gpioModeInputPull, 0);
#else
        GPIO_PinOutClear(OUT1_PORT,OUT1_PIN);
#endif
        GPIO_PinOutClear(OUT2_PORT,OUT2_PIN);
        
}
uint8_t CompVersion()
{
    const char CompTime[] = __TIME__;
    const char* ptr;
    uint8_t ver = 0;
    ptr = &CompTime[0];
    while(*ptr) ver += ((*(ptr++)) - 0x30) + (uint8_t)ptr;
    return ver;
}


void main()
{
        RM2_DEVICE      device; 
  
        CHIP_Init();
	RM2_RTC_Init();
	RM2_Tiffany_GPIO_Init();
        
        
        if(RM2_Trasfer_ID()) ClearAllParams();
        RM2_GetDevice(&device);
        serial = device.ID;
        UNB_Set_KEY_ptr(device.Key);
        
        random_seed = serial;
	UNB_FrequencyConf(UL_CENTER_FREQ - 25000, UL_CENTER_FREQ + 25000, 100);
#ifdef SIMPLE_ONESEC_TESTER
        UNB_setPower(NormalPower);
#else     
#ifdef  DIN
        UNB_setPower(NormalPower_m6);
#else
        UNB_setPower(NormalPower_m12);
#endif  
#ifdef  POWER_METER    
        GPIO_PinModeSet(OUT1_PORT, OUT1_PIN, gpioModeInputPull /*gpioModePushPull*/, 0);
#else
        GPIO_PinModeSet(OUT1_PORT, OUT1_PIN, gpioModePushPull, 0);
#endif
        GPIO_PinModeSet(OUT2_PORT, OUT2_PIN, gpioModePushPull, 0);
        GPIO_PinModeSet(IN1_PORT , IN1_PIN, gpioModeInput, 1);
        GPIO_PinModeSet(IN2_PORT , IN2_PIN, gpioModeInput, 1);

        GetInputs();
#endif
	Water5Init(0);
        
        // Send RESET message to indicate that reset has occured and give us reset cause flags
	//WaterV4_Send(RESTART_MSG, &hot_counter, &cold_counter);
	RMU_ResetCauseClear();
        
        uint16_t i = 0;

#ifndef SIMPLE_ONESEC_TESTER        
        do
        {
          RM2_LED(ON);
          battery_start_measure();
          TD_RTC_Delay(1000);
          RM2_LED(OFF);
          Voltage = battery_read_measure();
          if(++i > 600) break;   //wait no more than 10 min
        }
        while(Voltage < 0xA0); 
#endif  
        
   	while(1)	// main loop
	{
          
          if(Water5Loop())
          {
#ifndef SIMPLE_ONESEC_TESTER   
            if(Voltage > 0x94) PowLev = 3;         //if Ubat>3.2V then Power  = NormalPower_m3
              else if(Voltage > 0x80) if(++PowLev > 3) PowLev = 3;       ////if Ubat>3V then PowerLevel Up
              else if(Voltage < 25) { PowLev = 0;}
              else if(Voltage < 80)
              {
                  if(--PowLev < 0) PowLev = 0;     //if Ubat<2.8V then PowerLevel down
              }
#ifdef  DIN
        switch(PowLev)
              {
                case 0:
                  UNB_setPower(NormalPower_m6);
                 break;
                case 1:
                  UNB_setPower(NormalPower_m3);
                break;  
                case 2:
                  UNB_setPower(NormalPower);
                break;
                case 3:
                  UNB_setPower(HighPower);
                break;
              }
#endif      
#ifdef TIFFANY
              switch(PowLev)
              {
                case 0:
                  UNB_setPower(NormalPower_m12);
                 break;
                case 1:
                  UNB_setPower(NormalPower_m9);
                break;  
                case 2:
                  UNB_setPower(NormalPower_m6);
                break;
                case 3:
                  UNB_setPower(NormalPower_m3);
                break;
              }          
#endif
              
#ifdef  TIFFANY_14dBm
              switch(PowLev)
              {
                case 0:
                  UNB_setPower(NormalPower_m9);
                 break;
                case 1:
                  UNB_setPower(NormalPower_m6);
                break;  
                case 2:
                  UNB_setPower(NormalPower_m3);
                break;
                case 3:
                  UNB_setPower(NormalPower);
                break;
              }       
#endif
#ifdef  TIFFANY_17dBm
              switch(PowLev)
              {
                case 0:
                  UNB_setPower(NormalPower_m6);
                 break;
                case 1:
                  UNB_setPower(NormalPower_m3);
                break;  
                case 2:
                  UNB_setPower(NormalPower);
                break;
                case 3:
                  UNB_setPower(HighPower);
                break;
              }       
#endif
#endif              
          }

          TD_RTC_Sleep(); // wakes up every second
	}
}


void RM2_RTC_EverySecondEvent()// not every second anymore, td_rtc.c =) [S.Om]
{
        static uint8_t  timer = 0;
        
          
        GetInputs();  
        
#ifdef POWER_METER

        if(++timer%32) return;  //Tiffany power_meter
        
#endif
        
#ifdef WATER_METER
        
        if(++timer%16) return;  //Tiffany water_meter
#endif
        Water5OneSec();
}







