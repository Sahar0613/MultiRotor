/******************** (C) COPYRIGHT 2014 ANO Tech ********************************
  * ����   �������ƴ�
 * �ļ���  ��data_transfer.c
 * ����    �����ݴ���
 * ����    ��www.anotc.com
 * �Ա�    ��anotc.taobao.com
 * ����QȺ ��190169595
**********************************************************************************/

//#include "stm32f1xx_hal.h"
#include "ANO_DT.h"
#include "config.h"

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;


typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} Coord_Typedef;

typedef struct
{
	Coord_Typedef Acc;
	Coord_Typedef Gyro;
	Coord_Typedef Mag_Adc;
	uint8_t Acc_CALIBRATE;
	uint8_t Gyro_CALIBRATE;

} IMU_Typedef;

typedef struct
{
	float P, pout, I, iout, D, dout, IMAX, OUT;
} PID;

IMU_Typedef mpu6050;
// mpu6050.Acc = Acc;
// mpu6050.Gyro = Gyro;
// mpu6050.Mag_Adc = Mag_Adc;
// Acc.x = 1; Acc.y = 2; Acc.z = 3;
// Gyro.x = 4; Gyro.x = 5; Gyro.x = 6;
// Mag_Adc.x = 7; Mag_Adc.y = 8; Mag_Adc.z = 9;
// mpu6050.Acc_CALIBRATE = 1;
// mpu6050.Gyro_CALIBRATE = 0;

int16_t Roll = 10, Pitch = 22, Yaw = 33;
int32_t baroAlt = 14;
uint8_t fly_ready = 0;
uint16_t Rc_Pwm_In[8] = {777, 0, 777, 0, 777, 0, 777, 0};

//���ݲ�ֺ궨�壬�ڷ��ʹ���1�ֽڵ���������ʱ������int16��float�ȣ���Ҫ�����ݲ�ֳɵ����ֽڽ��з���
#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)	  ) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )

dt_flag_t f;						//��Ҫ�������ݵı�־
u8 data_to_send[50];				//�������ݻ���
extern UART_HandleTypeDef huart1;	//��Ҫ���͵Ĵ��ں�

