
#ifndef		__STC15_CONFIG_H
#define		__STC15_CONFIG_H


/*********************************************************/

//#define MAIN_Fosc		22118400L	//������ʱ��
//#define MAIN_Fosc		12000000L	//������ʱ��
#define MAIN_Fosc		11059200L	//������ʱ��
//#define MAIN_Fosc		 5529600L	//������ʱ��
//#define MAIN_Fosc		24000000L	//������ʱ��


/*********************************************************/

#include "STC15.h"
//#include "stc15xx_it.h"//�жϺ���

//��ȥ����Ҫʹ�õĿ⺯��ͷ�ļ�����ע��
//#include "STC15xx_ADC.h"
//#include "STC15xx_delay.h"
//#include "STC15xx_EEPROM.h"
//#include "STC15xx_EXTI.h"
//#include "STC15xx_GPIO.h"
//#include "STC15xx_PCA.h"
//#include "STC15xx_soft_uart.h"
//#include "STC15xx_timer.h"
//#include "STC15xx_USART1.h"


/**************************************************************************/

#define Main_Fosc_KHZ	(MAIN_Fosc / 1000)

/***********************************************************/

#endif
