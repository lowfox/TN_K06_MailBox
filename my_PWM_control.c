#include "my_PWM_control.h"

void my_PWM_init(void){

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;    							// Peripheral_Clock= PA Enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;    							// Peripheral_Clock= PC Enable

	GPIOA->MODER |= GPIO_MODER_MODER9_0;    							// IO_Mode=PA9(D8), set OUT
	GPIOC->MODER |= GPIO_MODER_MODER7_0;    							// IO_Mode=PA9(D8), set OUT
}


void my_PWM_control(int num, float pwm_cycle, float duty_ratio){
	if(pwm_cycle<0.0f){
		return;
	}
	if(duty_ratio>1.0f || duty_ratio<0.0f){
		return;
	}
	static float ON_TIME=0.0f;
	static float OFF_TIME=0.0f;

	ON_TIME=pwm_cycle*duty_ratio;
	OFF_TIME=pwm_cycle-ON_TIME;

	if(num==1 || num==0){
		GPIOA->ODR |= GPIO_ODR_ODR_9;//ON
	}
	if(num==2 || num==0){
		GPIOC->ODR |= GPIO_ODR_ODR_7;//ON
	}
	sil_dly_nse((ON_TIME) *1000 * 1000 *2) throw();
	if(num==1 || num==0){
		GPIOA->ODR &= ~GPIO_ODR_ODR_9;//OFF
	}
	if(num==2 || num==0){
		GPIOC->ODR &= ~GPIO_ODR_ODR_7;//OFF
	}
	sil_dly_nse((OFF_TIME) *1000 * 1000 *2) throw();
}
