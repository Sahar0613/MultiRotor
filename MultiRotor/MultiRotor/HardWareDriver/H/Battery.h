#ifndef _Battery_H_
#define _Battery_H_
#include "stm32f1xx_hal.h"


#define BAT_CHK_PRD 5000	//ms
#define BAT_ALARM_VAL  3.7	  //on ground
#define BAT_CHG_VAL    1.0	  // charge battery val.  unit :v



//��ѹ��Ϣ�ṹ��
typedef struct
{
    
int    BatteryAD;             //��ѹADֵ
float  BatteryVal;            //��ѹʵ��ֵ
float  BatReal;               //��ص�ʵ�ʵ�ѹ�������ñ��
float  ADRef;                 //AD�ο�Դ��ѹ�������ǵ�Ƭ�������ѹ��һ����3.3V���ң�Ҫʵ��
float  ADinput;               //AD���������ѹ--->R15��R17�����ĺ��̵�ѹ
float  Bat_K;                 //�����ѹֵϵ�������ڵ�ѹУ׼
int    BatteryADmin;          //��ѹ����
char   alarm;									//����λ
char   chargeSta;							//���״̬
}Bat_Typedef;



void BatteryCheckInit(void);	
uint8_t Get_Adc_Average(uint8_t ch,uint8_t times);             
int GetBatteryAD(void);     
void BatteryCheck(void);
//ʵ����һ����ѹ��Ϣ�ṹ��
extern Bat_Typedef Battery;


#endif
                
        