/****************************************************************************
*	��������: ANO_DT_Data_Exchange()
*	��    ��: �˺�������������ݷ������󣬱�����ʵ��ÿ5ms����һ�δ�������������λ�������ڴ˺�����ʵ��
*	��ڲ���:
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��: 2016-5-14
*	�޸�ʱ��:
*	˵    ��: �˺���Ӧ���û�ÿ1ms����һ��
****************************************************************************/
void ANO_DT_Data_Exchange(void)
{
	static u8 cnt = 0;
	static u8 senser_cnt 	= 10;
	static u8 status_cnt 	= 15;
	static u8 rcdata_cnt 	= 20;
	static u8 motopwm_cnt	= 20;
	static u8 power_cnt		=	50;

	if ((cnt % senser_cnt) == (senser_cnt - 1))
		f.send_senser = 1;

	if ((cnt % status_cnt) == (status_cnt - 1))
		f.send_status = 1;

	if ((cnt % rcdata_cnt) == (rcdata_cnt - 1))
		f.send_rcdata = 1;

	if ((cnt % motopwm_cnt) == (motopwm_cnt - 1))
		f.send_motopwm = 1;

	if ((cnt % power_cnt) == (power_cnt - 1))
		f.send_power = 1;

	cnt++;

	//�汾��Ϣ
	if (f.send_version)
	{
		f.send_version = 0;
		ANO_DT_Send_Version(4, 300, 100, 400, 0);
	}

	//�ɻ���̬�Ȼ�����Ϣ
	else if (f.send_status)
	{
		f.send_status = 0;
		ANO_DT_Send_Status(Roll, Pitch, Yaw, baroAlt, 0, fly_ready);
	}

	//�ɻ�����������
	else if (f.send_senser)
	{
		f.send_senser = 0;
		ANO_DT_Send_Senser(mpu6050.Acc.x, mpu6050.Acc.y, mpu6050.Acc.z,
		                   mpu6050.Gyro.x, mpu6050.Gyro.y, mpu6050.Gyro.z,
		                   mpu6050.Mag_Adc.x, mpu6050.Mag_Adc.y, mpu6050.Mag_Adc.z, 0);
	}

	//�ɻ��յ��Ŀ�������
	else if (f.send_rcdata)
	{
		f.send_rcdata = 0;
		ANO_DT_Send_RCData(Rc_Pwm_In[0], Rc_Pwm_In[1], Rc_Pwm_In[2], Rc_Pwm_In[3], Rc_Pwm_In[4], Rc_Pwm_In[5], Rc_Pwm_In[6], Rc_Pwm_In[7], 0, 0);
	}

	//���PWM(��Χ0- 1000)
	else if (f.send_motopwm)
	{
		f.send_motopwm = 0;
		ANO_DT_Send_MotoPWM(100, 200, 300, 400, 500, 600, 700, 800);
	}

	//��ص�ѹ������
	else if (f.send_power)
	{
		f.send_power = 0;
		ANO_DT_Send_Power(Battery.BatteryVal * 100, 100);
	}

	//PID����֡1
	else if (f.send_pid1)
	{
		f.send_pid1 = 0;
		//ANO_DT_Send_PID(1, ctrl_1.PID[PIDROLL].kp, ctrl_1.PID[PIDROLL].ki, ctrl_1.PID[PIDROLL].kd,
		// ctrl_1.PID[PIDPITCH].kp, ctrl_1.PID[PIDPITCH].ki, ctrl_1.PID[PIDPITCH].kd,
		//            ctrl_1.PID[PIDYAW].kp, ctrl_1.PID[PIDYAW].ki, ctrl_1.PID[PIDYAW].kd);
	}

	//PID����֡2
	else if (f.send_pid2)
	{
		f.send_pid2 = 0;
		//ANO_DT_Send_PID(2, ctrl_1.PID[PID4].kp, ctrl_1.PID[PID4].ki, ctrl_1.PID[PID4].kd,
		// ctrl_1.PID[PID5].kp, ctrl_1.PID[PID5].ki, ctrl_1.PID[PID5].kd,
		//            ctrl_1.PID[PID6].kp, ctrl_1.PID[PID6].ki, ctrl_1.PID[PID6].kd);
	}

	//PID����֡3
	else if (f.send_pid3)
	{
		f.send_pid3 = 0;
		//ANO_DT_Send_PID(3, ctrl_2.PID[PIDROLL].kp, ctrl_2.PID[PIDROLL].ki, ctrl_2.PID[PIDROLL].kd,
		// ctrl_2.PID[PIDPITCH].kp, ctrl_2.PID[PIDPITCH].ki, ctrl_2.PID[PIDPITCH].kd,
		//            ctrl_2.PID[PIDYAW].kp, ctrl_2.PID[PIDYAW].ki, ctrl_2.PID[PIDYAW].kd);
	}


}

/****************************************************************************
*	��������: ANO_DT_Send_Data(u8 *dataToSend , u8 length)
*	��    ��: �˺�����Э�������з������ݹ���ʹ�õ��ķ��ͺ���
*	��ڲ���: dataToSend,length
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: ��ֲʱ���û�Ӧ��������Ӧ�õ����������ʹ�õ�ͨ�ŷ�ʽ��ʵ�ִ˺���
****************************************************************************/
void ANO_DT_Send_Data(u8 *dataToSend , u8 length)
{
	HAL_UART_Transmit(&huart1, data_to_send, length, 10);
}

/****************************************************************************
*	��������: ANO_DT_Send_Check(u8 head, u8 check_sum)
*	��    ��: �˺����ṩ�������ݵ�У��
*	��ڲ���: u8 head, u8 check_sum
*	��    ��:
*	��    ��: ANO_DT_Send_Data(data_to_send, _cnt)
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��:
****************************************************************************/
static void ANO_DT_Send_Check(u8 head, u8 check_sum)
{
	data_to_send[0] = 0xAA;
	data_to_send[1] = 0xAA;
	data_to_send[2] = 0xEF;
	data_to_send[3] = 2;
	data_to_send[4] = head;
	data_to_send[5] = check_sum;


	u8 sum = 0;
	for (u8 i = 0; i < 6; i++)
		sum += data_to_send[i];
	data_to_send[6] = sum;

	ANO_DT_Send_Data(data_to_send, 7);
}


