
#include  "kernel_cfg.h"       // Toppers Kernel Config Header
#include "hal.h"             // Toppers Header
#include "hal_extention.h"   // Toppers HAL_Extention Header
#include <stdio.h>

extern int my_servo_init(int num, char* moterName);
//********************
//サーボモータの初期科関数
//num: 1~3 :サーボの個数。　サーボを1個使うとき1。3個使うときは3。
//motername: sg90, sg5010 :仕様するモーターの名前。

extern void my_servo_control(long angle, int num);
//********************
//サーボモータの動作関数
//センターに移動後、指定した角度に移動する
//angle:0~180 :動作させる角度0が一番右端。90がセンター。180が一番左端。
//num:1(A3), 2(A4), 3(A5) :カッコ内はポート番号


////********************
//
struct servo_moter{
	float maxControlAngle;		//制御角（最大）
	long PWMcycle;				//PWM周期(us)
	long right180DutyValue;		//us
	long reft0DutyValue;		//us
	long centerDutyValue;		//us
};


