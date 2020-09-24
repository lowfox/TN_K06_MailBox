
#include "my_servo.h"

#define SERVO1 GPIO_ODR_ODR_0
#define SERVO2 GPIO_ODR_ODR_1
#define SERVO3 GPIO_ODR_ODR_0

/*
 * SG90
 * ■主な仕様
 * ・PWMサイクル：20mS
 * ・制御パルス：0.5ms~2.4ms
 * ・制御角：±約90°(180°)
 * ・配線：茶=GND、赤=電源[+]、橙=制御信号 [JRタイプ]
 * ・トルク：1.8kgf・cm
 * ・動作速度：0.1秒/60度
 	 >>300m秒/180度
 * ・動作電圧：4.8V(~5V)
 * ・温度範囲：0℃~55℃
 * ・外形寸法：22.2x11.8x31mm
 * ・重量：9g
 *
 *
 *  PWMの周波数は50Hzが良い。（周期20ms）
 *  パルスが1.5msの時、サーボは中央に回転
 *  パルスが2.4msの時、サーボは中央から右回転90°の位置に回転
 *  パルスが0.6msの時、サーボは中央から左回転90°に回転
 *
 */
struct servo_moter sg90={
	180,
	20 * 1000,
	2400,
	500,
	1500
};



/*
 * SG5010
 * ◆主な仕様
 * ・PWMサイクル：20mS
 * ・制御パルス：0.5ms~2.4ms
 * ・制御角：±約90°(180°)
 * ・配線：茶=GND、赤=電源[+]、橙=制御信号 [JRタイプ]
 * ・トルク：5.5kgf・cm(4.8V)、6.5kgf・cm(6V)
 * ・動作速度：0.2秒/60度(4.8V)、0.16秒/60度(6V)
 	 >>600ms/180秒
 * ・動作電圧：4.8V~6V
 * ・温度範囲：0℃~55℃
 * ・外形寸法：40.2x20.2x43.2mm
 * ・重量：38g
 *
 */
struct servo_moter sg5010={
	180,
	20 * 1000,
	2350,
	650,
	1500
};


struct servo_moter servoMoter[3];

int my_servo_init(int num, char* moterName){
	char s_num[3]={0,0,0};
	//エラーリターン
	if(num<1 || num > 3){
		return -1;
	}

	if(moterName==NULL){
		return -1;
	}

	sio_open(TASK_PORTID);                                   			// SIO Open
	sprintf(s_num,"%d",num);
	sio_write_string(TASK_PORTID, "num : ");
	sio_write_string(TASK_PORTID, s_num);

	if(strcmp(moterName,"sg90")==0){
		sio_write_string(TASK_PORTID, "  : sg90\n\r");
		servoMoter[num-1] = sg90;

	}else if(strcmp(moterName,"sg5010")==0){
		sio_write_string(TASK_PORTID, "  : sg5010\n\r");
		servoMoter[num-1] = sg5010;
	}else{
		return -1;
	}

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;    							// Peripheral_Clock= PB Enable
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;    							// Peripheral_Clock= PC Enable

	if(num == 1){
		GPIOB->MODER |= GPIO_MODER_MODER0_0;    							// IO_Mode=PB0(A3), Servo_Moter OUT
	}else if(num == 2){
		GPIOC->MODER |= GPIO_MODER_MODER1_0;    							// IO_Mode=PC1(A4), Servo_Moter OUT
	}else if(num == 3){
		GPIOC->MODER |= GPIO_MODER_MODER0_0;    							// IO_Mode=PC0(A5), Servo_Moter OUT
	}else{
		return -1;
	}

	return 0;

}