/****************************************************************************
*	��������: void ANO_DT_Data_Receive_Prepare(u8 data)
*	��    ��:�˺�����Э��Ԥ����������Э��ĸ�ʽ�����յ������ݽ���һ�θ�ʽ�Խ�������ʽ��ȷ�Ļ��ٽ������ݽ������������������ϸ�ʽ������֡�󣬻����е������ݽ�������
*	��ڲ���: data
*	��    ��:
*	��    ��: ANO_DT_Data_Receive_Anl(u8 *data_buf,u8 num)
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��:�˺������������ϸ�ʽ������֡�󣬻����е������ݽ�������
****************************************************************************/
void ANO_DT_Data_Receive_Prepare(u8 data)
{
	static u8 RxBuffer[50];
	static u8 _data_len = 0, _data_cnt = 0;
	static u8 state = 0;

	if (state == 0 && data == 0xAA)
	{
		state = 1;
		RxBuffer[0] = data;
	}
	else if (state == 1 && data == 0xAF)
	{
		state = 2;
		RxBuffer[1] = data;
	}
	else if (state == 2 && data < 0XF1)
	{
		state = 3;
		RxBuffer[2] = data;
	}
	else if (state == 3 && data < 50)
	{
		state = 4;
		RxBuffer[3] = data;
		_data_len = data;
		_data_cnt = 0;
	}
	else if (state == 4 && _data_len > 0)
	{
		_data_len--;
		RxBuffer[4 + _data_cnt++] = data;
		if (_data_len == 0)
			state = 5;
	}
	else if (state == 5)
	{
		state = 0;
		RxBuffer[4 + _data_cnt] = data;
		ANO_DT_Data_Receive_Anl(RxBuffer, _data_cnt + 5);
	}
	else
		state = 0;
}


