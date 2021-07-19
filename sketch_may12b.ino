#include<Servo.h> 
#include <LiquidCrystal.h>
////////////////定义常量/////////////////
#define chan_number 8 //读取的通道总数
#define PPM_INPUT 15 //定义读取引脚
#define ESC_LPin 3  //定义左电调引脚
#define ESC_RPin 2 //定义右电调引脚
#define beep 31    //定义蜂鸣器引脚

//////////////////定义数组/////////////////
int chan_value[chan_number];  //定义各通道值实际数值    数组
int figured_value[chan_number];//换算后的各通道数值    数组  【常用】
int   Midvalue1,Midvalue2,Lowvalue3,Midvalue4;
float turning_Sensitivity = 1; //转弯灵敏度 倍数
//int hands[6]={4,5,6,7,8,9};
int s3_old=0;
int s5_old=0;
///////////////////定义子函数///////////////
void Reading_Value();//读各通道值 子函数
void figure_out_value(int v1,int v2,int v3,int v4);//转换成百分制,v1,v2,v3,v4分别为各通道中值
                                                    //转换后范围{-50~50,-50~50,0~100,-50~50,1200~2000,未定义,未定义,-1|0|1}
void Receiver_Init();//接收机初始化函数
void ESC_Init();//电调初始化 

void R_N_F();//！！！接收并计算，预计耗时20ms，用延时程序时，小延时尽量用这个代替，可以更频繁的更新数据!!!!!!!【常用】隔一阵子运行一次

void ESC_write(int L,int R);//电调赋值（左，右）范围-100~100
void Reporting();//反馈到电脑 子函数
void run_control(int thro,int angle);//运行控制，输入油门(0~100)，转动角度(-50~50)
void lcd_init();
void lcd_battary();
void hands_Init();
void hands_control(int s1,int s2,int s3,int s4,int s5,int s6);
//void hands_free();//不干扰的机械臂控制
void lose_control();



///////////////////舵机定义//////////////////
Servo L_ESC;//左电调
Servo R_ESC;//右电调
Servo hand1;
Servo hand2;
Servo hand3;
Servo hand4;
Servo hand5;//主转盘
Servo hand6;//爪子舵机
LiquidCrystal lcd(47, 46, 44, 42, 40, 38, 36, 34, 32, A14, A12);


///////////////////具体子函数/////////////////

void Reporting()//反馈到电脑 子函数
{
  for(int x=0;x<=7;x++){
     Serial.print(figured_value[x]); //Print the value
     Serial.print(" ");
     //chan_value[x]=0;
    }
    Serial.println();
    //Serial.write(hand1.readMicroseconds());
  }


  
void Reading_Value()//读各通道值 子函数
{
  while(pulseIn(PPM_INPUT,HIGH)<4000);
  for(int x=0;x<=chan_number-1;x++)
      chan_value[x]=pulseIn(PPM_INPUT,HIGH)+500;
    
  }


void Receiver_Init()//接收机初始化函数
{
  for(int i=0;i<=20;i++)
      Reading_Value();  
  Reading_Value();
  Midvalue1=chan_value[0];
  Midvalue2=chan_value[1];
  Lowvalue3=chan_value[2];
  Midvalue4=chan_value[3];
}


void figure_out_value(int v1,int v2,int v3,int v4)
{
  figured_value[0]=(chan_value[0]-v1)/7.88;
  figured_value[1]=(chan_value[1]-v2)/7.88;
  figured_value[2]=(chan_value[2]-v3+5)/7.88;
  figured_value[3]=(chan_value[3]-v4)/7.88;
  
  figured_value[4]=chan_value[4];//未定义
  figured_value[5]=chan_value[5];//未定义
  //figured_value[6]=chan_value[6];//未定义
  
  if (chan_value[7]>=1150 && chan_value[7]<=1250)
    figured_value[7]=1;
  else if (chan_value[7]>=1450 && chan_value[7]<=1600)
    figured_value[7]=0;
  else if (chan_value[7]>=1950 && chan_value[7]<=2050)
    figured_value[7]=-1;
  
  
  if (chan_value[6]>=1150 && chan_value[6]<=1250)
    figured_value[6]=1;
  else if (chan_value[6]>=1950 && chan_value[6]<=2050)
    figured_value[6]=0;
 }

void ESC_Init()
{
  L_ESC.attach(ESC_LPin);
  R_ESC.attach(ESC_RPin);
  L_ESC.write(90);
  R_ESC.write(90);
  delay(3000);
}

void ESC_write(int L,int R)
{
    int left,right;
    L=constrain(L,-100,100);
    R=constrain(R,-100,100);
    
    left=map(L,-100,100,1000,2000);
    right=map(R,-100,100,1000,2000);

    L_ESC.writeMicroseconds(left);
    R_ESC.writeMicroseconds(right);
}


void R_N_F()
{
    Reading_Value();
    figure_out_value(Midvalue1,Midvalue2,Lowvalue3,Midvalue4);
  }

