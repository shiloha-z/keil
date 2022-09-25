#include "reg52.h"
#include <intrins.h>

#define first_row 0x80
#define secode_row 0x80 + 0x40
#define uint unsigned int
#define uchar unsigned char
#define buzzer_open 1
#define buzzer_close 0

sbit led_en = P2 ^ 7; // led使能
sbit led_rw = P2 ^ 6; //读写
sbit led_rs = P2 ^ 5; //数据、指令选择
sbit background_light = P2 ^ 4;

sbit buzzer = P2 ^ 3;        //蜂鸣器
sbit button_buzzer = P1 ^ 3; //按键反馈音

sbit IO = P2 ^ 1; // 1302计时器
sbit SCLK = P2 ^ 0;
sbit RST = P2 ^ 2;

sbit button1 = P1 ^ 0; //控制按键
sbit button2 = P1 ^ 1;
sbit button3 = P1 ^ 2;

sbit hcsr04_trig = P1 ^ 4;
sbit hcsr04_echo = P1 ^ 5;

uchar code tab1[] = {"    20  -  -"}; //年显示的固定字符
uchar code tab2[] = {"  :  :"};       //时间显示的固定字符
uchar code weekday[7][3] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
uchar second, minute, hour, day, week, month, year, temp, tem, clk_hour, clk_min, clk_on;
uint dis, dis_pre, j;

void buzzer_on(uint);

void ds1302_init(void);        //
void write_1302(uchar, uchar); //向1302写入数据函数，指定读取数据来源地址,和写入数据
uchar read_1302(uchar);        //从1302读数据函数，指定读取数据来源地址

void lcd_init(void);       // lcd1602a
void write_lcd_com(uchar); //***液晶写入命令函数
void write_lcd_dat(uchar); //***液晶写入数据函数
void time_read(void);
void time_first_row_write(uchar, uchar); // lcd时间写入,参数为位置和数值
void time_second_row_write(uchar, uchar);
void time_weekday_write(uchar, uchar);
void lcd_time_write(void);

void initT0(void); //定时器、计数器设置函数
uchar BCD2Decimal(uchar);
void whole_hour_tall_time(void);
void alarm(void);

void CSB(void);      //超声波测距
void startCSB(void); //启动超声波模块
void endTime();      //超声波回来，停止记时
void startTime();    //超声波发出时刻，启动记时
uint getDistance();  //获取距离的函数

void main()
{
    button_buzzer = 0;
    lcd_init();
    ds1302_init(); //计时器初始化
    initT0();      //蜂鸣器异常

    buzzer_on(50); //上移会导致背光调节失效
    clk_on = 0;
    while (1)
        ;
}

void delay10us() //延时10us函数
{
    TH1 = 0xff;
    TL1 = 0xf6;
    TR1 = 1;
    while (!TF1)
        ;
    TF1 = 0;
}
void buzzer_on(uint ms)
{
    buzzer = buzzer_open;
    while ((ms--) > 0)
        delay10us();
    buzzer = buzzer_close;
}
void button_buzzer_on(uint ms)
{
    button_buzzer = buzzer_open;
    while ((ms--) > 0)
        delay10us();
    button_buzzer = buzzer_close;
}
uchar BCD2Decimal(uchar bcd) // BCD码转十进制函数，输入BCD，返回十进制
{
    uchar Decimal;
    Decimal = bcd >> 4;
    return (Decimal = Decimal * 10 + (bcd &= 0x0F));
}
void whole_hour_tall_time(void) //待完善
{
    if (hour == 12 && minute == 0 && second == 0)
        buzzer = buzzer_open;
    if (hour == 12 && minute == 0 && second == 30)
        buzzer = buzzer_close;
}
void alarm(void)
{
    if (hour == clk_hour && minute == clk_min && second == 0)
        buzzer = buzzer_open;
    if (hour == clk_hour && minute == clk_min && second == 30)
        buzzer = buzzer_close;
}

void lcd_init(void)
{
    uint a;
    write_lcd_com(0x38);          //设置液晶工作模式，意思：16*2行显示，5*7点阵，8位数据
    write_lcd_com(0x0c);          //开显示不显示光标
    write_lcd_com(0x06);          //整屏不移动，光标自动右移
    write_lcd_com(0x01);          //清显示
    write_lcd_com(first_row + 1); //日历显示固定符号从第一行第1个位置之后开始显示

    for (a = 0; a < 14; a++)
        write_lcd_dat(tab1[a]);    //向液晶屏写日历显示的固定符号部分
    write_lcd_com(secode_row + 3); //时间显示固定符号写入位置，从第2个位置后开始显示
    for (a = 0; a < 6; a++)
        write_lcd_dat(tab2[a]); //写显示时间固定符号，两个冒号
}
void write_lcd_com(uchar com) //****液晶写入指令函数****
{
    led_rs = 0; //数据/指令选择置为指令
    led_rw = 0; //读写选择置为写
    P0 = com;   //送入数据
    led_en = 1; //拉高使能端，为制造有效的下降沿做准备
    for (j = 6; j > 0; j--)
        delay10us();
    led_en = 0; // led_en由高变低，产生下降沿，液晶执行命令
    for (j = 3; j > 0; j--)
        delay10us();
}
void write_lcd_dat(uchar dat) //***液晶写入数据函数****
{
    uint i = 9;
    led_rs = 1; //数据/指令选择置为数据
    led_rw = 0; //读写选择置为写
    P0 = dat;   //送入数据
    led_en = 1; // led_en置高电平，为制造下降沿做准备
    for (i; i > 0; i--)
        delay10us();
    led_en = 0; // led_en由高变低，产生下降沿，液晶执行命令
}

