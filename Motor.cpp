#include "Motor.h"  
#include "cnComm.h"
#include <windows.h>
#include <math.h>
cnComm Cusart(true);
char cState = 0;
int iMax = 2147483647;
int iMin = -2147483648;
#define DELAY 100
/*---------------内部使用函数------------------*/
/*
*数据校验函数
*input：校验数据地址，校验数据长度
*output:校验结果
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
* int32 转char
* pos 转出的char在哪个位置开始写入
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
*发送数据
*/
int UsartSendData(char *szBuffer, int len)
{
	int iRtn = 0;
	DWORD dwBufferLength = len;
	iRtn = Cusart.Write(szBuffer, dwBufferLength);

	return iRtn;
}
/*
*读取数据
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
*初始化函数
*input：校验数据地址，校验数据长度
*output:if = 1 成功 else 失败
*/
int UsartSendInit(void)
{
	int iRtn = 0;

	return iRtn;
}

/*=============================================*/

/*
*建立连接
*输入，iCom：端口号，范围[1,1024];iBaud:波特率
*返回值：1表示成功，0表示失败
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
		SndBuf[1] = 0xc0;//功能码
		SndBuf[2] = 0x04;//个数
		SndBuf[3] = check(SndBuf,3);

		iRtn = UsartSendData(SndBuf,4);
		if (iRtn == 0)
		{
			//发送成功，等待消息返回
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
*断开连接
*返回值：1表示成功，0表示失败
*/
int MotorDisConneted(void)
{
	int iRtn = 1;

	Cusart.Close();
	cState = 0;

	return iRtn;
}

