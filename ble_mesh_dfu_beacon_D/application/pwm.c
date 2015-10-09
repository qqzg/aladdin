/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 * @defgroup pwm_example_main main.c
 * @{
 * @ingroup pwm_example
 * 
 * @brief  PWM Example Application main file.
 *
 * This file contains the source code for a sample application using PWM.
 *
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_timer.h"
#include "app_gpiote.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "bsp.h"
#include "pwm.h"
#define APP_TIMER_PRESCALER      0                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS     (1 + BSP_APP_TIMERS_NUMBER) /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE  2                           /**< Size of timer operation queues. */

#define BUTTON_PREV_ID           0
#define BUTTON_NEXT_ID           1

#ifdef BSP_LED_0
    #define PWM_OUTPUT_PIN_NUMBER (BSP_LED_0)               /**< Pin number for PWM output. */
#endif
#ifndef PWM_OUTPUT_PIN_NUMBER
    #error "Please define PWM_OUTPUT_PIN_NUMBER"
#endif


#ifdef BSP_LED_1
    #define PWM_OUTPUT_PIN_NUMBER_1 (BSP_LED_1)               /**< Pin number for PWM output. */
#endif
#ifndef PWM_OUTPUT_PIN_NUMBER_1
    #error "Please define PWM_OUTPUT_PIN_NUMBER_1"
#endif


#ifdef BSP_LED_2
    #define PWM_OUTPUT_PIN_NUMBER_2 (BSP_LED_2)               /**< Pin number for PWM output. */
#endif
#ifndef PWM_OUTPUT_PIN_NUMBER_2
    #error "Please define PWM_OUTPUT_PIN_NUMBER_2"
#endif

#define MAX_SAMPLE_LEVELS        (0xFF)              /**< Maximum number of sample levels. */
#define INCREMENT_VALUE          (256)               /**< Value by which the pulse_width is incremented/decremented for each event. */
#define TIMER_PRESCALERS         (1U)                /**< Prescaler setting for timer. */

int r_value = 0x01;//MAX_SAMPLE_LEVELS - 2;              /**< Fill of pwm signal (in range <1,MAX_SAMPLE_LEVELS-1>). */

int g_value = 0x01;//MAX_SAMPLE_LEVELS - 2;  

int b_value = 0x01;//MAX_SAMPLE_LEVELS - 2;  

static bool is_power_on = true;

void pwm_evt_handler(uint16_t r, uint16_t g, uint16_t b)	
{
	//add for turn on / turn off{
	
	if(r == 0xff && g == 0xff && b == 0xff) //turn off
	{
		if(is_power_on == true)
		{
			is_power_on = false;
			stop_pwm();
		}
		return ;
	}
	if(is_power_on == false)
	{
		is_power_on = true;
		restart_pwm();
	}
	//}
	if(r >= (MAX_SAMPLE_LEVELS))
		r = MAX_SAMPLE_LEVELS-1;
	if(r <= 0 )
		r = 1;
	//timer2
	if(NRF_TIMER2->CC[1] != r)
	{
		NRF_TIMER2->TASKS_STOP = 1;
		NRF_TIMER2->TASKS_CLEAR = 1;
		NRF_TIMER2->CC[1] = r;
		//nrf_gpiote_task_config(0, PWM_OUTPUT_PIN_NUMBER, \
							   NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
		nrf_gpiote_task_config(0, PWM_OUTPUT_PIN_NUMBER, \
							   NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
		
		NRF_TIMER2->TASKS_START = 1;
	}
	
#if 1	
	
	if(g >= (MAX_SAMPLE_LEVELS))
		g = MAX_SAMPLE_LEVELS-1 ;
	if(b >= (MAX_SAMPLE_LEVELS))
		b = MAX_SAMPLE_LEVELS-1 ;
	if(g <= 0 )
		g = 1;
	if(b <= 0 )
		b = 1;
	//timer1
	if(NRF_TIMER1->CC[1] != g  || NRF_TIMER1->CC[0] != b)
	{
		// green
		NRF_TIMER1->TASKS_STOP = 1;
		NRF_TIMER1->TASKS_CLEAR = 1;
		NRF_TIMER1->CC[1] = g;
		nrf_gpiote_task_config(1, PWM_OUTPUT_PIN_NUMBER_1, \
							   NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
		//back
		NRF_TIMER1->CC[0] = b;
			nrf_gpiote_task_config(2, PWM_OUTPUT_PIN_NUMBER_2, \
                           NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
		
		
		NRF_TIMER1->TASKS_START = 1;
	}
	
#endif	
	
}


/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    switch (evt)
    {
        case BSP_EVENT_KEY_0:
            r_value -= INCREMENT_VALUE;

            if ( r_value <= 0 )
                r_value = 1;
            break;

        case BSP_EVENT_KEY_1:
            r_value += INCREMENT_VALUE;

            if ( r_value >= MAX_SAMPLE_LEVELS )
                r_value = MAX_SAMPLE_LEVELS - 1;
            break;

        default:
            break; // no implementation needed
    }
}


/** @brief Function for handling timer 2 peripheral interrupts.
 */
void TIMER2_IRQHandler(void)
{
	/* 

    if ((NRF_TIMER2->EVENTS_COMPARE[1] != 0) &&
        ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE1_Msk) != 0))
    {
        // Sets the next CC1 value
        NRF_TIMER2->EVENTS_COMPARE[1] = 0;
        NRF_TIMER2->CC[1]             = (NRF_TIMER2->CC[1] + MAX_SAMPLE_LEVELS);

        // Every other interrupt CC0 and CC2 will be set to their next values.
        if (cc0_turn)
        {
            NRF_TIMER2->CC[0] = NRF_TIMER2->CC[1] + r_value;
        }
        else
        {
            NRF_TIMER2->CC[2] = NRF_TIMER2->CC[1] + r_value;
        }
        // Next turn the other CC will get its value.
        cc0_turn = !cc0_turn;
    }
	*/
}


/** @brief Function for initializing the Timer 2 peripheral.
 */
static void timer2_init(void)
{
    // Start 16 MHz crystal oscillator .
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    // Wait for the external oscillator to start up.
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
   {
        // Do nothing.
    }

    NRF_TIMER2->MODE      = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->BITMODE   = TIMER_BITMODE_BITMODE_08Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER2->PRESCALER = TIMER_PRESCALERS;

    // Clears the timer, sets it to 0.
    NRF_TIMER2->TASKS_CLEAR = 1;

    // Load the initial values to TIMER2 CC registers.
//    NRF_TIMER2->CC[0] = MAX_SAMPLE_LEVELS + r_value;
    NRF_TIMER2->CC[1] = r_value;//MAX_SAMPLE_LEVELS;

    // CC2 will be set on the first CC1 interrupt.
    NRF_TIMER2->CC[2] = MAX_SAMPLE_LEVELS;

    // Interrupt setup.
    NRF_TIMER2->INTENSET = (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos);
}


static void timer1_init(void)
{
    // Start 16 MHz crystal oscillator .
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    // Wait for the external oscillator to start up.
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
        // Do nothing.
    }

    NRF_TIMER1->MODE      = TIMER_MODE_MODE_Timer;
    NRF_TIMER1->BITMODE   = TIMER_BITMODE_BITMODE_08Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER1->PRESCALER = TIMER_PRESCALERS;

    // Clears the timer, sets it to 0.
    NRF_TIMER1->TASKS_CLEAR = 1;

    // Load the initial values to TIMER2 CC registers.
//    NRF_TIMER2->CC[0] = MAX_SAMPLE_LEVELS + r_value;
    NRF_TIMER1->CC[1] = g_value;

    // CC2 will be set on the first CC1 interrupt.
    NRF_TIMER1->CC[2] = MAX_SAMPLE_LEVELS;

	NRF_TIMER1->CC[0] = b_value;
	NRF_TIMER1->CC[3] = MAX_SAMPLE_LEVELS;
    // Interrupt setup.
    NRF_TIMER1->INTENSET = (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos);
}





/** @brief Function for initializing the GPIO Tasks/Events peripheral.
 */
static void gpiote_init(void)
{
    // Configure PWM_OUTPUT_PIN_NUMBER as an output.
    nrf_gpio_cfg_output(PWM_OUTPUT_PIN_NUMBER);

    // Configure GPIOTE channel 0 to toggle the PWM pin state
    // @note Only one GPIOTE task can be connected to an output pin.
    nrf_gpiote_task_config(0, PWM_OUTPUT_PIN_NUMBER, \
                           NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
	
	
	// Configure PWM_OUTPUT_PIN_NUMBER_1 as an output.
    nrf_gpio_cfg_output(PWM_OUTPUT_PIN_NUMBER_1);

    // Configure GPIOTE channel 1 to toggle the PWM pin state
    // @note Only one GPIOTE task can be connected to an output pin.
    nrf_gpiote_task_config(1, PWM_OUTPUT_PIN_NUMBER_1, \
                           NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
	
	
	// Configure PWM_OUTPUT_PIN_NUMBER_2 as an output.
    nrf_gpio_cfg_output(PWM_OUTPUT_PIN_NUMBER_2);
	// Configure GPIOTE channel 2 to toggle the PWM pin state
    // @note Only one GPIOTE task can be connected to an output pin.
	nrf_gpiote_task_config(2, PWM_OUTPUT_PIN_NUMBER_2, \
                           NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
						   
	nrf_gpio_pin_write(PWM_OUTPUT_PIN_NUMBER, 0);
	nrf_gpio_pin_write(PWM_OUTPUT_PIN_NUMBER_1, 0);
	nrf_gpio_pin_write(PWM_OUTPUT_PIN_NUMBER_2, 0);
}


/** @brief Function for initializing the Programmable Peripheral Interconnect peripheral.
 */
static void ppi_init(void)
{
    // Configure PPI channel 0 to toggle PWM_OUTPUT_PIN on every TIMER2 COMPARE[0] match.
//    NRF_PPI->CH[0].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[0];
//    NRF_PPI->CH[0].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[0];

    // Configure PPI channel 0 to toggle PWM_OUTPUT_PIN_1 on every TIMER2 COMPARE[1] match.
    NRF_PPI->CH[1].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[1];
    NRF_PPI->CH[1].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[0];

    // Configure PPI channel 1 to toggle PWM_OUTPUT_PIN_1 on every TIMER2 COMPARE[2] match.
    NRF_PPI->CH[2].EEP = (uint32_t)&NRF_TIMER2->EVENTS_COMPARE[2];
    NRF_PPI->CH[2].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[0];

    // Enable PPI channels 0-2.
//    NRF_PPI->CHEN = (PPI_CHEN_CH0_Enabled << PPI_CHEN_CH0_Pos)
//                    | (PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos)
//                    | (PPI_CHEN_CH2_Enabled << PPI_CHEN_CH2_Pos);
	
	
	// Configure PPI channel 1 to toggle PWM_OUTPUT_PIN_1 on every TIMER1 COMPARE[1] match.
	NRF_PPI->CH[3].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[1];
    NRF_PPI->CH[3].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[1];

    // Configure PPI channel 1 to toggle PWM_OUTPUT_PIN_1 on every TIMER2 COMPARE[2] match.
    NRF_PPI->CH[4].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[2];
    NRF_PPI->CH[4].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[1];
	
	// Configure PPI channel 2 to toggle PWM_OUTPUT_PIN_2 on every TIMER2 COMPARE[0] match.
	  NRF_PPI->CH[5].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[0];
      NRF_PPI->CH[5].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[2];

    // Configure PPI channel 2 to toggle PWM_OUTPUT_PIN_2 on every TIMER2 COMPARE[3] match.
    NRF_PPI->CH[6].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[3];
    NRF_PPI->CH[6].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[2];

	NRF_PPI->CHEN =   (PPI_CHEN_CH1_Enabled << PPI_CHEN_CH1_Pos)
                    | (PPI_CHEN_CH2_Enabled << PPI_CHEN_CH2_Pos)
					|  (PPI_CHEN_CH3_Enabled << PPI_CHEN_CH3_Pos)
                    | (PPI_CHEN_CH4_Enabled << PPI_CHEN_CH4_Pos)
					|  (PPI_CHEN_CH5_Enabled << PPI_CHEN_CH5_Pos)
                    | (PPI_CHEN_CH6_Enabled << PPI_CHEN_CH6_Pos);
}

#if 0
static void bsp_configuration()
{
 
    APP_GPIOTE_INIT(1);

    uint32_t err_code;
    err_code = bsp_init(BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), bsp_evt_handler);
    err_code = bsp_buttons_enable( (1 << BUTTON_PREV_ID) | (1 << BUTTON_NEXT_ID) );
    APP_ERROR_CHECK(err_code);
}
#endif

/**
 * @brief Function for application main entry.
 */
void pwm_init()
{
	timer2_init();
	timer1_init();
    gpiote_init();
//    bsp_configuration();
    ppi_init();

    // Enabling constant latency as indicated by PAN 11 "HFCLK: Base current with HFCLK 
    // running is too high" found at Product Anomaly document found at
    // https://www.nordicsemi.com/eng/Products/Bluetooth-R-low-energy/nRF51822/#Downloads
    //
    // @note This example does not go to low power mode therefore constant latency is not needed.
    //       However this setting will ensure correct behaviour when routing TIMER events through 
    //       PPI (shown in this example) and low power mode simultaneously.    
	NRF_POWER->TASKS_CONSTLAT = 1;



}

void start_timer2()
{

    *(uint32_t *)0x4000AC0C = 1;    
    // Start the timer.
    NRF_TIMER2->TASKS_START = 1;
}


void start_timer1()
{
	// Start the timer.
    NRF_TIMER1->TASKS_START = 1;
}
/** @} */
void stop_pwm()
{
	NRF_TIMER2->TASKS_STOP = 1;
	NRF_TIMER1->TASKS_STOP = 1;
	
	NRF_TIMER2->TASKS_CLEAR = 1;
	NRF_TIMER2->CC[1] = r_value;
		//nrf_gpiote_task_config(0, PWM_OUTPUT_PIN_NUMBER, \
							   NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
	nrf_gpiote_task_config(0, PWM_OUTPUT_PIN_NUMBER, \
							   NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
	
	
	NRF_TIMER1->TASKS_CLEAR = 1;
	NRF_TIMER1->CC[1] = g_value;
	nrf_gpiote_task_config(1, PWM_OUTPUT_PIN_NUMBER_1, \
						   NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
	//back
	NRF_TIMER1->CC[0] = b_value;
		nrf_gpiote_task_config(2, PWM_OUTPUT_PIN_NUMBER_2, \
					   NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);

//add for close led 	
	nrf_gpio_pin_write(PWM_OUTPUT_PIN_NUMBER, 0);
	nrf_gpio_pin_write(PWM_OUTPUT_PIN_NUMBER_1, 0);
	nrf_gpio_pin_write(PWM_OUTPUT_PIN_NUMBER_2, 0);
	
}

void restart_pwm()
{
	NRF_TIMER2->TASKS_START = 1;
	NRF_TIMER1->TASKS_START = 1;
}

