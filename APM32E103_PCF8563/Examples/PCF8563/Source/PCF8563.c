#include "main.h"
#include "delay.h"

#define uchar unsigned char
	
uchar TX_buf[10];


#define SDA(x)  GPIO_WriteBitValue(GPIOC,GPIO_PIN_0,x);
#define SCL(x)  GPIO_WriteBitValue(GPIOC,GPIO_PIN_1,x);

#define SDA_IN  GPIO_ReadInputBit(GPIOC,GPIO_PIN_0)

uchar g8563_Store[6]; /*时间交换区,全局变量声明*/
uchar  c8563_Store[6]={0x13,0x09,0x22,0x10,0x40,0x00}; /*写入时间初值：星期一 07:59:00*/


void IIC_GPIO_Init(void)
{
	  GPIO_Config_T gpioConfig;
	  RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_GPIOC);
	
	
		gpioConfig.pin = GPIO_PIN_0|GPIO_PIN_1;				
		gpioConfig.mode=GPIO_MODE_OUT_PP ;
		gpioConfig.speed=GPIO_SPEED_50MHz;
		GPIO_Config(GPIOC, &gpioConfig); 
	  SDA(1);
	  SCL(1);
}
 

void IIC_SDA_OUT(void)
{
	   GPIO_Config_T gpioConfig;
	  RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_GPIOC);
	
	
		gpioConfig.pin = GPIO_PIN_0;				
		gpioConfig.mode=GPIO_MODE_OUT_PP ;
		gpioConfig.speed=GPIO_SPEED_50MHz;
		GPIO_Config(GPIOC, &gpioConfig); 
}


void IIC_SDA_IN(void)
{
	  GPIO_Config_T gpioConfig;
	  RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_GPIOC);
	
	
		gpioConfig.pin = GPIO_PIN_0;				
		gpioConfig.mode=GPIO_MODE_IN_PU;
		gpioConfig.speed=GPIO_SPEED_50MHz;
		GPIO_Config(GPIOC, &gpioConfig); 
}






/********************************************
内部函数，延时1
********************************************/
#define  Delay()   delay_us(2)

