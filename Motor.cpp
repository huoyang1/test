#include "Motor.h"  
#include "cnComm.h"
#include <windows.h>
#include <math.h>
cnComm Cusart(true);
char cState = 0;
int iMax = 2147483647;
int iMin = -2147483648;
#define DELAY 100
/*---------------�ڲ�ʹ�ú���------------------*/
/*
*����У�麯��
*input��У�����ݵ�ַ��У�����ݳ���
*output:У����
*/
int check(char *iBufData, int len)
{
	char check = 0;
	int i = 0;

	for(i = 0; i < len; i++)
	{
		check = check + *(iBufData + i);
	}

	return check;
}
/*
* int32 תchar
* pos ת����char���ĸ�λ�ÿ�ʼд��
*/
void convInt4char(int iData, int pos, char *iBufData)
{
	iBufData[pos+0] = (iData&0xff000000)>>24;
	iBufData[pos+1] = (iData&0x00ff0000)>>16;
	iBufData[pos+2] = (iData&0x0000ff00)>>8;
	iBufData[pos+3] = (iData&0x000000ff);
}
int t = 0;
int ConvChar4Int(char pos,char *Rec_buf)
{
	int ibuf=0;
	char buf[4];
	buf[0] = Rec_buf[pos]&0xff;
	buf[1] = Rec_buf[pos+1]&0xff;
	buf[2] = Rec_buf[pos+2]&0xff;
	buf[3] = Rec_buf[pos+3]&0xff;
	t = ((buf[0]<<24)&0xff000000);
	t = t|((buf[1]<<16)&0x00ff0000);
	t = t|((buf[2]<<8)&0x0000ff00);
	t = t|(buf[3]&0x000000ff);
	return t;
}
/*
*��������
*/
int UsartSendData(char *szBuffer, int len)
{
	int iRtn = 0;
	DWORD dwBufferLength = len;
	iRtn = Cusart.Write(szBuffer, dwBufferLength);

	return iRtn;
}
/*
*��ȡ����
*/
int UsartRecData(char *szBuffer, int len)
{
	int iRtn = 0;
	int i=0;
	DWORD dwBufferLength = len;
	iRtn = Cusart.Read(szBuffer, dwBufferLength);
	

	return iRtn;
}
/*
*��ʼ������
*input��У�����ݵ�ַ��У�����ݳ���
*output:if = 1 �ɹ� else ʧ��
*/
int UsartSendInit(void)
{
	int iRtn = 0;

	return iRtn;
}

/*=============================================*/

/*
*��������
*���룬iCom���˿ںţ���Χ[1,1024];iBaud:������
*����ֵ��1��ʾ�ɹ���0��ʾʧ��
*/

int MotorConneted(int iCom, int iBaud)
{
	int iRtn = 0;
	DWORD dwPort = iCom;
	DWORD dwBaudRate = iBaud;
	char SndBuf[100]={0};
	char RecBuf[30]={0};

	iRtn = Cusart.Open(dwPort, dwBaudRate);

	if (iRtn == true)
	{
		cState = 0;
		SndBuf[0] = 0x01;
		SndBuf[1] = 0xc0;//������
		SndBuf[2] = 0x04;//����
		SndBuf[3] = check(SndBuf,3);

		iRtn = UsartSendData(SndBuf,4);
		if (iRtn == 0)
		{
			//���ͳɹ����ȴ���Ϣ����
			Sleep(DELAY);
			UsartRecData(RecBuf,25);

			printf("[%x][%x][%x][%x][%x][%x][%x]\r\n",RecBuf[0]&0xff,RecBuf[1]&0xff,RecBuf[2]&0xff,RecBuf[3]&0xff,RecBuf[4]&0xff,RecBuf[5]&0xff,RecBuf[6]&0xff);

			if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			{
				iRtn=1;
				cState = 1;
			}
		}
		
	}
	else
	{
		iRtn = 0;
		cState = 0;
	}

	return iRtn;
}

/*
*�Ͽ�����
*����ֵ��1��ʾ�ɹ���0��ʾʧ��
*/
int MotorDisConneted(void)
{
	int iRtn = 1;

	Cusart.Close();
	cState = 0;

	return iRtn;
}

