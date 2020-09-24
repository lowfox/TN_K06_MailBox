//**********************************************************************
//  STM32_Toppers: HAL_Extention_Header (Rev.05)
//        Enhanced to support easy use of GPIO functions for Driver.
//        Enhanced to support detection of SWT Timeout.
//																  Sumida
//**********************************************************************
//**************************************
// General #include
//**************************************
#include "stm32f4xx_hal.h"
#include "CMSIS\Device\ST\STM32F4xx\Include\stm32f4xx.h"
#include "sil.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>					// (for 可変個引数)
#include <time.h>					// (for rand関数)

//**************************************
// SIO_Function(UART2) extern
//**************************************
	    //#define non_terminal_sio	// 通信相手がProgramの場合Commentを外す
extern void sio_open(ID);
extern void sio_write_string(ID,char*);
extern void sio_write_char(ID,char*);
extern void sio_read_string(ID,char*);
extern void sio_read_char(ID,char*);

//**************************************
// GPIO_Function(LED,SWT,Servo..) extern   ...(Rev.4 Enhanced functions)
//**************************************
extern void  clock_set(char*);
extern void  moder_set(char*,char);
		#define	 p_output  1		// output mode
		#define  p_input   0		// input mode
extern void  portout_set(char*,char);
		#define  p_off     0		// off(0)
		#define  p_on      1		// on(1)
		#define  p_invert  2		// 反転(1<->0)
extern int   portin_set(char*,char,...); 
		// 可変個引数: 3rd_arg=SWT Push_Wait TimeoutValue in Sec, (if 0: Wait_Forever)
		// return code: 0=SWT_Pushed(OK), -1=Timeout
		#define  p_pullup  0		// pullup
		#define  p_waitforinput  1	// SWT押下&解放待ち

//***************************************
// _sbrk Function extern
//***************************************
extern caddr_t _sbrk_r (struct _reent *r, int incr);

//***************************************
// TIM2_Timer Attribute definition
//***************************************
#define INTNUM_TIM2	(16+28) // for TIM2 (Base16 + TIM2_Int_Code 28)
#define INTPRI_TIM2	-3		// Int Priority
#define INTATR_TIM2	0		// Attributes

//***************************************
// TIM3_Timer Attribute definition
//***************************************
#define INTNUM_TIM3	(16+29) // for TIM3 (Base16 + TIM2_Int_Code 29)
#define INTPRI_TIM3	-3		// Int Priority
#define INTATR_TIM3	0		// Attributes