void run_control(int thro,int angle)
{
    int PWM_left,PWM_right;
    if (figured_value[7]==-1)//判断前进还是后退信号
      {
      thro=-thro;
      PWM_left=-1*angle*turning_Sensitivity + thro ;
      PWM_right=1*angle*turning_Sensitivity + thro ;
      }
    else if (figured_value[7]==1)
    {
      PWM_left=1*angle*turning_Sensitivity + thro ;
      PWM_right=-1*angle*turning_Sensitivity + thro ;
    }
    else
    {
      PWM_left=0 ;
      PWM_right=0 ;
      }
    ESC_write(PWM_left,PWM_right);
    //Serial.print(PWM_left   );
    //Serial.println(PWM_right   );
}

void lcd_init()
{
  lcd.begin(16, 2);
  lcd.print("Welcome!");
  tone(beep,600,200);
  lcd.setCursor(8,2);
  lcd.print("--by MIC");
  delay(500);
  tone(beep,600,200);
  delay(500);
  tone(beep,600,200);
  delay(500);
  lcd.clear();
  lcd.print("Waiting for ");
  lcd.setCursor(5,2);
  //lcd.autoscroll();
  lcd.print("Receiver...");
  
  }

void lcd_battary()
{
  float Va1,Va2,Va3;
  Va1=analogRead(A0);
  Va2=analogRead(A1);
  Va3=analogRead(A3);
  //float Ba1=Va1*5/1020;
  //float Ba2=2*Va2*5/1020-Ba1;
  float Ba3=3*Va3*5/1020;
  lcd.clear();
  lcd.print("Robot by MIC");
  lcd.setCursor(0,1);
  lcd.print("Bat Value:");
  lcd.setCursor(11,1);
  lcd.print(Ba3);
  lcd.setCursor(15,1);
  lcd.print("V");
  /*lcd.setCursor(6,2);
  lcd.print(Ba2);
  lcd.setCursor(11,2);
  lcd.print(Ba3);*/
  if(Ba3<10.5&&Ba3>3.8)
    //if (Ba1<3.4||Ba2<3.4||Ba3<3.4)
      tone(beep,800,200);
  }

void hands_Init()
{
  hand1.attach(4);
  hand2.attach(5);
  hand3.attach(6);
  hand4.attach(7);
  hand5.attach(8);
  hand6.attach(9);

  hand1.write(90);
  hand2.writeMicroseconds(2200);
  hand3.write(30);
  hand4.write(90);
  hand5.write(20);
  hand6.write(100);
  
  
  }

void hands_control(int s1,int s2,int s3,int s4,int s5,int s6)
{
  if(abs(s1)>7 )
  {
    int handms1=hand1.readMicroseconds()-s1;
    hand1.writeMicroseconds(constrain(handms1,500,2500));
    }
   if( abs(s4)>7)
   {
    int handms4=hand4.readMicroseconds()+s4;
    hand4.writeMicroseconds(constrain(handms4,500,2500));
    }
   if(abs(s2)>7)
   {
    int handms2=hand2.readMicroseconds()-s2;
    hand2.writeMicroseconds(constrain(handms2,900,2400));
    }
   if(abs(figured_value[4]-s5_old)>70){
    hand5.write(map(figured_value[4],1100,2000,0,180));
    s5_old=figured_value[4]; 
    }
   if(abs(figured_value[5]-s3_old)>70){
    hand3.write(map(figured_value[5],1100,2000,0,180));
    s3_old=figured_value[5];
   } 
   if (figured_value[6])
    hand6.write(0);
   else
    hand6.write(95);
  }


void lose_control()
{
  if (chan_value[6]<=1000){
     lcd.clear();
     lcd.print("Warning: ");
     lcd.setCursor(0,1);
     lcd.print("Losing control !");
     tone(beep,800,200);
     delay(200);
     tone(beep,800,200);
     delay(1500);
     lcd.clear();
     lcd.print("Warning: ");
     lcd.setCursor(0,1);
     lcd.print("Please Restart !");
     tone(beep,800,200);
     delay(200);
     tone(beep,800,200);
     delay(1500);
     ESC_Init();
     hands_Init();
    }
  }


//////////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  hands_Init();
  pinMode(PPM_INPUT,INPUT);
  pinMode(beep,OUTPUT);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A3,INPUT);
  lcd_init();
  Receiver_Init();
  lcd.noAutoscroll();
  lcd.clear();
  lcd.print("Welcome!");
  lcd.setCursor(1,1);
  lcd.print("Initializing...");
  ESC_Init();
  tone(beep,600,500);
    delay(100);
    tone(beep,400,500);
    delay(100);
    tone(beep,800,600);
  lcd.clear();
  lcd.print("Robot Ready!");
  delay(1000);
  R_N_F();
  
  
}








  
void loop() {
  // put your main code here, to run repeatedly:
  R_N_F();
  //Reporting();
  lose_control();
  if(figured_value[7])
    run_control(figured_value[2],figured_value[3]);
  else
    hands_control(figured_value[0],figured_value[1],figured_value[2],figured_value[3],figured_value[4],figured_value[5]);
  //hands_free();
  lcd_battary();
  
  //hand5.writeMicroseconds(figured_value[4]);
  //Serial.write(hand1.readMicroseconds());
}