/********************************************
内部函数，I2C开始
********************************************/
void Start()
{ 
	  	IIC_SDA_OUT();
	    SDA(1);
   		SCL(1);
   		Delay();
   		SDA(0);
   		Delay();
   		SCL(0);
}
/********************************************
内部函数，I2C结束
********************************************/
void Stop()
{
      IIC_SDA_OUT();
	    SDA(0);
   		SCL(0);
	    Delay();
   		SCL(1);
   		Delay();
   		SDA(1);
   		Delay();
}
/********************************************
内部函数，输出ACK ,每个字节传输完成，输出ack=0,结束读书据，ack=1;
********************************************/
void WriteACK(uchar ack)
{
		  IIC_SDA_OUT();  
	    SDA(ack);
   		Delay();
   		SCL(1);
   		Delay();
   		SCL(0);
}
/********************************************
内部函数，等待ACK
********************************************/
void WaitACK()
{  
		  uchar errtime=20;
	    IIC_SDA_OUT();  
   		SDA(1);
   		Delay(); /*读ACK*/
   		SCL(1);
	    IIC_SDA_IN();  
   		Delay();
   		while(SDA_IN)
   		{  
				errtime--;
      			if(!errtime) 
						Stop();
   		}
   		SCL(0);
   		Delay();
}
/********************************************
内部函数.输出数据字节
入口:B=数据
********************************************/
void writebyte(uchar wdata)
{
		  uchar i;
	    IIC_SDA_OUT();  
   		for(i=0;i<8;i++)
   		{
        		if(wdata&0x80) 
						{
						  SDA(1);
						}
       		  else 
						{
						  SDA(0);
						}
       			wdata<<=1;
       			SCL(1);
       			Delay();
       			SCL(0);
   		}
   		WaitACK();     //I2C器件或通讯出错，将会退出I2C通讯
}
/********************************************
内部函数.输入数据
出口:B
********************************************/
uchar Readbyte()
{
			uchar i,bytedata;
			IIC_SDA_OUT(); 
			SDA(1);
			IIC_SDA_IN(); 
   		for(i=0;i<8;i++)
   		{
      			SCL(1); 
      			bytedata<<=1;
      			bytedata|=SDA_IN;
      			SCL(0);
      			Delay();
   		}
   		return(bytedata);
}
/********************************************
输出数据->pcf8563
********************************************/
void writeData(uchar address,uchar mdata)
{
		Start();
   	writebyte(0xa2); /*写命令*/
		writebyte(address); /*写地址*/
	  writebyte(mdata); /*写数据*/
		Stop();
}
/********************************************
输入数据<-pcf8563
********************************************/
/*uchar ReadData(uchar address) //单字节
{  
		  uchar rdata;
   		Start();
	    writebyte(0xa2); //写命令
	    writebyte(address); //写地址
	    Start();
	    writebyte(0xa3); //读命令
	    rdata=Readbyte();
	    WriteACK(1);
	    Stop();
	    return(rdata);
}	*/
void ReadData1(uchar address,uchar count,uchar * buff) /*多字节*/
{  
		  uchar i;
   		Start();
   		writebyte(0xa2); /*写命令*/
	    writebyte(address); /*写地址*/
	    Start();
	    writebyte(0xa3); /*读命令*/
	    for(i=0;i<count;i++)
	    {
	    		buff[i]=Readbyte();
	       		if(i<count-1) 
						WriteACK(0);
	    }
	    WriteACK(1);
      Stop();
}  
/********************************************
内部函数,读入时间到内部缓冲区
********************************************/
void P8563_Read()
{   
		  uchar time[7];
    	ReadData1(0x02,0x07,time);
	    g8563_Store[0]=time[0]&0x7f; /*秒 */
	    g8563_Store[1]=time[1]&0x7f; /*分 */
	    g8563_Store[2]=time[2]&0x3f; /*小时 */
		  g8563_Store[3]=time[3]&0x3f; /*日 */
	    g8563_Store[4]=time[5]&0x1f; /*月 */
		  g8563_Store[5]=time[6]; /*年  */

	    TX_buf[0] = g8563_Store[5];
		  TX_buf[1] = g8563_Store[4];
		  TX_buf[2] = g8563_Store[3];
	    TX_buf[3] = g8563_Store[2];	
		  TX_buf[4] = g8563_Store[1];	
		  TX_buf[5] = g8563_Store[0];	 	
}
/********************************************
读入时间到内部缓冲区----外部调用 
********************************************/
void P8563_gettime(void)
{
    	P8563_Read();
    	if(g8563_Store[0]==0)
      P8563_Read(); /*如果为秒=0，为防止时间变化，再读一次*/
}	
/********************************************
写时间修改值
********************************************/
void P8563_settime()
{
	    //uchar i;
	    writeData(8,g8563_Store[0]); //年 
	 	  writeData(7,g8563_Store[1]); //月 
		  writeData(5,g8563_Store[2]); //日 
		  writeData(4,g8563_Store[3]); //时 
	   	writeData(3,g8563_Store[4]); //分  
		  writeData(2,g8563_Store[5]); //秒 
}
/********************************************
P8563的初始化-----外部调用
********************************************/
void P8563_init(void)
{
    uchar i;
	  // P8563_settime();
    for(i=0;i<=5;i++) g8563_Store[i]=c8563_Store[i]; /*初始化时间*/
        P8563_settime();  
   // if((ReadData(0x0a)&0x3f)!=0x08) /*检查是否第一次启动，是则初始化时间*/
   // {
//	    P3_4 = 0;
 //       for(i=0;i<=3;i++) g8563_Store[i]=c8563_Store[i]; /*初始化时间*/
 //       P8563_settime();
 //       writeData(0x0,0x00);
 //       writeData(0xa,0x8); /*8:00报警*/
 //       writeData(0x1,0x12); /*报警有效*/
  //      writeData(0xd,0xf0);  //编程输出32.768K的频率
  //  }
}

