//#ifndef _DLL_API  
//#define _DLL_API _declspec(dllexport)  
//#else  
//#define _DLL_API _declspec(dllimport)  
//#endif  
      

/*
*建立连接
*输入，iCom：端口号，范围[1,1024];iBaud:波特率
*返回值：1表示成功，0表示失败
*/
 int MotorConneted(int iCom, int iBaud);

/*
*断开连接
*返回值：1表示成功，0表示失败
*/
 int MotorDisConneted(void);

/*
*电机速度模式运动
*输入：（1）cMotorDone=1,立即执行，0和其它电机联动,范围[0,1]
	   （2）cMotorId，电机标号，范围[0,3]
	   （3）iSpeed，速度值，范围[100,2000]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*/
 int MotorSpeedSet(char cMotorDone, char cMotorId, int iSpeed);

/*
*电机位置模式运动-绝对位置运动
*输入：（1）cMotorDone=1,立即执行，0和其它电机联动,范围[0,1]
	   （2）cMotorId，电机标号，范围[0,3]
	   （3）iSpeed，速度值，范围[100,2000]
	   （4）iPos，目标位置，范围[-2147483648,2147483647]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*/
 int MotorPosSet(char cMotorDone, char cMotorId, int iSpeed, int iPos);

/*
*电机位置模式运动-相对位置运动
*输入：（1）cMotorDone=1,立即执行，0和其它电机联动,范围[0,1]
	   （2）cMotorId，电机标号，范围[0,3]
	   （3）iSpeed，速度值，范围[100,2000]
	   （4）iPos，目标位置，范围[-2147483648,2147483647]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*/
 int MotorAddPosSet(char cMotorDone, char cMotorId, int iSpeed, int iPos);

/*
*电机速度使能运动 
*输入：（1）cMotorEn，char型，bit0-3,分别代表电机0-3,1表示使能，0表示不使能
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：必须先下发参数，才能执行此指令
*/
 int MotorSpeedEn(char cMotorEn);

/*
*电机急停
*输入：（1）cMotorEn，char型，bit0-3,分别代表电机0-3,1表示使能，0表示不使能
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：必须先下发参数，才能执行此指令
*/
 int MotorStop(char cMotorStop);

/*
*电机位置模式运动-绝对位置运动
*输入：（1）iPitch,俯仰，绕x轴旋转，范围[-900,900]代表-90,90，精度0.1
	   （2）yaw,航向，绕y轴旋转，范围[-1800,1800]代表-180,180，精度0.1
	   （3）roll，横滚，绕z轴旋转，范围[-1800,1800]代表-180,180，精度0.1
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*/
 int MotorMPUSet(int iPitch,int iYaw,int iRoll);

/*
*电机位置使能运动 
*输入：（1）cMotorEn，char型，bit0-3,分别代表电机0-3,1表示使能，0表示不使能
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：必须先下发参数，才能执行此指令
*/
 int MotorPosEn(char cMotorEn);
 /*
*记录当前位置
*输入：位置编号，[0-9]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
*/
int MotorRecordPos(char cNumber);
 /*
*记录当前角度
*输入：角度编号，[0-9]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
*/
int MotorRecordAng(char cNumber);
 /*
*电机回零
*输入：电机编号，[0-3]
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
*/
int MotorOrig(char cMotorId);
/*
*设置当前位置为最大值或者最小值
*输入：0:最大值，1:最小值，2:恢复初始最大最小值 min:-2147483648  max:2147483647
*输出：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*前置条件：无
*/
int SetMotorLimitPos(char cNumber);
/*
*获取电机当前位置
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
*/
 int GetMotorPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3);

/*
*获取云台角度
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：云台角度，返回结果除以10为真实值
*/
 int GetMotorMPU(int *piPitch,int *piRoll,int *piYaw);
 /*
*获取存储的电机位置
*输入：位置编号0-9
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
*/
int GetMotorRecordPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3,char cNumber);
/*
*获取存储的云台角度
*输入：位置编号0-9
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：云台角度，返回结果除以10为真实值
*/
int GetMotorRecordMPU(int *piPitch,int *piRoll,int *piYaw,char cNumber);
/*<b4>
*获取电机状态
*输入：无
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个字节，每个字节对应一个电机，0停止，1运动，2故障
*/
int GetMotorState(char cMotorState[4]);
/*
*获取电机位置最大值
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
*/
int GetMotorMaxPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3);
/*
*获取电机最小位置
*返回值：1表示接收成功并开始执行，0表示失败,-1参数错误，-2未初始化
*输出：四个电机的位置信息
*/
int GetMotorMinPos(int *piPos0,int *piPos1,int *piPos2,int *piPos3);