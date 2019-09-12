/************************************************************
程序说明：
首先要确定模块已经注册到网络
然后正确的硬件连接   P3.0-----STXD或者5VT   P3.1-----SRXD或者5VR
GND---GND（只要保证公地即可，没必要单独接一次）
然后确认你单片机上的晶振，根据晶振修改自己的程序。
推荐先将单片机与电脑相连，确定单片机发送的数据是正确的。如果发送的是乱码，请检查晶振与单片机的串口波特率。
如果通过以上几条还解决不了问题，请看群共享文件 AN0004 。

*************************************************************/
#include <REG51.H>
#include <string.H>
#define uchar unsigned char
#define uint unsigned int
//以下是板子上LED的配置，把Px_x改成自己对应的脚。LED 只是用来提示，非必须。
//以下是你的51单片机的晶振大小

sbit P10 = P1 ^ 0;
sbit P11 = P1 ^ 1;
sbit P12 = P1 ^ 2;
sbit P13 = P1 ^ 3;
sbit P14 = P1 ^ 4;
sbit P15 = P1 ^ 5;

#define FOSC_110592M
//#define FOSC_12M
//以下是开机后拨打的手机号，改成自己想要打的号码即可。
uchar num[] = "ATD10086;\r\n";
//以下是GSM模块返回数据
uchar rec_data[50];
uchar rec_num;
//注意，无论接收到信号还是发送完信号，都会进中断服务程序的
/*初始化程序（必须使用，否则无法收发），次程序将会使用定时器1*/
void SerialInti()  //初始化程序（必须使用，否则无法收发）
{
  TMOD = 0x20;  //定时器1操作模式2:8位自动重载定时器

#ifdef FOSC_12M  //在这里根据晶振大小设置不同的数值初始化串口
  TH1 = 0xf3;    //装入初值，波特率2400
  TL1 = 0xf3;
#else
  TH1 = 0xfd;  //装入初值，波特率9600
  TL1 = 0xfd;
#endif  // end of SOC_12M

  TR1 = 1;  //打开定时器
  SM0 =
      0;  //设置串行通讯工作模式，（10为一部发送，波特率可变，由定时器1的溢出率控制）
  SM1 = 1;  //(同上)在此模式下，定时器溢出一次就发送一个位的数据
  REN = 1;  //串行接收允许位（要先设置sm0sm1再开串行允许）
  EA = 1;   //开总中断
  ES = 1;   //开串行口中断
}

/*串行通讯中断，收发完成将进入该中断*/  //如：+CMTI:"SM",2
void Serial_interrupt() interrupt 4 {
  uchar temp;
  temp = SBUF;
  rec_data[rec_num++] = temp;
  if (rec_num >= 50)
    rec_num = 0;
  else
    ;
  RI = 0;  //接收中断信号清零，表示将继续接收
}

//串行口连续发送char型数组，遇到终止号/0将停止
void Uart1Sends(uchar *str) {
  while (*str != '\0') {
    SBUF = *str;
    while (!TI)
      ;  //等待发送完成信号（TI=1）出现
    TI = 0;
    str++;
  }
}
void Uart1BYTE(uchar temp) {
  SBUF = temp;
  while (!TI)
    ;  //等待发送完成信号（TI=1）出现
  TI = 0;
}

uchar hand(uchar *ptr) {
  if (strstr(rec_data, ptr) != NULL)
    return 1;
  else
    return 0;
}

void clear_rec_data() {
  uchar i;
  for (i = 0; i < strlen(rec_data); i++) {
    rec_data[i] = '0';
  }
  rec_num = 0;
}
//延时函数大概是1s钟，不过延时大的话不准...
void DelaySec(int sec) {
  uint i, j = 0;

  for (i = 0; i < sec; i++) {
    for (j = 0; j < 65535; j++) {
    }
  }
}

