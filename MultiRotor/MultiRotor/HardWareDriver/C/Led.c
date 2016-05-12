/*------------------------------------
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \|     |//  `.
            /  \|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
           ���汣��       ����BUG

�ļ�����Led.h
��д�ߣ�WJ
����E-mail��cqrg01@163.com
���뻷����MDK 5.0
�汾�ţ�1.0
����ʱ��:2016-5-10
�޸�ʱ�䣺
˵����
1.�ɿ���led IO�ڳ�ʼ��
2.��ʼ��Ĭ�Ϲر�����LED��
------------------------------------*/
#include "Led.h"
//#include "config.h"
////#include "UART1.h"

LED_t LEDCtrl;
//�ӿ��Դ�
LEDBuf_t LEDBuf;

/****************************************************************************
*  ��������:  Led��ʼ������
*  ��    ��:  1.����Led�ӿ�IO�������
              2.�ر�����Led(����Ĭ�Ϸ�ʽ)
*  ��ڲ���:
*  ��    ��:
*  ��    ��:
*  ��    ��:  WJ
*  ʱ    ��:  2016-5-10 20:34:08
*  �޸�ʱ��:
*  ˵    ��:  Led�ӿڣ�
              Led1-- > PB12
              Led2-- > PB13
              ��ӦIO = 0������
****************************************************************************/

void LedInit(void)
{
  LedA_off;
  LedB_off;
  printf("״̬LED�Ƴ�ʼ�����...\r\n");
}

//�ײ���� ��10Hz
void LEDReflash(void)
{
  if (LEDBuf.bits.A)
    LedA_on;
  else
    LedA_off;

  if (LEDBuf.bits.B)
    LedB_on;
  else
    LedB_off;
}

//�¼�������
void LEDFSM(void)
{
//  static uint16_t cnt=0;
//  uint8_t event=0;

  switch (LEDCtrl.event)
  {
  case E_READY:
    if (++LEDCtrl.cnt >= 3)   //0 1 2 in loop, 0 on ,else off
      LEDCtrl.cnt = 0;
    if (LEDCtrl.cnt == 0)
      LEDBuf.byte = LA | LB;
    else
      LEDBuf.byte = 0;
    break;
  case E_CALI:
    LEDBuf.byte = LA | LB;
    break;
  case E_BAT_LOW:
    if (++LEDCtrl.cnt >= 3)   //0 1  in loop
      LEDCtrl.cnt = 0;
    if (LEDCtrl.cnt == 0)
      LEDBuf.byte = 0x03;
    else
      LEDBuf.byte = 0;
    break;
  case E_CALI_FAIL:
    if (++LEDCtrl.cnt >= 4)
      LEDCtrl.cnt = 0;
    if (LEDCtrl.cnt < 2)
      LEDBuf.byte = LA ;
    else
      LEDBuf.byte = LB;
    break;
  case E_LOST_RC:
    if (++LEDCtrl.cnt >= 4)
      LEDCtrl.cnt = 0;
    LEDBuf.byte = 1 << LEDCtrl.cnt ;
//        if(LEDCtrl.cnt==0)
//            LEDBuf.byte =LA;
//        else
//            LEDBuf.byte =0;
    break;
  case E_AUTO_LANDED:
    LEDBuf.byte = 0x03;
    break;

  case E_BatChg:
    LEDBuf.byte = 0x00;
    break;

  }

  LEDReflash();
}