/****************************************************************************
*	��������: ANO_DT_Data_Receive_Anl(u8 *data_buf,u8 num)
*	��    ��: �˺�����Э�����ݽ������������������Ƿ���Э���ʽ��һ������֡���ú��������ȶ�Э�����ݽ���У�飬У��ͨ��������ݽ��н�����ʵ����Ӧ����
*	��ڲ���: data_buf,num
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: �˺������Բ����û����е��ã��ɺ���Data_Receive_Prepare�Զ�����
****************************************************************************/
void ANO_DT_Data_Receive_Anl(u8 *data_buf, u8 num)
{
	u8 sum = 0;
	for (u8 i = 0; i < (num - 1); i++)
		sum += *(data_buf + i);
	if (!(sum == *(data_buf + num - 1)))		return;		//�ж�sum
	if (!(*(data_buf) == 0xAA && *(data_buf + 1) == 0xAF))		return;		//�ж�֡ͷ

	if (*(data_buf + 2) == 0X01)
	{
		if (*(data_buf + 4) == 0X01)
			mpu6050.Acc_CALIBRATE = 1;
		if (*(data_buf + 4) == 0X02)
			mpu6050.Gyro_CALIBRATE = 1;
		if (*(data_buf + 4) == 0X03)
		{
			mpu6050.Acc_CALIBRATE = 1;
			mpu6050.Gyro_CALIBRATE = 1;
		}
	}

	if (*(data_buf + 2) == 0X02)
	{
		if (*(data_buf + 4) == 0X01)
		{
			f.send_pid1 = 1;
			f.send_pid2 = 1;
			f.send_pid3 = 1;
			f.send_pid4 = 1;
			f.send_pid5 = 1;
			f.send_pid6 = 1;
		}
		if (*(data_buf + 4) == 0X02)
		{

		}
		if (*(data_buf + 4) == 0XA0)		//��ȡ�汾��Ϣ
		{
			f.send_version = 1;
		}
		if (*(data_buf + 4) == 0XA1)		//�ָ�Ĭ�ϲ���
		{
			//	Para_ResetToFactorySetup();
		}
	}

	if (*(data_buf + 2) == 0X10)								//PID1
	{
//		ctrl_1.PID[PIDROLL].kp  = 0.001 * ( (vs16)(*(data_buf + 4) << 8) | *(data_buf + 5) );
//		ctrl_1.PID[PIDROLL].ki  = 0.001 * ( (vs16)(*(data_buf + 6) << 8) | *(data_buf + 7) );
//		ctrl_1.PID[PIDROLL].kd  = 0.001 * ( (vs16)(*(data_buf + 8) << 8) | *(data_buf + 9) );
//		ctrl_1.PID[PIDPITCH].kp = 0.001 * ( (vs16)(*(data_buf + 10) << 8) | *(data_buf + 11) );
//		ctrl_1.PID[PIDPITCH].ki = 0.001 * ( (vs16)(*(data_buf + 12) << 8) | *(data_buf + 13) );
//		ctrl_1.PID[PIDPITCH].kd = 0.001 * ( (vs16)(*(data_buf + 14) << 8) | *(data_buf + 15) );
//		ctrl_1.PID[PIDYAW].kp 	= 0.001 * ( (vs16)(*(data_buf + 16) << 8) | *(data_buf + 17) );
//		ctrl_1.PID[PIDYAW].ki 	= 0.001 * ( (vs16)(*(data_buf + 18) << 8) | *(data_buf + 19) );
//		ctrl_1.PID[PIDYAW].kd 	= 0.001 * ( (vs16)(*(data_buf + 20) << 8) | *(data_buf + 21) );
		ANO_DT_Send_Check(*(data_buf + 2), sum);
//		Param_SavePID();
	}
	if (*(data_buf + 2) == 0X11)								//PID2
	{
//		ctrl_1.PID[PID4].kp 	= 0.001 * ( (vs16)(*(data_buf + 4) << 8) | *(data_buf + 5) );
//		ctrl_1.PID[PID4].ki 	= 0.001 * ( (vs16)(*(data_buf + 6) << 8) | *(data_buf + 7) );
//		ctrl_1.PID[PID4].kd 	= 0.001 * ( (vs16)(*(data_buf + 8) << 8) | *(data_buf + 9) );
//		ctrl_1.PID[PID5].kp 	= 0.001 * ( (vs16)(*(data_buf + 10) << 8) | *(data_buf + 11) );
//		ctrl_1.PID[PID5].ki 	= 0.001 * ( (vs16)(*(data_buf + 12) << 8) | *(data_buf + 13) );
//		ctrl_1.PID[PID5].kd 	= 0.001 * ( (vs16)(*(data_buf + 14) << 8) | *(data_buf + 15) );
//		ctrl_1.PID[PID6].kp	  = 0.001 * ( (vs16)(*(data_buf + 16) << 8) | *(data_buf + 17) );
//		ctrl_1.PID[PID6].ki 	= 0.001 * ( (vs16)(*(data_buf + 18) << 8) | *(data_buf + 19) );
//		ctrl_1.PID[PID6].kd 	= 0.001 * ( (vs16)(*(data_buf + 20) << 8) | *(data_buf + 21) );
		ANO_DT_Send_Check(*(data_buf + 2), sum);
//		Param_SavePID();
	}
	if (*(data_buf + 2) == 0X12)								//PID3
	{
//		ctrl_2.PID[PIDROLL].kp  = 0.001 * ( (vs16)(*(data_buf + 4) << 8) | *(data_buf + 5) );
//		ctrl_2.PID[PIDROLL].ki  = 0.001 * ( (vs16)(*(data_buf + 6) << 8) | *(data_buf + 7) );
//		ctrl_2.PID[PIDROLL].kd  = 0.001 * ( (vs16)(*(data_buf + 8) << 8) | *(data_buf + 9) );
//		ctrl_2.PID[PIDPITCH].kp = 0.001 * ( (vs16)(*(data_buf + 10) << 8) | *(data_buf + 11) );
//		ctrl_2.PID[PIDPITCH].ki = 0.001 * ( (vs16)(*(data_buf + 12) << 8) | *(data_buf + 13) );
//		ctrl_2.PID[PIDPITCH].kd = 0.001 * ( (vs16)(*(data_buf + 14) << 8) | *(data_buf + 15) );
//		ctrl_2.PID[PIDYAW].kp 	= 0.001 * ( (vs16)(*(data_buf + 16) << 8) | *(data_buf + 17) );
//		ctrl_2.PID[PIDYAW].ki 	= 0.001 * ( (vs16)(*(data_buf + 18) << 8) | *(data_buf + 19) );
//		ctrl_2.PID[PIDYAW].kd 	= 0.001 * ( (vs16)(*(data_buf + 20) << 8) | *(data_buf + 21) );
		ANO_DT_Send_Check(*(data_buf + 2), sum);
//		Param_SavePID();
	}
	if (*(data_buf + 2) == 0X13)								//PID4
	{
		ANO_DT_Send_Check(*(data_buf + 2), sum);
	}
	if (*(data_buf + 2) == 0X14)								//PID5
	{
		ANO_DT_Send_Check(*(data_buf + 2), sum);
	}
	if (*(data_buf + 2) == 0X15)								//PID6
	{
		ANO_DT_Send_Check(*(data_buf + 2), sum);
	}
}


