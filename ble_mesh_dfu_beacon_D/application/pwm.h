#ifndef __PWM_H__

#define __PWM_H__
#include "stdint.h"
void pwm_init(void);
void start_timer2(void);
void start_timer1(void);
void stop_timer2(void);
void start_timer0(void);
void pwm_evt_handler(uint16_t r, uint16_t g, uint16_t b);
void stop_pwm(void);
void restart_pwm(void);
#endif