void main() {
  uchar i = 0;
  SerialInti();
  Uart1Sends(
      "AT\r\n");  //同步波特率，如果将模块配置固定波特率，此条指令就不需要发了
  while (!hand("OK")) {
    clear_rec_data();
    i++;
    Uart1Sends("AT\r\n");  //
    DelaySec(1);           //延时
    if (i >= 5) {
      break;
      // return;
    } else
      ;
  }
  Uart1Sends(
      "AT\r\n");  //同步波特率，如果将模块配置固定波特率，此条指令就不需要发了
  while (!hand("OK")) {
    clear_rec_data();
    i++;
    Uart1Sends("AT\r\n");  //
    DelaySec(1);           //延时
    if (i >= 5) {
      break;
      // return;
    } else
      ;
  }
  Uart1Sends("ATI\r\n");  //握手
  while (!hand("OK")) {
    clear_rec_data();
    i++;
    Uart1Sends("ATI\r\n");  //
    DelaySec(1);            //延时
    if (i >= 5) {
      break;
      // return;
    } else
      ;
  }
  P10 = 0;
  clear_rec_data();  //删除存储的GSM模块返回的数据，以便于以后继续判断
  DelaySec(1);               //延时
  Uart1Sends("AT+CSQ\r\n");  //信号质量
  DelaySec(1);               //延时
  i = 0;
  while (!hand("OK"))  //检测此条指令GSM模块是否执行OK
  {
    clear_rec_data();
    i++;
    Uart1Sends("AT+CSQ\r\n");  //
    DelaySec(1);               //延时
    if (i >= 5) {
      break;
      // return;
    }
  }
  P11 = 0;
  clear_rec_data();
  DelaySec(1);                 //延时
  Uart1Sends("AT+CPIN?\r\n");  //查看是否读到手机卡
  DelaySec(1);                 //延时
  i = 0;
  while (!hand("READY"))  //检测SIM模块是否收到SIM卡READY
  {
    clear_rec_data();
    i++;
    Uart1Sends("AT+CPIN?\r\n");  //是否注册到网络
    DelaySec(1);                 //延时
    if (i >= 5) {
      break;
      // return;
    } else
      ;
  }
  P12 = 0;
  Uart1Sends("AT+COPS?\r\n");  //是否注册到网络
  DelaySec(1);                 //延时
  i = 0;
  while (!hand("CHINA"))  //检测是否收到模块返回的CHINA
  {
    clear_rec_data();
    i++;
    Uart1Sends("AT+COPS?\r\n");  //是否注册到网络
    DelaySec(1);                 //延时
    if (i >= 5) {
      break;
      // return;
    } else
      ;
  }
  P13 = 0;
  clear_rec_data();
  DelaySec(1);                          //延时
                                        //以下是发送英文短信短信
  Uart1Sends("AT+CSCS=\"GSM\"\r\n");    //
  DelaySec(1);                          //延时
  Uart1Sends("AT+CSCA?\r\n");           //短信中心号码
  DelaySec(1);                          //延时
  Uart1Sends("AT+CMGF=1\r\n");          //方式1
  DelaySec(1);                          //延时
  Uart1Sends("AT+CMGS=\"10086\"\r\n");  //此处修改短信接收方电话号
  DelaySec(1);                          //延时
  Uart1Sends("ye");                     //此处修改短信内容
  DelaySec(1);                          //延时
  Uart1BYTE(0X1A);
  DelaySec(1);  //延时
  DelaySec(1);  //延时
  DelaySec(1);  //延时
  DelaySec(1);  //延时
  P14 = 0;
  //发送英文短信结束
  Uart1Sends("AT+CMGD=1,4\r\n");  //删除SIM卡中的内容
  DelaySec(1);                    //延时
  //以下是拨打10086代码
  Uart1Sends(num);  //拨打10086
  DelaySec(1);      //延时
  DelaySec(1);      //延时
  DelaySec(1);      //延时
  DelaySec(1);      //延时
  //拨打电话代码结束
  //以下内容没有实际测试，仅供参考
  while (1) {
    if (hand("CMTI")) {
      P13 = 0;
      clear_rec_data();
      DelaySec(1);                  //延时
      Uart1Sends("AT+CMGR=1\r\n");  //
      DelaySec(1);                  //延时
      i = 0;
      while (!hand("OK")) {
        clear_rec_data();
        i++;
        Uart1Sends("AT+CMGR=1,4\r\n");  //
        DelaySec(1);                    //延时
        if (i >= 2)                     // chucuo
        {
          break;
          // return;
        } else
          ;
      }
      clear_rec_data();
      DelaySec(1);  //延时
      if (hand("relay on")) {
        P14 = 0;
        // break;
      } else if (hand("relay off")) {
        P15 = 0;
        // break;
      } else {
        // break;
      }
      Uart1Sends("AT+CMGD=1,4\r\n");  //
      DelaySec(1);                    //延时

    } else if (hand("ring"))  //电话进来 挂断电话
    {
      Uart1Sends("ATH\r\n");  //
      DelaySec(1);            //延时
      i = 0;
      while (!hand("OK")) {
        clear_rec_data();
        i++;
        Uart1Sends("ATH\r\n");  //
        DelaySec(1);            //延时
        if (i >= 2) {
          break;
          // return;
        } else
          ;
      }
      clear_rec_data();
      DelaySec(1);  //延时

    } else {
      ;
    }
  }
}