/****************************************************************************
*	��������: ANO_DT_Send_Version(u8 hardware_type, u16 hardware_ver, u16 software_ver, u16 protocol_ver, u16 bootloader_ver)
*	��    ��: ���Ͱ汾��Ϣ
*	��ڲ���: hardware_type,hardware_ver,software_ver,protocol_ver,bootloader_ver
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: HardwareTypeӲ�����ࡢHardwareVER*100Ӳ���汾��SoftwareVER*100����汾��ProtocolVER*100Э��汾��BootloaderVER*100
****************************************************************************/
void ANO_DT_Send_Version(u8 hardware_type, u16 hardware_ver, u16 software_ver, u16 protocol_ver, u16 bootloader_ver)
{
	u8 _cnt = 0;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x00;
	data_to_send[_cnt++] = 0;

	data_to_send[_cnt++] = hardware_type;
	data_to_send[_cnt++] = BYTE1(hardware_ver);
	data_to_send[_cnt++] = BYTE0(hardware_ver);
	data_to_send[_cnt++] = BYTE1(software_ver);
	data_to_send[_cnt++] = BYTE0(software_ver);
	data_to_send[_cnt++] = BYTE1(protocol_ver);
	data_to_send[_cnt++] = BYTE0(protocol_ver);
	data_to_send[_cnt++] = BYTE1(bootloader_ver);
	data_to_send[_cnt++] = BYTE0(bootloader_ver);

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;

	ANO_DT_Send_Data(data_to_send, _cnt);
}