void  my_servo_control(long angle, int num) {
	char s_angle[10]={0,0,0,0,0,0,0,0,0,0};
	char s_dutyValue[10]={0,0,0,0,0,0,0,0,0,0};
	char s_num[3]={0,0,0};
	/*
	 *
	 * num=1(PB0:A3)
	 * num=2(PC1:A4)
	 * num=3(PC0:A5)
	 *
	 */

	//Duty値（HIGHの時間）=2000-(1000*(angle/180))

	if(angle<0 || angle > servoMoter[num-1].maxControlAngle || num < 1 || num > 3){
		return;
	}

	float dutyValue = 0;//DUTY値
	float ratio = (float)angle / servoMoter[num-1].maxControlAngle;

	//Duty値計算
	dutyValue = (float)servoMoter[num-1].right180DutyValue -
			(float)(servoMoter[num-1].right180DutyValue - servoMoter[num-1].reft0DutyValue) * ratio;


	//サーボの番号をSIOで表示
	sprintf(s_num, "%d", num);
	sio_write_string(TASK_PORTID, ">>>num:");
	sio_write_string(TASK_PORTID, s_num);
	sio_write_string(TASK_PORTID, "\n\r");

	//指定角度をSIOで表示
	sprintf(s_angle, "%d", (int)angle);
	sio_write_string(TASK_PORTID, ">>>angle:");
	sio_write_string(TASK_PORTID, s_angle);
	sio_write_string(TASK_PORTID, "\n\r");

	//Duty値をSIOで表示
	sprintf(s_dutyValue, "%d", (int)dutyValue);
	sio_write_string(TASK_PORTID, ">>>DutyValue=");
	sio_write_string(TASK_PORTID, s_dutyValue);
	sio_write_string(TASK_PORTID, "\n\r");


	switch(num){
	case 1:
		//中心に移動
		for (int i = 0; i < 30; i++) {
			GPIOB->ODR |= GPIO_ODR_ODR_0;
			sil_dly_nse(servoMoter[num-1].centerDutyValue * 1000 *2) throw();
			GPIOB->ODR &= ~GPIO_ODR_ODR_0;
			sil_dly_nse((servoMoter[num-1].PWMcycle - servoMoter[num-1].centerDutyValue) * 1000 *2) throw();
		}

		//指定角度に移動
		for (int i = 0; i < 30; i++) {
			GPIOB->ODR |= GPIO_ODR_ODR_0;
			sil_dly_nse(dutyValue * 1000 *2 ) throw();
			GPIOB->ODR &= ~GPIO_ODR_ODR_0;
			sil_dly_nse((servoMoter[num-1].PWMcycle - dutyValue) * 1000 *2) throw();
		}
		break;

	case 2:
		//中心に移動
		for (int i = 0; i < 30; i++) {
			GPIOC->ODR |= GPIO_ODR_ODR_1;
			sil_dly_nse(servoMoter[num-1].centerDutyValue * 1000 *2) throw();
			GPIOC->ODR &= ~GPIO_ODR_ODR_1;
			sil_dly_nse((servoMoter[num-1].PWMcycle - servoMoter[num-1].centerDutyValue)* 1000 *2) throw();
		}

		//指定角度に移動
		for (int i = 0; i < 30; i++) {
			GPIOC->ODR |= GPIO_ODR_ODR_1;
			sil_dly_nse(dutyValue * 1000 *2 ) throw();
			GPIOC->ODR &= ~GPIO_ODR_ODR_1;
			sil_dly_nse((servoMoter[num-1].PWMcycle - dutyValue) * 1000 *2) throw();
		}
		break;

	case 3:
		//中心に移動
		for (int i = 0; i < 30; i++) {
			GPIOC->ODR |= GPIO_ODR_ODR_0;
			sil_dly_nse(servoMoter[num-1].centerDutyValue * 1000 *2 ) throw();
			GPIOC->ODR &= ~GPIO_ODR_ODR_0;
			sil_dly_nse((servoMoter[num-1].PWMcycle - servoMoter[num-1].centerDutyValue) * 1000 *2) throw();
		}

		//指定角度に移動
		for (int i = 0; i < 30; i++) {
			GPIOC->ODR |= GPIO_ODR_ODR_0;
			sil_dly_nse(dutyValue * 1000 *2) throw();
			GPIOC->ODR &= ~GPIO_ODR_ODR_0;
			sil_dly_nse((servoMoter[num-1].PWMcycle - dutyValue) * 1000 *2 ) throw();
		}
		break;

	default:
		break;
	}
	return;

}
