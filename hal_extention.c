//********************************************************************
//  STM32_Toppers: HAL_Extention_Functions  (Rev.05)
//        Enhanced to support easy use of GPIO functions for Driver.
//        Enhanced to support detection of SWT Timeout.
//																Sumida
//********************************************************************
#include "kernel_cfg.h"             // Toppers Kernel Config Header
#include "hal.h"					// Toppers Header
#include "hal_extention.h"			// Toppers HAL_Extention Header (Must be >Rev.04)

//**************************************
//  SIO_Function (UART2)
// 	  At the moment, "portid" must be 
//    "TASK_PORTID" shareing with Syslog
//**************************************
#ifndef non_terminal_sio
	#define  string_end '\r'					// Communicate with Terminal
#else
	#define  string_end '\0'					// Communicate with Program
#endif
// **** Open SIO Port ****
void sio_open(ID portid) {
 	ER ercd;
   	ercd = serial_opn_por(portid);
   	if (ercd < 0 && MERCD(ercd) != E_OBJ) {  	// Port Shareing with Syslog is not Error
      	syslog_1(LOG_NOTICE,"SIO Open Err (ercd=%d); Prog Terminate",ercd);
       	ext_tsk();
   	}else {
       	serial_ctl_por(portid,(IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)); // Set Controls
   	}
}
// **** Write String ****
void sio_write_string(ID portid, char* w_pnt) {
  	for (; *w_pnt!='\0'; w_pnt++) {
      	serial_wri_dat(portid, w_pnt,1);
   	}
}
// **** Write Character ****
void sio_write_char(ID portid, char* w_pnt) {
   	serial_wri_dat(portid, w_pnt,1);
}
// **** Read String ****
void sio_read_string(ID portid, char* r_pnt) {
   	for (;; r_pnt++) {
      	serial_rea_dat(portid, r_pnt, 1);
      	if (*r_pnt == string_end){           	// If Enter_Key, Put \0 to end of String
          	*r_pnt = '\0';
          	break;
	  	}
	  	#ifndef non_terminal_sio
      	if (*r_pnt == '\b'){                	// If BackSpace, Rewind the Pointer
          	r_pnt=r_pnt -2;
      	}else {                                 // EchoBack Input Char to Terminal
          	serial_wri_dat(portid, r_pnt, 1);
      	}
	  	#endif
   	}
   	#ifndef non_terminal_sio
   	sio_write_string(TASK_PORTID,"\n\r");      	// New Line
   	#endif
}
// **** Read Character ****
void sio_read_char(ID portid, char* r_pnt){
	char temp;
	char *t=&temp;
	serial_rea_dat(portid,t,1);
	serial_wri_dat(portid,t,1);					// EchoBack Input Char to Terminal
	*r_pnt=*t;
	for (; *t != string_end; ){					// Dummy Read until Enter_Key
		serial_rea_dat(portid,t,1);
	}
	sio_write_string(TASK_PORTID,"\n\r");		// New Line
}