/****************************************************************************
*	��������: ANO_DT_Send_Status(float angle_rol, float angle_pit, float angle_yaw, s32 alt, u8 fly_model, u8 armed)
*	��    ��: ���ͷɻ���̬�Ȼ�����Ϣ
*	��ڲ���: angle_rol,  angle_pit,  angle_yaw,  alt,  fly_model,  armed
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: int16 ROL*100��int16 PIT*100��int16 YAW*100��int32 ALT_USE(�߶�cm)��uint8 FLY_MODEL(����ģʽ����uint8 ARMED : 0���� 1����"
****************************************************************************/
void ANO_DT_Send_Status(float angle_rol, float angle_pit, float angle_yaw, s32 alt, u8 fly_model, u8 armed)
{
	u8 _cnt = 0;
	vs16 _temp;
	vs32 _temp2 = alt;

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x01;
	data_to_send[_cnt++] = 0;

	_temp = (int)(angle_rol * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = (int)(angle_pit * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = (int)(angle_yaw * 100);
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	data_to_send[_cnt++] = BYTE3(_temp2);
	data_to_send[_cnt++] = BYTE2(_temp2);
	data_to_send[_cnt++] = BYTE1(_temp2);
	data_to_send[_cnt++] = BYTE0(_temp2);

	data_to_send[_cnt++] = fly_model;

	data_to_send[_cnt++] = armed;

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;

	ANO_DT_Send_Data(data_to_send, _cnt);
}


/****************************************************************************
*	��������: ANO_DT_Send_Senser(s16 a_x, s16 a_y, s16 a_z, s16 g_x, s16 g_y, s16 g_z, s16 m_x, s16 m_y, s16 m_z, s32 bar)
*	��    ��: �ɻ�����������
*	��ڲ���: a_x,a_y,a_z,g_x,g_y,g_z,m_x,m_y,m_z,bar
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: ���ٶȼ�(ACC)��������(GYRO)��������(MAG)����ѹ��(BAR)
*			  int16 ACC_X��int16 ACC_Y��int16 ACC_Z��int16 GYRO_X��int16 GYRO_Y��int16 GYRO_Z��int16 MAG_X��int16 MAG_Y��int16 MAG_Z��int32 BAR
 ***************************************************************************/
void ANO_DT_Send_Senser(s16 a_x, s16 a_y, s16 a_z, s16 g_x, s16 g_y, s16 g_z, s16 m_x, s16 m_y, s16 m_z, s32 bar)
{
	u8 _cnt = 0;
	vs16 _temp;

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x02;
	data_to_send[_cnt++] = 0;

	_temp = a_x;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = a_y;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = a_z;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = g_x;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = g_y;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = g_z;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	_temp = m_x;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = m_y;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = m_z;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];
	data_to_send[_cnt++] = sum;

	ANO_DT_Send_Data(data_to_send, _cnt);
}

/****************************************************************************
*	��������: ANO_DT_Send_RCData(u16 thr, u16 yaw, u16 rol, u16 pit, u16 aux1, u16 aux2, u16 aux3, u16 aux4, u16 aux5, u16 aux6)
*	��    ��: �ɻ��յ��Ŀ�������
*	��ڲ���: thr,yaw,rol,pit,aux1,aux2,aux3,aux4,aux5,aux6
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: ����(THR)������(ROLL)������(PITCH)������(YAW)������(AUX)
*			  int16 THR��int16 YAW��int16 ROL��int16 PIT��int16 AUX1��int16 AUX2��int16 AUX3��int16 AUX4��int16 AUX5��int16 AUX6
****************************************************************************/
void ANO_DT_Send_RCData(u16 thr, u16 yaw, u16 rol, u16 pit, u16 aux1, u16 aux2, u16 aux3, u16 aux4, u16 aux5, u16 aux6)
{
	u8 _cnt = 0;

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x03;
	data_to_send[_cnt++] = 0;
	data_to_send[_cnt++] = BYTE1(thr);
	data_to_send[_cnt++] = BYTE0(thr);
	data_to_send[_cnt++] = BYTE1(yaw);
	data_to_send[_cnt++] = BYTE0(yaw);
	data_to_send[_cnt++] = BYTE1(rol);
	data_to_send[_cnt++] = BYTE0(rol);
	data_to_send[_cnt++] = BYTE1(pit);
	data_to_send[_cnt++] = BYTE0(pit);
	data_to_send[_cnt++] = BYTE1(aux1);
	data_to_send[_cnt++] = BYTE0(aux1);
	data_to_send[_cnt++] = BYTE1(aux2);
	data_to_send[_cnt++] = BYTE0(aux2);
	data_to_send[_cnt++] = BYTE1(aux3);
	data_to_send[_cnt++] = BYTE0(aux3);
	data_to_send[_cnt++] = BYTE1(aux4);
	data_to_send[_cnt++] = BYTE0(aux4);
	data_to_send[_cnt++] = BYTE1(aux5);
	data_to_send[_cnt++] = BYTE0(aux5);
	data_to_send[_cnt++] = BYTE1(aux6);
	data_to_send[_cnt++] = BYTE0(aux6);

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];

	data_to_send[_cnt++] = sum;

	ANO_DT_Send_Data(data_to_send, _cnt);
}