/*<a0>
*电机速度模式运动
*输入：（1）cMotorDone=1,立即执行，0和其它电机联动,范围[0,1]
	   （2）cMotorId，电机标号，范围[0,3]
	   （3）iSpeed，速度值，范围±[100,2000] [-100,2000]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
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
	SndBuf[1] = 0xa0;//功能码
	SndBuf[2] = 0x0a;//个数
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
		//发送成功，等待消息返回
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
*电机位置模式运动-绝对位置运动
*输入：（1）cMotorDone=1,立即执行，0和其它电机联动,范围[0,1]
	   （2）cMotorId，电机标号，范围[0,3]
	   （3）iSpeed，速度值，范围[100,2000]
	   （4）iPos，目标位置，范围[-2147483648,2147483647]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
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
	SndBuf[1] = 0xa1;//功能码
	SndBuf[2] = 0x0e;//个数
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
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 

/*<a2>
*电机位置模式运动-相对位置运动
*输入：（1）cMotorDone=1,立即执行，0和其它电机联动,范围[0,1]
	   （2）cMotorId，电机标号，范围[0,3]
	   （3）iSpeed，速度值，范围[100,2000]
	   （4）iPos，目标位置，范围[-2147483648,2147483647]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
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
	SndBuf[1] = 0xa2;//功能码
	SndBuf[2] = 0x0e;//个数
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
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}  

/*<a3>
*电机速度使能运动 
*输入：（1）cMotorEn，char型，bit0-3,分别代表电机0-3,1表示使能，0表示不使能
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：必须先下发参数，才能执行此指令
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
	SndBuf[1] = 0xa3;//功能码
	SndBuf[2] = 0x05;//个数
	if (cMotorEn >= 0 && cMotorEn <= 0x0f)
		SndBuf[3] = cMotorEn;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a4>
*电机急停
*输入：（1）cMotorEn，char型，bit0-3,分别代表电机0-3,1表示使能，0表示不使能
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：必须先下发参数，才能执行此指令
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
	SndBuf[1] = 0xa4;//功能码
	SndBuf[2] = 0x5;//个数
	if (cMotorStop >= 0 && cMotorStop <= 0x0f)
		SndBuf[3] = cMotorStop;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a5>
*云台运动至相应的角度
*输入：（1）iPitch,俯仰，绕x轴旋转，范围[-900,900]代表-90,90，精度0.1
	   （2）yaw,航向，绕y轴旋转，范围[-1800,1800]代表-180,180，精度0.1
	   （3）roll，横滚，绕z轴旋转，范围[-1800,1800]代表-180,180，精度0.1
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
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
	SndBuf[1] = 0xa5;//功能码
	SndBuf[2] = 0x10;//个数

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
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}
/*<a6>
*电机位置使能运动 
*输入：（1）cMotorEn，char型，bit0-3,分别代表电机0-3,1表示使能，0表示不使能
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：必须先下发参数，才能执行此指令
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
	SndBuf[1] = 0xa6;//功能码
	SndBuf[2] = 0x05;//个数
	if (cMotorEn >= 0 && cMotorEn <= 0x0f)
		SndBuf[3] = cMotorEn;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a7>
*记录当前位置
*输入：位置编号，[0-9]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
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
	SndBuf[1] = 0xa7;//功能码
	SndBuf[2] = 0x05;//个数
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<a8>
*记录当前角度
*输入：角度编号，[0-9]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
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
	SndBuf[1] = 0xa8;//功能码
	SndBuf[2] = 0x05;//个数
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}
/*<a9>
*电机回零
*输入：电机编号，[0-3]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
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
	SndBuf[1] = 0xa9;//功能码
	SndBuf[2] = 0x05;//个数
	if (cMotorId >= 0 && cMotorId <= 0x9)
		SndBuf[3] = cMotorId;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
}
/*<aa>
*设置当前位置为最大值或者最小值
*输入：0:最大值，1:最小值，2:恢复初始最大最小值 min:-2147483648  max:2147483647
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
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
	SndBuf[1] = 0xaa;//功能码
	SndBuf[2] = 0x05;//个数
	if (cNumber >= 0 && cNumber <= 0x2)
		SndBuf[3] = cNumber;
	else
		return -1;

	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
		Sleep(DELAY);
		UsartRecData(RecBuf,25);
		iRtn=0;
		if ((check(RecBuf,4)==RecBuf[4])&&(RecBuf[1]==SndBuf[1])&&(RecBuf[2]==1))
			iRtn=1;
	}
	return iRtn;
} 
/*<b0>
*获取电机当前位置
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
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
	SndBuf[1] = 0xb0;//功能码
	SndBuf[2] = 0x4;//个数

	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	*piPos0 = 0;
	*piPos1 = 0;
	*piPos2 = 0;
	*piPos3 = 0;
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
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
*获取云台角度
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：云台角度，返回结果除以10为真实值
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
	SndBuf[1] = 0xb1;//功能码
	SndBuf[2] = 0x4;//个数
	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
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
*获取存储的电机位置
*输入：位置编号0-9
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
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
	SndBuf[1] = 0xb2;//功能码
	SndBuf[2] = 0x5;//个数
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;
	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
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
*获取存储的云台角度
*输入：位置编号0-9
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：云台角度，返回结果除以10为真实值
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
	SndBuf[1] = 0xb3;//功能码
	SndBuf[2] = 0x5;//个数
	if (cNumber >= 0 && cNumber <= 0x9)
		SndBuf[3] = cNumber;
	else
		return -1;
	SndBuf[4] = check(SndBuf,4);
	iRtn = UsartSendData(SndBuf,5);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
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
*获取电机状态
*输入：无
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个字节，每个字节对应一个电机，0停止，1运动，2故障
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
	SndBuf[1] = 0xb4;//功能码
	SndBuf[2] = 0x4;//个数
	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
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
*获取电机位置最大值
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
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
	SndBuf[1] = 0xb5;//功能码
	SndBuf[2] = 0x4;//个数

	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	*piPos0 = 0;
	*piPos1 = 0;
	*piPos2 = 0;
	*piPos3 = 0;
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
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
*获取电机最小位置
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
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
	SndBuf[1] = 0xb6;//功能码
	SndBuf[2] = 0x4;//个数

	SndBuf[3] = check(SndBuf,3);
	iRtn = UsartSendData(SndBuf,4);
	*piPos0 = 0;
	*piPos1 = 0;
	*piPos2 = 0;
	*piPos3 = 0;
	if (iRtn == 0)
	{
		//发送成功，等待消息返回
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