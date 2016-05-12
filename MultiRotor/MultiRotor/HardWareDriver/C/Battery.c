/*


Battery.c file
��д�ߣ�WJ
����E-mail:cqrg01@163.com
���뻷����MDK  Version: v5.15
����ʱ��: 2016-5-13
���ܣ�
1.��ؼ��AD��ʼ��
2.����ѹ����ã��ṩƬ���¶ȴ���������
------------------------------------
*/

#include "Battery.h"
#include "config.h"
//#include "UART1.h"
//#include "stdio.h"
//#include "CommApp.h"
//#include "ReceiveData.h"

extern ADC_HandleTypeDef hadc1;

//ʵ����һ����ѹ��Ϣ�ṹ��
Bat_Typedef Battery;

uint8_t FLY_ENABLE=0;//����ʹ�ܶ�  7/-5    14/15


void BatteryCheckInit()
{
	
	HAL_ADCEx_Calibration_Start(&hadc1);//ADCУ׼

	Battery.BatReal = 3.95;//��λΪv ���ʵ�ʵ�ѹ  У׼��ѹʱ�޸�
	Battery.ADinput = 1.98;//��λΪv R15��R17���Ӵ���ѹ У׼��ѹʱ�޸�
	Battery.ADRef   = 3.26;//��λΪv ��Ƭ�������ѹ   У׼��ѹʱ�޸�
	Battery.Bat_K   = Battery.BatReal / Battery.ADinput; //�����ѹ����ϵ��
	Battery.BatteryADmin = 2000;//��ѹ����ADֵ
	printf("��ѹ���AD��ʼ�����...\r\n");
}

uint16_t Get_Adc()
{
	uint32_t temp_Adc;
	if (HAL_ADC_Start(&hadc1) != HAL_OK)
	{
		/* Start Conversation Error */
	}
	if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK)
	{
		/* End Of Conversion flag not set on time */
	}
	else
	{
		/* ADC conversion completed */
		/*##-5- Get the converted value of regular channel  ########################*/
		temp_Adc= HAL_ADC_GetValue(&hadc1);
	}
	return temp_Adc;
}

uint16_t Get_Adc_Average(uint8_t times)
{
	uint32_t temp_val = 0;
	uint8_t t;
	for (t = 0; t < times; t++)
	{
		temp_val += Get_Adc();
	}
	return temp_val / times;
}

//���ص�ص�ѹADֵ
int GetBatteryAD()
{
 return Get_Adc_Average(8);
}

//����ص�ѹ
void BatteryCheck(void)
{
	Battery.BatteryAD  = GetBatteryAD();            //��ص�ѹ���
	Battery.BatteryVal = Battery.Bat_K * (Battery.BatteryAD / 4096.0) * Battery.ADRef; //ʵ�ʵ�ѹֵ����
if(FLY_ENABLE)
		{
			if(Battery.BatteryAD <= Battery.BatteryADmin)
			{
					Battery.alarm=1;
			}
			else
					Battery.alarm=0;
		}
		else
		{
			if((Battery.BatteryVal < BAT_ALARM_VAL)&&(Battery.BatteryVal > BAT_CHG_VAL))	//����3.7v �Ҵ��ڳ�����ѹ BAT_CHG_VAL
				Battery.alarm=1;
			else
				Battery.alarm=0;
		}
				if(Battery.BatteryVal < BAT_CHG_VAL) //on charge
		{
			Battery.chargeSta = 1; 
		}
		else 					
			Battery.chargeSta = 0;
}
