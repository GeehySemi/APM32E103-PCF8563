#include "main.h"
#include "delay.h"

#define uchar unsigned char
	
uchar TX_buf[10];


#define SDA(x)  GPIO_WriteBitValue(GPIOC,GPIO_PIN_0,x);
#define SCL(x)  GPIO_WriteBitValue(GPIOC,GPIO_PIN_1,x);

#define SDA_IN  GPIO_ReadInputBit(GPIOC,GPIO_PIN_0)

uchar g8563_Store[6]; /*ʱ�佻����,ȫ�ֱ�������*/
uchar  c8563_Store[6]={0x13,0x09,0x22,0x10,0x40,0x00}; /*д��ʱ���ֵ������һ 07:59:00*/


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
�ڲ���������ʱ1
********************************************/
#define  Delay()   delay_us(2)

/********************************************
�ڲ�������I2C��ʼ
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
�ڲ�������I2C����
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
�ڲ����������ACK ,ÿ���ֽڴ�����ɣ����ack=0,��������ݣ�ack=1;
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
�ڲ��������ȴ�ACK
********************************************/
void WaitACK()
{  
		  uchar errtime=20;
	    IIC_SDA_OUT();  
   		SDA(1);
   		Delay(); /*��ACK*/
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
�ڲ�����.��������ֽ�
���:B=����
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
   		WaitACK();     //I2C������ͨѶ���������˳�I2CͨѶ
}
/********************************************
�ڲ�����.��������
����:B
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
�������->pcf8563
********************************************/
void writeData(uchar address,uchar mdata)
{
		Start();
   	writebyte(0xa2); /*д����*/
		writebyte(address); /*д��ַ*/
	  writebyte(mdata); /*д����*/
		Stop();
}
/********************************************
��������<-pcf8563
********************************************/
/*uchar ReadData(uchar address) //���ֽ�
{  
		  uchar rdata;
   		Start();
	    writebyte(0xa2); //д����
	    writebyte(address); //д��ַ
	    Start();
	    writebyte(0xa3); //������
	    rdata=Readbyte();
	    WriteACK(1);
	    Stop();
	    return(rdata);
}	*/
void ReadData1(uchar address,uchar count,uchar * buff) /*���ֽ�*/
{  
		  uchar i;
   		Start();
   		writebyte(0xa2); /*д����*/
	    writebyte(address); /*д��ַ*/
	    Start();
	    writebyte(0xa3); /*������*/
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
�ڲ�����,����ʱ�䵽�ڲ�������
********************************************/
void P8563_Read()
{   
		  uchar time[7];
    	ReadData1(0x02,0x07,time);
	    g8563_Store[0]=time[0]&0x7f; /*�� */
	    g8563_Store[1]=time[1]&0x7f; /*�� */
	    g8563_Store[2]=time[2]&0x3f; /*Сʱ */
		  g8563_Store[3]=time[3]&0x3f; /*�� */
	    g8563_Store[4]=time[5]&0x1f; /*�� */
		  g8563_Store[5]=time[6]; /*��  */

	    TX_buf[0] = g8563_Store[5];
		  TX_buf[1] = g8563_Store[4];
		  TX_buf[2] = g8563_Store[3];
	    TX_buf[3] = g8563_Store[2];	
		  TX_buf[4] = g8563_Store[1];	
		  TX_buf[5] = g8563_Store[0];	 	
}
/********************************************
����ʱ�䵽�ڲ�������----�ⲿ���� 
********************************************/
void P8563_gettime(void)
{
    	P8563_Read();
    	if(g8563_Store[0]==0)
      P8563_Read(); /*���Ϊ��=0��Ϊ��ֹʱ��仯���ٶ�һ��*/
}	
/********************************************
дʱ���޸�ֵ
********************************************/
void P8563_settime()
{
	    //uchar i;
	    writeData(8,g8563_Store[0]); //�� 
	 	  writeData(7,g8563_Store[1]); //�� 
		  writeData(5,g8563_Store[2]); //�� 
		  writeData(4,g8563_Store[3]); //ʱ 
	   	writeData(3,g8563_Store[4]); //��  
		  writeData(2,g8563_Store[5]); //�� 
}
/********************************************
P8563�ĳ�ʼ��-----�ⲿ����
********************************************/
void P8563_init(void)
{
    uchar i;
	  // P8563_settime();
    for(i=0;i<=5;i++) g8563_Store[i]=c8563_Store[i]; /*��ʼ��ʱ��*/
        P8563_settime();  
   // if((ReadData(0x0a)&0x3f)!=0x08) /*����Ƿ��һ�������������ʼ��ʱ��*/
   // {
//	    P3_4 = 0;
 //       for(i=0;i<=3;i++) g8563_Store[i]=c8563_Store[i]; /*��ʼ��ʱ��*/
 //       P8563_settime();
 //       writeData(0x0,0x00);
 //       writeData(0xa,0x8); /*8:00����*/
 //       writeData(0x1,0x12); /*������Ч*/
  //      writeData(0xd,0xf0);  //������32.768K��Ƶ��
  //  }
}