/****************************************************************************
*	��������: ANO_DT_Send_Power(u16 votage, u16 current)
*	��    ��: ��ص�ѹ����������
*	��ڲ���: votage, current
*	��    ��:
*	��    ��:ANO_DT_Send_Data(data_to_send, _cnt)
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: uint16 Votage*100��uint16 Current*100
****************************************************************************/
void ANO_DT_Send_Power(u16 votage, u16 current)
{
	u8 _cnt = 0;
	u16 temp;

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x05;
	data_to_send[_cnt++] = 0;

	temp = votage;
	data_to_send[_cnt++] = BYTE1(temp);
	data_to_send[_cnt++] = BYTE0(temp);
	temp = current;
	data_to_send[_cnt++] = BYTE1(temp);
	data_to_send[_cnt++] = BYTE0(temp);

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];

	data_to_send[_cnt++] = sum;

	ANO_DT_Send_Data(data_to_send, _cnt);
}


/****************************************************************************
*	��������: ANO_DT_Send_MotoPWM(u16 m_1, u16 m_2, u16 m_3, u16 m_4, u16 m_5, u16 m_6, u16 m_7, u16 m_8)
*	��    ��: ���PWM(��Χ0-1000)
*	��ڲ���: m_1, m_2, m_3, m_4, m_5, m_6, m_7, m_8
*	��    ��:
*	��    ��:
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: uint16 PWM_MOTO1��uint16 PWM_MOTO2
****************************************************************************/
void ANO_DT_Send_MotoPWM(u16 m_1, u16 m_2, u16 m_3, u16 m_4, u16 m_5, u16 m_6, u16 m_7, u16 m_8)
{
	u8 _cnt = 0;

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x06;
	data_to_send[_cnt++] = 0;

	data_to_send[_cnt++] = BYTE1(m_1);
	data_to_send[_cnt++] = BYTE0(m_1);
	data_to_send[_cnt++] = BYTE1(m_2);
	data_to_send[_cnt++] = BYTE0(m_2);
	data_to_send[_cnt++] = BYTE1(m_3);
	data_to_send[_cnt++] = BYTE0(m_3);
	data_to_send[_cnt++] = BYTE1(m_4);
	data_to_send[_cnt++] = BYTE0(m_4);
	data_to_send[_cnt++] = BYTE1(m_5);
	data_to_send[_cnt++] = BYTE0(m_5);
	data_to_send[_cnt++] = BYTE1(m_6);
	data_to_send[_cnt++] = BYTE0(m_6);
	data_to_send[_cnt++] = BYTE1(m_7);
	data_to_send[_cnt++] = BYTE0(m_7);
	data_to_send[_cnt++] = BYTE1(m_8);
	data_to_send[_cnt++] = BYTE0(m_8);

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];

	data_to_send[_cnt++] = sum;

	ANO_DT_Send_Data(data_to_send, _cnt);
}


/****************************************************************************
*	��������: ANO_DT_Send_PID(u8 group, float p1_p, float p1_i, float p1_d, float p2_p, float p2_i, float p2_d, float p3_p, float p3_i, float p3_d)
*	��    ��: PID����֡
*	��ڲ���: group, p1_p, p1_i, p1_d, p2_p, p2_i, p2_d, p3_p, p3_i, p3_d
*	��    ��:
*	��    ��: ANO_DT_Send_Data
*	��    ��: WJ
*	ʱ    ��:
*	�޸�ʱ��:
*	˵    ��: PID����*1000���͸���λ��
****************************************************************************/
void ANO_DT_Send_PID(u8 group, float p1_p, float p1_i, float p1_d, float p2_p, float p2_i, float p2_d, float p3_p, float p3_i, float p3_d)
{
	u8 _cnt = 0;
	vs16 _temp;

	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0x10 + group - 1;
	data_to_send[_cnt++] = 0;


	_temp = p1_p * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p1_i  * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p1_d  * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p2_p  * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p2_i  * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p2_d * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p3_p  * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p3_i  * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);
	_temp = p3_d * 1000;
	data_to_send[_cnt++] = BYTE1(_temp);
	data_to_send[_cnt++] = BYTE0(_temp);

	data_to_send[3] = _cnt - 4;

	u8 sum = 0;
	for (u8 i = 0; i < _cnt; i++)
		sum += data_to_send[i];

	data_to_send[_cnt++] = sum;

	ANO_DT_Send_Data(data_to_send, _cnt);
}