/*<a0>
*����ٶ�ģʽ�˶�
*���룺��1��cMotorDone=1,����ִ�У�0�������������,��Χ[0,1]
	   ��2��cMotorId�������ţ���Χ[0,3]
	   ��3��iSpeed���ٶ�ֵ����Χ��[100,2000] [-100,2000]
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*/
int MotorSpeedSet(char cMotorDone, char cMotorId, int iSpeed)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	int i=0;
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa0;//������
	SndBuf[2] = 0x0a;//����
	if (cMotorDone >= 0 && cMotorDone <= 1)
		SndBuf[3] = cMotorDone;
	else
		return -1;
	if (cMotorId >= 0 && cMotorId <= 3)
		SndBuf[4] = cMotorId;
	else
		return -1;

    if (iSpeed >= 100 && (iSpeed) <= 2000)
		convInt4char(iSpeed,5,SndBuf);//5-8
	else if (iSpeed <= -100 && (iSpeed) >= -2000)
		convInt4char(iSpeed,5,SndBuf);//5-8
	else
		return -1;

	SndBuf[9] = check(SndBuf,9);

	iRtn = UsartSendData(SndBuf,10);
	
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);

		//printf("[%d][%d][%d][%d][%d]\r\n",RecBuf[0],RecBuf[1],RecBuf[2],RecBuf[3],RecBuf[4]);
		printf("[%x][%x][%x][%x][%x][%x][%x]\r\n",RecBuf[0]&0xff,RecBuf[1]&0xff,RecBuf[2]&0xff,RecBuf[3]&0xff,RecBuf[4]&0xff,RecBuf[5]&0xff,RecBuf[6]&0xff);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}

/*<a1>
*���λ��ģʽ�˶�-����λ���˶�
*���룺��1��cMotorDone=1,����ִ�У�0�������������,��Χ[0,1]
	   ��2��cMotorId�������ţ���Χ[0,3]
	   ��3��iSpeed���ٶ�ֵ����Χ[100,2000]
	   ��4��iPos��Ŀ��λ�ã���Χ[-2147483648,2147483647]
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*/
int MotorPosSet(char cMotorDone, char cMotorId, int iSpeed, int iPos)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa1;//������
	SndBuf[2] = 0x0e;//����
	if (cMotorDone >= 0 && cMotorDone <= 1)
		SndBuf[3] = cMotorDone;
	else
		return -1;
	if (cMotorId >= 0 && cMotorId <= 3)
		SndBuf[4] = cMotorId;
	else
		return -1;
	if (iSpeed >= 100 && iSpeed <= 2000)
		convInt4char(iSpeed,5,SndBuf);//5-8
	else
		return -1;
	if (iSpeed >= iMin && iSpeed <= iMax)
		convInt4char(iPos,9,SndBuf);//9-12
	else
		return -1;

	SndBuf[13] = check(SndBuf,13);
	iRtn = UsartSendData(SndBuf,14);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 

/*<a2>
*���λ��ģʽ�˶�-���λ���˶�
*���룺��1��cMotorDone=1,����ִ�У�0�������������,��Χ[0,1]
	   ��2��cMotorId�������ţ���Χ[0,3]
	   ��3��iSpeed���ٶ�ֵ����Χ[100,2000]
	   ��4��iPos��Ŀ��λ�ã���Χ[-2147483648,2147483647]
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*/
int MotorAddPosSet(char cMotorDone, char cMotorId, int iSpeed, int iPos)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa2;//������
	SndBuf[2] = 0x0e;//����
	if (cMotorDone >= 0 && cMotorDone <= 1)
		SndBuf[3] = cMotorDone;
	else
		return -1;
	if (cMotorId >= 0 && cMotorId <= 3)
		SndBuf[4] = cMotorId;
	else
		return -1;
	if (iSpeed >= 100 && iSpeed <= 2000)
		convInt4char(iSpeed,5,SndBuf);//5-8
	else
		return -1;
	if (iSpeed >= iMin && iSpeed <= iMax)
		convInt4char(iPos,9,SndBuf);//9-12
	else
		return -1;

	SndBuf[13] = check(SndBuf,13);
	iRtn = UsartSendData(SndBuf,14);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}  

