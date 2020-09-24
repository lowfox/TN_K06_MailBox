
#include  "kernel_cfg.h"       // Toppers Kernel Config Header
#include "hal.h"             // Toppers Header
#include "hal_extention.h"   // Toppers HAL_Extention Header
#include <stdio.h>

extern void my_PWM_init(void);
//********************
//pwm制御初期化関数(D8(PA9))
//出力ポートの設定。ポートはD8固定。



extern void my_PWM_control(int num, float pwm_cycle, float duty_ratio);
//********************
//pwm制御関数(D8(PA9))
//PWM周期、duty比を設定して、PWMの1周期する関数
//num:1(D8),2(D9),0(全ポート） :　ポート番号
//pwm_cycle: 0.0f~(ms):PWM周期
//duty_ratio:0.0f~1.0f:duty比

