/*
 * led.c
 *
 *  Created on: Oct 26, 2025
 *      Author: M S I
 */


#include "led.h"

void toggle_led1(){
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}
void toggle_led2(){
	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}
void toggle_led3(){
	HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
}
void toggle_led4(){
	HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
}
void toggle_led5(){
	HAL_GPIO_TogglePin(LED5_GPIO_Port, LED5_Pin);
}
