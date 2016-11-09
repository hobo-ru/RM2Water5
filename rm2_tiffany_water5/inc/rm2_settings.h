//
//  RM2 Defines and settings
//  12.07.2015 S.Omelchenko
//  somelchenko@strij.net
//

#ifndef __RM2SETTINGS_H
#define __RM2SETTINGS_H

#include "td_module.h"
#include "defines.h"



#ifdef DIN      //DIN

#define OUT1_PORT       USR1_PORT
#define OUT1_PIN        USR1_BIT
#define OUT2_PORT       USR0_PORT
#define OUT2_PIN        USR0_BIT

#define IN1_PORT        TIM1_PORT
#define IN1_PIN         TIM1_BIT
#define IN2_PORT        TIM2_PORT
#define IN2_PIN         TIM2_BIT


#define N_OF_BUTTONS	2

#define SB1_PORT		IO1_PORT
#define SB1_PIN			IO1_BIT

#define BUTTON_PORT		SB1_PORT	// for rm2_hal.c
#define BUTTON_PIN		SB1_PIN

#define JP2_PORT		RX_PORT
#define JP2_PIN			RX_BIT

#define JP2_GND_PORT	        TX_PORT		// both JP2 pins are GPIO,
#define JP2_GND_PIN		TX_BIT		// need to tie one to GND at init time for button operation

#define LED_PORT		ADC1_PORT
#define LED_PIN			ADC1_BIT


#else   //TIFFANY

#define OUT1_PORT       ADC0_PORT
#define OUT1_PIN        ADC0_BIT
#define OUT2_PORT       DAC0_PORT
#define OUT2_PIN        DAC0_BIT

#define IN1_PORT        USR0_PORT
#define IN1_PIN         USR0_BIT
#define IN2_PORT        ADC1_PORT
#define IN2_PIN         ADC1_BIT

#define N_OF_BUTTONS	2

#define SB1_PORT		USR1_PORT
#define SB1_PIN			USR1_BIT

#define BUTTON_PORT		SB1_PORT	// for rm2_hal.c
#define BUTTON_PIN		SB1_PIN

#define JP2_PORT		RX_PORT
#define JP2_PIN			RX_BIT

#define JP2_GND_PORT	        TX_PORT		// both JP2 pins are GPIO,
#define JP2_GND_PIN		TX_BIT		// need to tie one to GND at init time for button operation

#define LED_PORT		TIM2_PORT
#define LED_PIN			TIM2_BIT

#endif


#endif