/*<a3>
*����ٶ�ʹ���˶� 
*���룺��1��cMotorEn��char�ͣ�bit0-3,�ֱ������0-3,1��ʾʹ�ܣ�0��ʾ��ʹ��
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*ǰ���������������·�����������ִ�д�ָ��
*/
int MotorSpeedEn(char cMotorEn)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa3;//������
	SndBuf[2] = 0x05;//����
	if (cMotorEn >= 0 && cMotorEn <= 0x0f)
		SndBuf[3] = cMotorEn;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a4>
*�����ͣ
*���룺��1��cMotorEn��char�ͣ�bit0-3,�ֱ������0-3,1��ʾʹ�ܣ�0��ʾ��ʹ��
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*ǰ���������������·�����������ִ�д�ָ��
*/
int MotorStop(char cMotorStop)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa4;//������
	SndBuf[2] = 0x5;//����
	if (cMotorStop >= 0 && cMotorStop <= 0x0f)
		SndBuf[3] = cMotorStop;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a5>
*��̨�˶�����Ӧ�ĽǶ�
*���룺��1��iPitch,��������x����ת����Χ[-900,900]����-90,90������0.1
	   ��2��yaw,������y����ת����Χ[-1800,1800]����-180,180������0.1
	   ��3��roll���������z����ת����Χ[-1800,1800]����-180,180������0.1
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*/
int MotorMPUSet(int iPitch,int iYaw,int iRoll)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa5;//������
	SndBuf[2] = 0x10;//����

	if (iPitch >= -900 && iPitch <= 900)
		convInt4char(iPitch,3,SndBuf);//3-6
	else
		return -1;
	if (iYaw >= -1800 && iYaw <= 1800)
		convInt4char(iYaw,7,SndBuf);//7-10
	else
		return -1;
	if (iRoll >= -1800 && iRoll <= 1800)
		convInt4char(iRoll,11,SndBuf);//11-14
	else
		return -1;

	SndBuf[15] = check(SndBuf,15);
	iRtn = UsartSendData(SndBuf,16);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}
/*<a6>
*���λ��ʹ���˶� 
*���룺��1��cMotorEn��char�ͣ�bit0-3,�ֱ������0-3,1��ʾʹ�ܣ�0��ʾ��ʹ��
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*ǰ���������������·�����������ִ�д�ָ��
*/
int MotorPosEn(char cMotorEn)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa6;//������
	SndBuf[2] = 0x05;//����
	if (cMotorEn >= 0 && cMotorEn <= 0x0f)
		SndBuf[3] = cMotorEn;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a7>
*��¼��ǰλ��
*���룺λ�ñ�ţ�[0-9]
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*ǰ����������
*/
int MotorRecordPos(char cNumber)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa7;//������
	SndBuf[2] = 0x05;//����
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a8>
*��¼��ǰ�Ƕ�
*���룺�Ƕȱ�ţ�[0-9]
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*ǰ����������
*/
int MotorRecordAng(char cNumber)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa8;//������
	SndBuf[2] = 0x05;//����
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}
/*<a9>
*�������
*���룺�����ţ�[0-3]
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*ǰ����������
*/
int MotorOrig(char cMotorId)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xa9;//������
	SndBuf[2] = 0x05;//����
	if (cMotorId >= 0 && cMotorId <= 0x9)
		SndBuf[3] = cMotorId;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}
/*<aa>
*���õ�ǰλ��Ϊ���ֵ������Сֵ
*���룺0:���ֵ��1:��Сֵ��2:�ָ���ʼ�����Сֵ min:-2147483648  max:2147483647
*�����1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*ǰ����������
*/
int SetMotorLimitPos(char cNumber)
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xaa;//������
	SndBuf[2] = 0x05;//����
	if (cNumber >= 0 && cNumber <= 0x2)
		SndBuf[3] = cNumber;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<b0>
