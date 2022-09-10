//code:cpp
//程序说明：
//接线方式：
//0-增加时间按钮：按下此按钮后，消毒时间增加五分钟
//1-消毒开始/暂停按钮：按下此按钮后，消毒步骤开始或终止
//2~8-数码管口：分别对应A至G，两数码管同名阴极以并联方式连接，注意电阻
//9~10-数码管阳极口：9对应个位数数码管；10对应十位数数码管
//11-蜂鸣器阳极口
//12-继电器口
//13-led口：该灯在消毒时亮起，不消毒时灭；实际产品没有这部分，仅用于调试和演示效果使用
//需要注意：
//程序调试时，应该打开led接口便于观察效果，建议将*倒计时*部分时间差值改为1000方面演示
//实际工作时，led等的相关代码*需要被注释掉*，并且注意时间差值要设置为60000

int clr[8]={1,1,1,1,1,1,1,1};//清除显示数组
int number[10][8] = 
{
 {0,0,0,0,0,0,1,1}, //显示0
 {1,0,0,1,1,1,1,1}, //显示1
 {0,0,1,0,0,1,0,1}, //显示2
 {0,0,0,0,1,1,0,1}, //显示3
 {1,0,0,1,1,0,0,1}, //显示4
 {0,1,0,0,1,0,0,1}, //显示5
 {0,1,0,0,0,0,0,1}, //显示6
 {0,0,0,1,1,1,1,1}, //显示7
 {0,0,0,0,0,0,0,1}, //显示8
 {0,0,0,0,1,0,0,1} //显示9
};

int led=13;//灯亮表示消毒开始，实际没有这部分，只是便于展示而已
int buzzer=11;//蜂鸣器接口
int relay=12;//继电器使用这个接口，当工作开始时，该口给出低电平
int addTimeButton=0;//增加时间按钮相关设置
int addTimeButtonState=LOW;
long long int lasttimechange1=0;
bool iskeychange1=false;
int lastbutton1state=LOW;

int startButton=1;//开始按钮相关设置
int button2state=LOW;
long long int lasttimechange2=0;
bool iskeychange2=false;
int lastbutton2state=LOW;

int digit1=10;//十位数数码管接线位置
int digit2=9;//个位数数码管接线位置
int lefttime=0;//倒计时剩余时间（默认值）
bool working=false;//记录工作状态
long long int record;//上次减少时间的时刻

void clear(){//该函数清空当前显示
  for(int i=2;i<=8;++i){
    digitalWrite(i,clr[i-2]);
  }

}

//该函数提供欲显示的两位数的接口，效果为显示这个两位数；这个函数每次调用有10ms延迟；该函数需要被反复调用
void displayNumber(int x){
  int dis1=x/10;//显示的十位数数字
  int dis2=x%10;//显示的各位数数字
  
  digitalWrite(digit1,HIGH);//第一位设置为高电平，准备修改第一位数字  
  
  for(int i=2;i<=8;++i){//数码管每段接线位置
    digitalWrite(i,number[dis1][i-2]);//显示数字
  }//end for
  delay(5);//延迟5毫秒，利用视觉暂留现象
  
  clear();//清空显示
  
  digitalWrite(digit1,LOW);//停止修改第一位
  digitalWrite(digit2,HIGH);//准备修改第二位
  
  for(int i=2;i<=8;++i){//同样赋值
    digitalWrite(i,number[dis2][i-2]);
  }//end for
  delay(5);
  
  clear();//清空显示
  
  digitalWrite(digit2,LOW);//停止修改第二位数字
}

void beginworking(){//工作函数
  digitalWrite(led,HIGH);
    working=true;
    digitalWrite(relay,LOW);
}

void endworking(){//停止函数
  digitalWrite(led,LOW);
    working=false;
  digitalWrite(relay,HIGH);
}

void setup()
{
  for(int i=2;i<=8;++i){
    pinMode(i,OUTPUT);
  }//初始化引脚，这些是七段分别对应的引脚（不使用小数点）
  
  pinMode(addTimeButton,INPUT);//初始化时间按钮
  pinMode(startButton,INPUT);//初始化开始按钮
  pinMode(led,OUTPUT);//用于展示的led口
  pinMode(buzzer,OUTPUT);//蜂鸣器接口初始化
  pinMode(relay,OUTPUT);//继电器接口初始化
  
  pinMode(digit1,OUTPUT);
  pinMode(digit2,OUTPUT);//初始化数码管供电
  digitalWrite(digit1,LOW);
  digitalWrite(digit2,LOW);//数码管起始状态不供电
  
  record=millis();//记录程序开始的时间，一般为零
}

void loop()
{
  displayNumber(lefttime);//不断显示剩余时间
    
    //以下部分控制增加时间，每次按下按钮增加五分钟；最多不能超过95分钟，做了时间溢出的保护
    addTimeButtonState=digitalRead(addTimeButton);
    if(addTimeButtonState!=lastbutton1state&&iskeychange1==false){
      lasttimechange1=millis();
      iskeychange1=true;
    }
    if((millis()-lasttimechange1)>50&&iskeychange1==true){//等待去抖时间
      if(addTimeButtonState!=lastbutton1state){//按钮状态还是改变
          lastbutton1state=addTimeButtonState;//更新按钮状态
          iskeychange1=false;
          if(addTimeButtonState==LOW&&lefttime<95){
              lefttime+=5;
          }
        }
    }
  
    //以下部分控制消毒的开始，按键已做消抖处理；工作状态下按下立刻终止消毒
    button2state=digitalRead(startButton);
    if(button2state!=lastbutton2state&&iskeychange2==false){
      lasttimechange2=millis();
      iskeychange2=true;
    }
    if((millis()-lasttimechange2)>50&&iskeychange2==true){//等待去抖时间
      if(button2state!=lastbutton2state){//按钮状态还是改变
          lastbutton2state=button2state;//更新按钮状态
          iskeychange2=false;
          if(button2state==LOW){
              if(!working) beginworking();//没有工作，则开始工作
                else endworking();//在工作，则停止工作
          }
        }
    }
  
    //倒计时开始
    if(working){//如果在工作，那么启动计时器
      if(millis()-record>1000){//间隔一分钟，减少剩余时间（调试时建议使用1000ms）
        lefttime--;
        record=millis();
      }
    }
  
    //计时结束，停止工作，开始警报
  if(lefttime==0&&working){
    endworking();
    for(int i=1;i<=3;++i){//i是蜂鸣次数，可以修改
      tone(buzzer,500);
        delay(1000);
        noTone(buzzer);
        delay(1000);
    }
  }
  
  
}