//**************************************
//  GPIO_Function (LED,SWT,Servo・・)        ...(Rev.4 Enhanced functions)
//     char port[4]: port[0]    ='portG'
//                   port[1]~[3]="port"
//     use_ex.for PA12 (OnBoard LED2)
//        char port[]="A12";	 	 : Define portG&port
//        clock_set(port);			 : Set Port Clock
//        moder_set(port, p_output); : Set Port Mode
//        portout_set(port, p_on);   : Set Port Operation
//     use_ex.for PC13 (onBoard SWT)
//        char port[]="C13";
//		  clock_set(port);
//        moder_set(port, p_input);
//        portin_set(port, p_pullup);: Set Port Pullup
//        int rtn = portin_set(port, p_waitforinput,5); : Wait for SWT push 5_Sec.
//            rtn 0  : SWT pushed(OK)                    (if <=0, wait for forever)
//            rtn -1 : SWT timeout
//**************************************
// **** Set Port_Clock ****
void clock_set(char* port){
	switch(*port){
		case 'A' :  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  break;
		case 'B' :  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;  break;
		case 'C' :  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;  break;
	}
}
// **** Set Port_Mode ****
void moder_set(char* port,char op){
	unsigned long moder = 0x1 << (atol(port+1)*2);// set GPIO_MODER_MODERx_0
	switch(*port){
		case 'A' :  switch(op) {
						case p_output : GPIOA->MODER |=  moder; break;
						case p_input  : GPIOA->MODER &= ~moder; break;
					}break;
		case 'B' :  switch(op) {
						case p_output : GPIOB->MODER |=  moder; break;
						case p_input  : GPIOB->MODER &= ~moder; break;
					}break;
		case 'C' :  switch(op) {
						case p_output : GPIOC->MODER |=  moder; break;
						case p_input  : GPIOC->MODER &= ~moder; break;
					}break;
	}
}
// **** Set PortOut_Operation ****
void portout_set(char* port,char op){
	unsigned int odr = 0x1 << (atoi(port+1));	// set GPIO_ODR_ODR_x
    switch(*port){
    	case 'A' : 	switch(op) {
						case p_on    :  GPIOA->ODR |=  odr; break;
						case p_off   :  GPIOA->ODR &= ~odr; break;
						case p_invert:  GPIOA->ODR ^=  odr; break;
					}break;
		case 'B' : 	switch(op) {
						case p_on    :  GPIOB->ODR |=  odr; break;
						case p_off   :  GPIOB->ODR &= ~odr; break;
						case p_invert:  GPIOB->ODR ^=  odr; break;
					}break;
		case 'C' : 	switch(op) {
						case p_on    :  GPIOC->ODR |=  odr; break;
						case p_off   :  GPIOC->ODR &= ~odr; break;
						case p_invert:  GPIOC->ODR ^=  odr; break;
					}break;
    }
}
// **** Set PortIn_Operation ****
int portin_set(char* port, char op, ...){			// 3rd arg = timeout_value in Sec.
	#define  push_wait 100							// Timeout check cycle time
	unsigned int  idr = 0x1 << (atoi(port+1));		// set GPIO_IDR_IDR_x
	unsigned long pupdr = 0x1 << (atol(port+1)*2);	// set GPIO_PUPDR_PUPDRx_0
	int rspcd = 0;									// return_code : OK=0, Timeout=-1
	int timeout_val;
	va_list args;
	va_start(args,op);
	timeout_val=va_arg(args,int)*1000;				// set (SWT)waitforinput timeout_value(ms)
	va_end(args);

    switch(*port){
    	case 'A' : 	switch(op) {
						case p_pullup:  GPIOA->PUPDR |=  pupdr; break;
						case p_waitforinput :
 							 if (timeout_val > 0){			 	// required timer_set (wait for forever: timeout_val <=0)
								for(; ((GPIOA->IDR & idr)&&(timeout_val > 0)); timeout_val -= push_wait){  // count timeout
										dly_tsk(push_wait);		// wait a moment
								}
								if (timeout_val <= 0){			// timeout
									rspcd = -1;
									break;
								}
							 }
							 while   (GPIOA->IDR & idr) { dly_tsk(push_wait); }
							 while (!(GPIOA->IDR & idr)){ dly_tsk(push_wait); }
							 break;
					}break;
		case 'B' : 	switch(op) {
						case p_pullup:  GPIOB->PUPDR |=  pupdr; break;
						case p_waitforinput :
 							 if (timeout_val > 0){
								for(; ((GPIOB->IDR & idr)&&(timeout_val > 0)); timeout_val -= push_wait){
										dly_tsk(push_wait);
								}
								if (timeout_val <= 0){
									rspcd = -1;
									break;
								}
							 }
							 while 	 (GPIOB->IDR & idr) { dly_tsk(push_wait); }
							 while (!(GPIOB->IDR & idr)){ dly_tsk(push_wait); }
							 break;
					}break;
		case 'C' : 	switch(op) {
						case p_pullup:  GPIOC->PUPDR |=  pupdr; break;
						case p_waitforinput :
 							if (timeout_val > 0){
								for(; ((GPIOC->IDR & idr)&&(timeout_val > 0)); timeout_val -= push_wait){
										dly_tsk(push_wait);
								}
								if (timeout_val <= 0){
									rspcd = -1;
									break;
								}
							 }
							 while   (GPIOC->IDR & idr) { dly_tsk(push_wait); }
							 while (!(GPIOC->IDR & idr)){ dly_tsk(push_wait); }
							 break;
					}break;
    }
	return rspcd;
}


//**************************************
//  _sbrk Function ..for 問題回避 (角田)
//**************************************
char * stack_ptr asm("sp");
caddr_t _sbrk_r (struct _reent *r, int incr){
  extern char   end asm ("end");
  static char * heap_end;
  char * prev_heap_end;
  if (heap_end == NULL){
     heap_end = & end;
  }
  prev_heap_end = heap_end;
  if (heap_end + incr > stack_ptr){
     errno = ENOMEM;
     return (caddr_t) -1;
  }
  heap_end += incr;
  return (caddr_t) prev_heap_end;
}