*��ȡ�����ǰλ��
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*������ĸ������λ����Ϣ
*/
int GetMotorPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3)
{
	int iRtn = 0;
	int i = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[50]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xb0;//������
	SndBuf[2] = 0x4;//����

	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	*piPos0 = 0;
	*piPos1 = 0;
	*piPos2 = 0;
	*piPos3 = 0;
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY*2);
		UsartRecData(RecBuf,50);
		//printf("----------len=%d------------\r\n",strlen(RecBuf));
		//for(i = 0; i < 20; i++)
		//		printf("[%x]-",RecBuf[i]&0xff);
		//	printf("\r\n");
		iRtn=0;
		//printf("-------[%x]=?[%x]-----\r\n",0xff&check(RecBuf,19),0xff&RecBuf[19]);
		//printf("-------[%x]=?[%x]-----\r\n",0xff&RecBuf[1],0xff&SndBuf[1]);
		//printf("-------[%x]=?[%x]-----\r\n",20,0xff&RecBuf[2]);
		if ((check(RecBuf,19)==RecBuf[19])&&(RecBuf[1]==SndBuf[1])&&20 == RecBuf[2])
		{
			*piPos0 = ConvChar4Int(3,RecBuf);
			*piPos1 = ConvChar4Int(7,RecBuf);
			*piPos2 = ConvChar4Int(11,RecBuf);
			*piPos3 = ConvChar4Int(15,RecBuf);
			iRtn=1;
		}
		else
		{
			iRtn=0;
			iRtn = UsartSendData(SndBuf,4);
			if (iRtn == 0)
			{
				Sleep(DELAY*2);
				UsartRecData(RecBuf,50);
				//printf("-------[%x]=?[%x]-----\r\n",0xff&check(RecBuf,19),0xff&RecBuf[19]);
				//printf("-------[%x]=?[%x]-----\r\n",0xff&RecBuf[1],0xff&SndBuf[1]);
				//printf("-------[%x]=?[%x]-----\r\n",20,0xff&RecBuf[2]);
			    if ((check(RecBuf,19)==RecBuf[19])&&(RecBuf[1]==SndBuf[1])&&20 == RecBuf[2])
				{
					*piPos0 = ConvChar4Int(3,RecBuf);
					*piPos1 = ConvChar4Int(7,RecBuf);
					*piPos2 = ConvChar4Int(11,RecBuf);
					*piPos3 = ConvChar4Int(15,RecBuf);
					iRtn=1;
				}
			}
		}
		
	}
	return iRtn;
}
/*<b1>
*��ȡ��̨�Ƕ�
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*�������̨�Ƕȣ����ؽ������10Ϊ��ʵֵ
*/
int GetMotorMPU(int *piPitch,int *piRoll,int *piYaw)
{
	int iRtn = 0;
	int i = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[50]={0};
	if (cState != 1)
		return -2;
	short a =0,b = 0;
	float af = 0;
	float bf = 0;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xb1;//������
	SndBuf[2] = 0x4;//����
	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY*2);
		UsartRecData(RecBuf,50);
		iRtn=0;
		//for(i = 0; i < 8; i++)
		//{
		//	printf("[%x]",RecBuf[i]&0xff);
		//}
		//printf("\r\n");
		if ((check(RecBuf,7)==RecBuf[7])&&(RecBuf[1]==SndBuf[1]))
		{
			a = ((0xff&RecBuf[3])<<8)+0xff&RecBuf[4];
			b = ((0xff&RecBuf[5])<<8)+0xff&RecBuf[6];
			//af = (a-900)/10.0f;
			//bf = (b-900)/10.0f;
			*piPitch = a - 900;
			*piRoll = b - 900;
			//printf("a=%d--b=%d--\r\n",*piPitch,*piRoll);
			//printf("af=%lf--bf=%lf--\r\n",af,bf);
			*piYaw = 0;
			if((*piPitch >= -900 && *piPitch <= 900) && (*piRoll >= -900 && *piRoll <= 900))
			{
				iRtn=1;
			}
			else
			{
				iRtn=0;
				*piPitch = 0;
				*piRoll = 0;
				*piYaw = 0;
			}
		}
	}
	return iRtn;
}
/*<b2>
*��ȡ�洢�ĵ��λ��
*���룺λ�ñ��0-9
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*������ĸ������λ����Ϣ
*/
int GetMotorRecordPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3,char cNumber)
{
	int iRtn = 0;
	int i = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[50]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xb2;//������
	SndBuf[2] = 0x5;//����
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;
	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,50);
		//printf("----------len=%d------------\r\n",strlen(RecBuf));
		//for(i = 0; i < 20; i++)
		//		printf("[%x]-",RecBuf[i]&0xff);
		//	printf("\r\n");
		iRtn=0;
		//printf("-------[%x]=?[%x]-----\r\n",0xff&check(RecBuf,19),0xff&RecBuf[19]);
		if ((check(RecBuf,19)==RecBuf[19])&&(RecBuf[1]==SndBuf[1])&&20 == RecBuf[2])
		{
			*piPos0 = ConvChar4Int(3,RecBuf);
			*piPos1 = ConvChar4Int(7,RecBuf);
			*piPos2 = ConvChar4Int(11,RecBuf);
			*piPos3 = ConvChar4Int(15,RecBuf);
			iRtn=1;
		}
		
	}
	return iRtn;
}
/*<b3>
*��ȡ�洢����̨�Ƕ�
*���룺λ�ñ��0-9
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*�������̨�Ƕȣ����ؽ������10Ϊ��ʵֵ
*/
int GetMotorRecordMPU(int *piPitch,int *piRoll,int *piYaw,char cNumber)
{
	int iRtn = 0;
	int i = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[50]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xb3;//������
	SndBuf[2] = 0x5;//����
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;
	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,50);
		iRtn=0;
		if ((check(RecBuf,15)==RecBuf[15])&&(RecBuf[1]==SndBuf[1]))
		{
			*piPitch = ConvChar4Int(3,RecBuf) - 900;
			*piRoll = ConvChar4Int(7,RecBuf) - 900;
			*piYaw = 0;
			iRtn=1;
		}
	}
	return iRtn;
}
/*<b4>
*��ȡ���״̬
*���룺��
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*������ĸ��ֽڣ�ÿ���ֽڶ�Ӧһ�������0ֹͣ��1�˶���2����
*/
int GetMotorState(char cMotorState[4])
{
	int iRtn = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[30]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xb4;//������
	SndBuf[2] = 0x4;//����
	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,7)==RecBuf[7])&&(RecBuf[1]==SndBuf[1]))
		{
			cMotorState[0] = RecBuf[3];
			cMotorState[1] = RecBuf[4];
			cMotorState[2] = RecBuf[5];
			cMotorState[3] = RecBuf[6];
			iRtn=1;
		}
	}
	return iRtn;
}
/*<b5>
*��ȡ���λ�����ֵ
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*������ĸ������λ����Ϣ
*/
int GetMotorMaxPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3)
{
	int iRtn = 0;
	int i = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[50]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xb5;//������
	SndBuf[2] = 0x4;//����

	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	*piPos0 = 0;
	*piPos1 = 0;
	*piPos2 = 0;
	*piPos3 = 0;
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,50);
		//printf("----------len=%d------------\r\n",strlen(RecBuf));
		//for(i = 0; i < 20; i++)
		//		printf("[%x]-",RecBuf[i]&0xff);
		//	printf("\r\n");
		iRtn=0;
		//printf("-------[%x]=?[%x]-----\r\n",0xff&check(RecBuf,19),0xff&RecBuf[19]);
		//printf("-------[%x]=?[%x]-----\r\n",0xff&RecBuf[1],0xff&SndBuf[1]);
		//printf("-------[%x]=?[%x]-----\r\n",20,0xff&RecBuf[2]);
		if ((check(RecBuf,19)==RecBuf[19])&&(RecBuf[1]==SndBuf[1])&&20 == RecBuf[2])
		{
			*piPos0 = ConvChar4Int(3,RecBuf);
			*piPos1 = ConvChar4Int(7,RecBuf);
			*piPos2 = ConvChar4Int(11,RecBuf);
			*piPos3 = ConvChar4Int(15,RecBuf);
			iRtn=1;
		}
		else
		{
			iRtn=0;
			iRtn = UsartSendData(SndBuf,4);
			if (iRtn == 0)
			{
				Sleep(35);
				UsartRecData(RecBuf,50);
				//printf("-------[%x]=?[%x]-----\r\n",0xff&check(RecBuf,19),0xff&RecBuf[19]);
				//printf("-------[%x]=?[%x]-----\r\n",0xff&RecBuf[1],0xff&SndBuf[1]);
				//printf("-------[%x]=?[%x]-----\r\n",20,0xff&RecBuf[2]);
			    if ((check(RecBuf,19)==RecBuf[19])&&(RecBuf[1]==SndBuf[1])&&20 == RecBuf[2])
				{
					*piPos0 = ConvChar4Int(3,RecBuf);
					*piPos1 = ConvChar4Int(7,RecBuf);
					*piPos2 = ConvChar4Int(11,RecBuf);
					*piPos3 = ConvChar4Int(15,RecBuf);
					iRtn=1;
				}
			}
		}
		
	}
	return iRtn;
}
/*<b6>
*��ȡ�����Сλ��
*����ֵ��1��ʾ���ճɹ�����ʼִ�У�0��ʾʧ��,-1��������-2δ��ʼ��
*������ĸ������λ����Ϣ
*/
int GetMotorMinPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3)
{
	int iRtn = 0;
	int i = 0;
	char cCheck=0;
	char SndBuf[100]={0};
	char RecBuf[50]={0};
	if (cState != 1)
		return -2;
	SndBuf[0] = 0x01;
	SndBuf[1] = 0xb6;//������
	SndBuf[2] = 0x4;//����

	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	*piPos0 = 0;
	*piPos1 = 0;
	*piPos2 = 0;
	*piPos3 = 0;
	if (iRtn == 0)
	{
		//���ͳɹ����ȴ���Ϣ����
		Sleep(DELAY);
		UsartRecData(RecBuf,50);
		//printf("----------len=%d------------\r\n",strlen(RecBuf));
		//for(i = 0; i < 20; i++)
		//		printf("[%x]-",RecBuf[i]&0xff);
		//	printf("\r\n");
		iRtn=0;
		//printf("-------[%x]=?[%x]-----\r\n",0xff&check(RecBuf,19),0xff&RecBuf[19]);
		//printf("-------[%x]=?[%x]-----\r\n",0xff&RecBuf[1],0xff&SndBuf[1]);
		//printf("-------[%x]=?[%x]-----\r\n",20,0xff&RecBuf[2]);
		if ((check(RecBuf,19)==RecBuf[19])&&(RecBuf[1]==SndBuf[1])&&20 == RecBuf[2])
		{
			*piPos0 = ConvChar4Int(3,RecBuf);
			*piPos1 = ConvChar4Int(7,RecBuf);
			*piPos2 = ConvChar4Int(11,RecBuf);
			*piPos3 = ConvChar4Int(15,RecBuf);
			iRtn=1;
		}
		else
		{
			iRtn=0;
			iRtn = UsartSendData(SndBuf,4);
			if (iRtn == 0)
			{
				Sleep(35);
				UsartRecData(RecBuf,50);
				//printf("-------[%x]=?[%x]-----\r\n",0xff&check(RecBuf,19),0xff&RecBuf[19]);
				//printf("-------[%x]=?[%x]-----\r\n",0xff&RecBuf[1],0xff&SndBuf[1]);
				//printf("-------[%x]=?[%x]-----\r\n",20,0xff&RecBuf[2]);
			    if ((check(RecBuf,19)==RecBuf[19])&&(RecBuf[1]==SndBuf[1])&&20 == RecBuf[2])
				{
					*piPos0 = ConvChar4Int(3,RecBuf);
					*piPos1 = ConvChar4Int(7,RecBuf);
					*piPos2 = ConvChar4Int(11,RecBuf);
					*piPos3 = ConvChar4Int(15,RecBuf);
					iRtn=1;
				}
			}
		}
		
	}
	return iRtn;
}