void ds1302_init() // 1302芯片初始化子函数
{
    RST = 0;
    SCLK = 0;
    write_1302(0x8e, 0x00); //允许写，禁止写保护 10001110
    write_1302(0xa0, 0x00); //允许写，禁止写保护 10100000
    write_1302(0xa0, 0x00); //允许写，禁止写保护
    write_1302(0xa0, 0x00); //允许写，禁止写保护
    /*     write_1302(0x80, 0x30); //秒
        write_1302(0x82, 0x31); //分
        write_1302(0x84, 0x20); //时
        write_1302(0x86, 0x25); //日
        write_1302(0x88, 0x09); //月
        write_1302(0x8c, 0x22); //年 */
    write_1302(0x8e, 0x80);
}
void write_1302(uchar add, uchar dat) //向1302芯片写函数，指定写入地址，数据
{
    uchar i;
    RST = 0;
    SCLK = 0;
    RST = 1;
    for (i = 0; i < 8; i++)
    {
        IO = add & (0x01 << i);
        SCLK = 0;
        SCLK = 1;
    }
    for (i = 0; i < 8; i++)
    {
        IO = dat & (0x01 << i);
        SCLK = 0;
        SCLK = 1;
    }
    SCLK = 0;
}
uchar read_1302(uchar add) //
{
    uchar i, DATA = 0x00;
    RST = 0;
    SCLK = 0;
    RST = 1;
    for (i = 0; i < 8; i++)
    {
        IO = add & (0x01 << i);
        SCLK = 1;
        SCLK = 0;
    }
    for (i = 0; i < 8; i++)
    {
        if (IO)
            DATA = DATA | (0x01 << i);
        SCLK = 1;
        SCLK = 0;
    }
    return DATA;
}

void initT0(void) //定时器、计数器设置函数
{
    TMOD = 0x01;
    TH0 = 0xD8; // 65536-10000
    TL0 = 0xF0; // 0xD8F0=65536-10000
    EA = 1;
    ET0 = 1;
    TR0 = 1;
}

void time_read(void)
{
    second = BCD2Decimal(read_1302(0x81));
    minute = BCD2Decimal(read_1302(0x83));
    hour = BCD2Decimal(read_1302(0x85));
    day = BCD2Decimal(read_1302(0x87));
    week = BCD2Decimal(read_1302(0x8b));
    month = BCD2Decimal(read_1302(0x89));
    year = BCD2Decimal(read_1302(0x8d));
}
void time_first_row_write(uchar add, uchar dat)
{
    uchar unit, decade; //个位十位
    write_lcd_com(first_row + add);
    decade = dat / 10;
    unit = dat % 10;
    write_lcd_dat('0' + decade); //'0'=0x30
    write_lcd_dat('0' + unit);
}
void time_second_row_write(uchar add, uchar dat)
{
    uchar unit, decade; //个位十位
    write_lcd_com(secode_row + add);
    // bai = dat/100;
    decade = dat / 10;
    unit = dat % 10;
    // write_lcd_dat('0'+bai);
    write_lcd_dat('0' + decade);
    write_lcd_dat('0' + unit);
}
void time_weekday_write(uchar add, uchar dat)
{
    write_lcd_com(first_row + add);
    for (j = 0; j < 3; j++)
        write_lcd_dat(weekday[dat - 1][j]);
}
void lcd_time_write(void)
{
    time_first_row_write(3, year);
    time_first_row_write(6, month);
    time_first_row_write(9, day);
    time_second_row_write(3, hour);
    time_second_row_write(6, minute);
    time_second_row_write(9, second);
    time_weekday_write(12, week);
    time_second_row_write(12, temp);
    // time_second_row_write(0, tem);
}

void CSB(void)
{

    startCSB();
    while (hcsr04_echo != 1)
        ; //发出超声波，开始计算时间
    startTime();
    while (hcsr04_echo != 0)
        ; //超声波回来，时间截止
    endTime();
    dis_pre = dis;
    dis = getDistance(); //获取距离

    temp = dis;

    if (dis_pre - dis > 30)
    {
        background_light = 0;
        TH2 = 0x00; // 65536-10000
        TL2 = 0x00; // 55536
        EA = 1;
        ET2 = 1;
        TR2 = 1;
    }
}
void startCSB(void) //启动超声波模块
{
    hcsr04_trig = 0;
    hcsr04_trig = 1;
    delay10us();
    hcsr04_trig = 0;
}
void startTime() //超声波发出时刻，启动记时
{
    TH1 = 0;
    TL1 = 0;
    TR1 = 1;
}
void endTime() //超声波回来，停止记时
{
    TR1 = 0;
}
uint getDistance() //获取距离的函数
{
    uint distance;
    uint time;
    time = TH1 << 8 | TL1;
    distance = (time >> 7) % 100;
    return distance;
}

void timer0_lcd_change() interrupt 1
{
    TH0 = 0xD8;
    TL0 = 0xF0;
    time_read();
    lcd_time_write();

    { //超声波，闹钟
        if (clk_on == 1)
            alarm();
        whole_hour_tall_time(); //整点报时
        CSB();
    }

    { //按键监测
        if (button1 == 0 || button2 == 0 || button3 == 0)
        {
            _nop_();
            if (button1 == 0)
            {
                button_buzzer_on(2000);
            }
            else if (button2 == 0)
            {
                button_buzzer_on(2000);
            }
            else if (button3 == 0)
            {
                button_buzzer_on(2000);
            }
        }
    }
}

void back_ground_delay() interrupt 5
{
    background_light = 0;
}
