#include<reg52.h>
#include <string.h>
#include <intrins.h>
#define uint unsigned int
#define uchar unsigned char



#define yh 0x80 //LCD第一行的初始位置,因为LCD1602字符地址首位D7恒定为1（100000000=80）
#define er 0x80+0x44 //LCD第二行初始位置（因为第二行第一个字符位置地址是0x40）
#define ner 0x80+0x40

//液晶屏的与C51之间的引脚连接定义（显示数据线接C51的P0口）
sbit en=P2^7;
sbit rw=P2^6;   //如果硬件上rw接地，就不用写这句和后面的rw=0了
sbit rs=P2^5;
sbit P37=P3^7;
code uchar TH0T=(65535-1000)/256; 
code uchar TL0T=(65535-1000)%256; 

//校时按键与C51的引脚连接定义

sbit set=P3^0;  	//设置键
sbit add=P3^1;  	//加键
sbit dec=P3^2;  	//减键
sbit seeNL_NZ=P3^3;	//查看/闹钟
	

sbit DQ=P2^4;		//
sbit buzzer=P2^3;	//蜂鸣器，通过三极管8550驱动，端口低电平响


sbit led=P3^7; 		
bit  led1=1;

unsigned char temp_miao;

uchar tflag;//温度正负标志

//DS1302时钟芯片与C51之间的引脚连接定义
sbit IO=P2^1;
sbit SCLK=P2^0;
sbit RST=P2^2;

unsigned int STPtime=0;
uchar oSTPtime=100,STPSec=0,STPMin=0;

uchar a,miao,shi,fen,ri,yue,nian,week,setn,temp;
uint flag;
//flag用于读取头文件中的温度值，和显示温度值
bit c_moon;

uchar nz_shi=12,nz_fen=0,nz_miao=0,setNZn;    	//定义闹钟变量
uchar shangyimiao,bsn,temp_hour;			//记录上一秒时间
uchar T_NL_NZ;							//计数器
bit timerOn=0;							//闹钟启用标志位
bit baoshi=0;							//整点报时标志位
bit  p_r=0;		 						//平年/润年  =0表示平年，=1表示润年
data uchar year_moon,month_moon,day_moon,week;

bit fSTPW=0,fAlarm=0,STPRun=0,STPPause=0;

sbit ACC0=ACC^0;
sbit ACC7=ACC^7;
/************************************************************
ACC累加器=A
ACC.0=E0H 

ACC.0就是ACC的第0位。Acc可以位寻址。

累加器ACC是一个8位的存储单元，是用来放数据的。但是，这个存储单元有其特殊的地位，
是单片机中一个非常关键的单元，很多运算都要通过ACC来进行。以后在学习指令时，
常用A来表示累加器。但有一些地方例外，比如在PUSH指令中，就必须用ACC这样的名字。
一般的说法，A代表了累加器中的内容、而ACC代表的是累加器的地址。 
***************************************************************/



//********阳历转换阴历表************************************
code uchar year_code[597]={
                    0x04,0xAe,0x53,    //1901 0
                    0x0A,0x57,0x48,    //1902 3
                    0x55,0x26,0xBd,    //1903 6
                    0x0d,0x26,0x50,    //1904 9
                    0x0d,0x95,0x44,    //1905 12
                    0x46,0xAA,0xB9,    //1906 15
                    0x05,0x6A,0x4d,    //1907 18
                    0x09,0xAd,0x42,    //1908 21
                    0x24,0xAe,0xB6,    //1909
                    0x04,0xAe,0x4A,    //1910
                    0x6A,0x4d,0xBe,    //1911
                    0x0A,0x4d,0x52,    //1912
                    0x0d,0x25,0x46,    //1913
                    0x5d,0x52,0xBA,    //1914
                    0x0B,0x54,0x4e,    //1915
                    0x0d,0x6A,0x43,    //1916
                    0x29,0x6d,0x37,    //1917
                    0x09,0x5B,0x4B,    //1918
                    0x74,0x9B,0xC1,    //1919
                    0x04,0x97,0x54,    //1920
                    0x0A,0x4B,0x48,    //1921
                    0x5B,0x25,0xBC,    //1922
                    0x06,0xA5,0x50,    //1923
                    0x06,0xd4,0x45,    //1924
                    0x4A,0xdA,0xB8,    //1925
                    0x02,0xB6,0x4d,    //1926
                    0x09,0x57,0x42,    //1927
                    0x24,0x97,0xB7,    //1928
                    0x04,0x97,0x4A,    //1929
                    0x66,0x4B,0x3e,    //1930
                    0x0d,0x4A,0x51,    //1931
                    0x0e,0xA5,0x46,    //1932
                    0x56,0xd4,0xBA,    //1933
                    0x05,0xAd,0x4e,    //1934
                    0x02,0xB6,0x44,    //1935
                    0x39,0x37,0x38,    //1936
                    0x09,0x2e,0x4B,    //1937
                    0x7C,0x96,0xBf,    //1938
                    0x0C,0x95,0x53,    //1939
                    0x0d,0x4A,0x48,    //1940
                    0x6d,0xA5,0x3B,    //1941
                    0x0B,0x55,0x4f,    //1942
                    0x05,0x6A,0x45,    //1943
                    0x4A,0xAd,0xB9,    //1944
                    0x02,0x5d,0x4d,    //1945
                    0x09,0x2d,0x42,    //1946
                    0x2C,0x95,0xB6,    //1947
                    0x0A,0x95,0x4A,    //1948
                    0x7B,0x4A,0xBd,    //1949
                    0x06,0xCA,0x51,    //1950
                    0x0B,0x55,0x46,    //1951
                    0x55,0x5A,0xBB,    //1952
                    0x04,0xdA,0x4e,    //1953
                    0x0A,0x5B,0x43,    //1954
                    0x35,0x2B,0xB8,    //1955
                    0x05,0x2B,0x4C,    //1956
                    0x8A,0x95,0x3f,    //1957
                    0x0e,0x95,0x52,    //1958
                    0x06,0xAA,0x48,    //1959
                    0x7A,0xd5,0x3C,    //1960
                    0x0A,0xB5,0x4f,    //1961
                    0x04,0xB6,0x45,    //1962
                    0x4A,0x57,0x39,    //1963
                    0x0A,0x57,0x4d,    //1964
                    0x05,0x26,0x42,    //1965
                    0x3e,0x93,0x35,    //1966
                    0x0d,0x95,0x49,    //1967
                    0x75,0xAA,0xBe,    //1968
                    0x05,0x6A,0x51,    //1969
                    0x09,0x6d,0x46,    //1970
                    0x54,0xAe,0xBB,    //1971
                    0x04,0xAd,0x4f,    //1972
                    0x0A,0x4d,0x43,    //1973
                    0x4d,0x26,0xB7,    //1974
                    0x0d,0x25,0x4B,    //1975
                    0x8d,0x52,0xBf,    //1976
                    0x0B,0x54,0x52,    //1977
                    0x0B,0x6A,0x47,    //1978
                    0x69,0x6d,0x3C,    //1979
                    0x09,0x5B,0x50,    //1980
                    0x04,0x9B,0x45,    //1981
                    0x4A,0x4B,0xB9,    //1982
                    0x0A,0x4B,0x4d,    //1983
                    0xAB,0x25,0xC2,    //1984
                    0x06,0xA5,0x54,    //1985
                    0x06,0xd4,0x49,    //1986
                    0x6A,0xdA,0x3d,    //1987
                    0x0A,0xB6,0x51,    //1988
                    0x09,0x37,0x46,    //1989
                    0x54,0x97,0xBB,    //1990
                    0x04,0x97,0x4f,    //1991
                    0x06,0x4B,0x44,    //1992
                    0x36,0xA5,0x37,    //1993
                    0x0e,0xA5,0x4A,    //1994
                    0x86,0xB2,0xBf,    //1995
                    0x05,0xAC,0x53,    //1996
                    0x0A,0xB6,0x47,    //1997
                    0x59,0x36,0xBC,    //1998
                    0x09,0x2e,0x50,    //1999 294
                    0x0C,0x96,0x45,    //2000 297
                    0x4d,0x4A,0xB8,    //2001
                    0x0d,0x4A,0x4C,    //2002
                    0x0d,0xA5,0x41,    //2003
                    0x25,0xAA,0xB6,    //2004
                    0x05,0x6A,0x49,    //2005
                    0x7A,0xAd,0xBd,    //2006
                    0x02,0x5d,0x52,    //2007
                    0x09,0x2d,0x47,    //2008
                    0x5C,0x95,0xBA,    //2009
                    0x0A,0x95,0x4e,    //2010
                    0x0B,0x4A,0x43,    //2011
                    0x4B,0x55,0x37,    //2012
                    0x0A,0xd5,0x4A,    //2013
                    0x95,0x5A,0xBf,    //2014
                    0x04,0xBA,0x53,    //2015
                    0x0A,0x5B,0x48,    //2016
                    0x65,0x2B,0xBC,    //2017
                    0x05,0x2B,0x50,    //2018
                    0x0A,0x93,0x45,    //2019
                    0x47,0x4A,0xB9,    //2020
                    0x06,0xAA,0x4C,    //2021
                    0x0A,0xd5,0x41,    //2022
                    0x24,0xdA,0xB6,    //2023
                    0x04,0xB6,0x4A,    //2024
                    0x69,0x57,0x3d,    //2025
                    0x0A,0x4e,0x51,    //2026
                    0x0d,0x26,0x46,    //2027
                    0x5e,0x93,0x3A,    //2028
                    0x0d,0x53,0x4d,    //2029
                    0x05,0xAA,0x43,    //2030
                    0x36,0xB5,0x37,    //2031
                    0x09,0x6d,0x4B,    //2032
                    0xB4,0xAe,0xBf,    //2033
                    0x04,0xAd,0x53,    //2034
                    0x0A,0x4d,0x48,    //2035
                    0x6d,0x25,0xBC,    //2036
                    0x0d,0x25,0x4f,    //2037
                    0x0d,0x52,0x44,    //2038
                    0x5d,0xAA,0x38,    //2039
                    0x0B,0x5A,0x4C,    //2040
                    0x05,0x6d,0x41,    //2041
                    0x24,0xAd,0xB6,    //2042
                    0x04,0x9B,0x4A,    //2043
                    0x7A,0x4B,0xBe,    //2044
                    0x0A,0x4B,0x51,    //2045
                    0x0A,0xA5,0x46,    //2046
                    0x5B,0x52,0xBA,    //2047
                    0x06,0xd2,0x4e,    //2048
                    0x0A,0xdA,0x42,    //2049
                    0x35,0x5B,0x37,    //2050
                    0x09,0x37,0x4B,    //2051
                    0x84,0x97,0xC1,    //2052
                    0x04,0x97,0x53,    //2053
                    0x06,0x4B,0x48,    //2054
                    0x66,0xA5,0x3C,    //2055
                    0x0e,0xA5,0x4f,    //2056
                    0x06,0xB2,0x44,    //2057
                    0x4A,0xB6,0x38,    //2058
                    0x0A,0xAe,0x4C,    //2059
                    0x09,0x2e,0x42,    //2060
                    0x3C,0x97,0x35,    //2061
                    0x0C,0x96,0x49,    //2062
                    0x7d,0x4A,0xBd,    //2063
                    0x0d,0x4A,0x51,    //2064
                    0x0d,0xA5,0x45,    //2065
                    0x55,0xAA,0xBA,    //2066
                    0x05,0x6A,0x4e,    //2067
                    0x0A,0x6d,0x43,    //2068
                    0x45,0x2e,0xB7,    //2069
                    0x05,0x2d,0x4B,    //2070
                    0x8A,0x95,0xBf,    //2071
                    0x0A,0x95,0x53,    //2072
                    0x0B,0x4A,0x47,    //2073
                    0x6B,0x55,0x3B,    //2074
                    0x0A,0xd5,0x4f,    //2075
                    0x05,0x5A,0x45,    //2076
                    0x4A,0x5d,0x38,    //2077
                    0x0A,0x5B,0x4C,    //2078
                    0x05,0x2B,0x42,    //2079
                    0x3A,0x93,0xB6,    //2080
                    0x06,0x93,0x49,    //2081
                    0x77,0x29,0xBd,    //2082
                    0x06,0xAA,0x51,    //2083
                    0x0A,0xd5,0x46,    //2084
                    0x54,0xdA,0xBA,    //2085
                    0x04,0xB6,0x4e,    //2086
                    0x0A,0x57,0x43,    //2087
                    0x45,0x27,0x38,    //2088
                    0x0d,0x26,0x4A,    //2089
                    0x8e,0x93,0x3e,    //2090
                    0x0d,0x52,0x52,    //2091
                    0x0d,0xAA,0x47,    //2092
                    0x66,0xB5,0x3B,    //2093
                    0x05,0x6d,0x4f,    //2094
                    0x04,0xAe,0x45,    //2095
                    0x4A,0x4e,0xB9,    //2096
                    0x0A,0x4d,0x4C,    //2097
                    0x0d,0x15,0x41,    //2098
                    0x2d,0x92,0xB5,    //2099
};

///月份数据表
code uchar day_code1[9]={0x0,0x1f,0x3b,0x5a,0x78,0x97,0xb5,0xd4,0xf3};
code uint day_code2[3]={0x111,0x130,0x14e};



/*
函数功能:输入BCD阳历数据,输出BCD星期数据(只允许1901-2099年)
调用函数示例:Conver_week(c_sun,year_sun,month_sun,day_sun)
如:计算2004年10月16日Conversion(0,0x4,0x10,0x16);
c_sun,year_sun,month_sun,day_sun均为BCD数据,c_sun为世纪标志位,c_sun=0为21世
纪,c_sun=1为19世纪
调用函数后,原有数据不变,读week得出阴历BCD数据
*/
code uchar table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表
/*
算法:日期+年份+所过闰年数+月较正数之和除7 的余数就是星期但如果是在
闰年又不到3 月份上述之和要减一天再除7
星期数为0
*/
void Conver_week(uchar year,uchar month,uchar day)
{//c=0 为21世纪,c=1 为19世纪 输入输出数据均为BCD数据
    uchar p1,p2;
    year+=0x64;  //如果为21世纪,年份数加100
    p1=year/0x4;  //所过闰年数只算1900年之后的
    p2=year+p1;
    p2=p2%0x7;  //为节省资源,先进行一次取余,避免数大于0xff,避免使用整型数据
    p2=p2+day+table_week[month-1];
    if (year%0x4==0&&month<3)p2-=1;
    week=p2%0x7;
}


/**************************************************************/

uchar code tab1[]={"20  -  -   "}; 	//年显示的固定字符
uchar code tab2[]={"  :  :  "};		//时间显示的固定字符

uchar code NZd[]={"timer:    -  -  "};	//显示闹钟固定点
uchar code qk[]= {"                "};	//清空显示
uchar code tm[]= {"time"};





//*****************************************************************************

//延时函数，后面经常调用
void delay(uint xms)//延时函数，有参函数
{
	uint x,y;
	for(x=xms;x>0;x--)
	 for(y=110;y>0;y--);
}

/********液晶写入指令函数与写入数据函数，以后可调用**************/

/*在这个程序中，液晶写入有关函数会在DS1302的函数中调用，所以液晶程序要放在前面*/

void write_1602com(uchar com)//****液晶写入指令函数****
{
	rs=0;//数据/指令选择置为指令
	rw=0; //读写选择置为写
	P0=com;//送入数据
	delay(1);
	en=1;//拉高使能端，为制造有效的下降沿做准备
	delay(1);
	en=0;//en由高变低，产生下降沿，液晶执行命令
}


void write_1602dat(uchar dat)//***液晶写入数据函数****
{
	rs=1;//数据/指令选择置为数据
	rw=0; //读写选择置为写
	P0=dat;//送入数据
	delay(1);
	en=1; //en置高电平，为制造下降沿做准备
	delay(1);
	en=0; //en由高变低，产生下降沿，液晶执行命令
}


void lcd_init(void)//***液晶初始化函数****
{
	write_1602com(0x38);//设置液晶工作模式，意思：16*2行显示，5*7点阵，8位数据
	write_1602com(0x0c);//开显示不显示光标
	write_1602com(0x06);//整屏不移动，光标自动右移
	write_1602com(0x01);//清显示

	write_1602com(yh+1);//日历显示固定符号从第一行第1个位置之后开始显示
	for(a=0;a<14;a++)
	{
	write_1602dat(tab1[a]);//向液晶屏写日历显示的固定符号部分
	//delay(3);
	}
	write_1602com(er);//时间显示固定符号写入位置，从第2个位置后开始显示
	for(a=0;a<8;a++)
	{
	write_1602dat(tab2[a]);//写显示时间固定符号，两个冒号
	//delay(3);
	}

}

/*********************over***********************/


/***************DS1302有关子函数********************/
void write_byte(uchar dat)//写一个字节
{
	ACC=dat;
	RST=1;
	for(a=8;a>0;a--)
	{
		IO=ACC0;
		SCLK=0;
		SCLK=1;
		ACC=ACC>>1;
	}
}
uchar read_byte()//读一个字节
{
	RST=1;
	for(a=8;a>0;a--)
	{
		ACC7=IO;
		SCLK=1;
		SCLK=0;
		ACC=ACC>>1;

	}
	return (ACC);
}
//----------------------------------------
void write_1302(uchar add,uchar dat)//向1302芯片写函数，指定写入地址，数据
{
	RST=0;
	SCLK=0;
	RST=1;
	write_byte(add);
	write_byte(dat);
	SCLK=1;
	RST=0;
}
uchar read_1302(uchar add)//从1302读数据函数，指定读取数据来源地址
{
	uchar temp;
	RST=0;
	SCLK=0;
	RST=1;
	write_byte(add);
	temp=read_byte();
	SCLK=1;
	RST=0;
	return(temp);
}

uchar BCD_Decimal(uchar bcd)//BCD码转十进制函数，输入BCD，返回十进制
{
	 uchar Decimal;
	 Decimal=bcd>>4;
	 return(Decimal=Decimal*10+(bcd&=0x0F));
}

//--------------------------------------
void ds1302_init() //1302芯片初始化子函数(2010-01-07,12:00:00,week4)
{
	RST=0;
	SCLK=0;
	
	write_1302(0x8e,0x00); //允许写，禁止写保护 
	write_1302(0xa0,0x00); //允许写，禁止写保护 
	write_1302(0xa0,0x00); //允许写，禁止写保护 
	write_1302(0xa0,0x00); //允许写，禁止写保护 
	
	//write_1302(0x80,0x00); //向DS1302内写秒寄存器80H写入初始秒数据00
	//write_1302(0x82,0x00);//向DS1302内写分寄存器82H写入初始分数据00
	//write_1302(0x84,0x12);//向DS1302内写小时寄存器84H写入初始小时数据12
	//write_1302(0x8a,0x04);//向DS1302内写周寄存器8aH写入初始周数据4
	//write_1302(0x86,0x07);//向DS1302内写日期寄存器86H写入初始日期数据07
	//write_1302(0x88,0x01);//向DS1302内写月份寄存器88H写入初始月份数据01
	//write_1302(0x8c,0x10);//向DS1302内写年份寄存器8cH写入初始年份数据10
	write_1302(0x8e,0x80); //打开写保护
}




//------------------------------------
//时分秒显示子函数
void write_sfm(uchar add,uchar dat)//向LCD写时分秒,有显示位置加、现示数据，两个参数
{
	uchar gw,sw;
	gw=dat%10;//取得个位数字
	sw=dat/10;//取得十位数字
   write_1602com(ner+2);//er是头文件规定的值0x80+0x40
	write_1602dat(' ');// 空格
	write_1602com(er+add);//er是头文件规定的值0x80+0x40
	write_1602dat(0x30+sw);//数字+30得到该数字的LCD1602显示码
	write_1602dat(0x30+gw);//数字+30得到该数字的LCD1602显示码				
}

//------------------------------------
//时分秒显示子函数
void write_nsfm(uchar add,uchar dat)//向LCD写时分秒,有显示位置加、现示数据，两个参数
{
	uchar gw,sw;
	gw=dat%10;//取得个位数字
	sw=dat/10;//取得十位数字
	write_1602com(ner+add);//er是头文件规定的值0x80+0x40
	write_1602dat(0x30+sw);//数字+30得到该数字的LCD1602显示码
	write_1602dat(0x30+gw);//数字+30得到该数字的LCD1602显示码				
}

//-------------------------------------
//年月日显示子函数
void write_nyr(uchar add,uchar dat)//向LCD写年月日，有显示位置加数、显示数据，两个参数
{
	uchar gw,sw;
	gw=dat%10;//取得个位数字
	sw=dat/10;//取得十位数字
	write_1602com(yh+add);//设定显示位置为第一个位置+add
	write_1602dat(0x30+sw);//数字+30得到该数字的LCD1602显示码
	write_1602dat(0x30+gw);//数字+30得到该数字的LCD1602显示码	
}




//-------------------------------------------
void write_week(uchar week)//写星期函数
{
	write_1602com(yh+0x0c);//星期字符的显示位置
	switch(week)
	{
		case 1:write_1602dat('M');//星期数为1时，显示
			   write_1602dat('O');
			   write_1602dat('N');
            write_1602dat(' ');
			   break;
	   
		case 2:write_1602dat('T');//星期数据为2时显示
			   write_1602dat('U');
			   write_1602dat('E');
            write_1602dat(' ');
			   break;
		
		case 3:write_1602dat('W');//星期数据为3时显示
			   write_1602dat('E');
			   write_1602dat('D');
            write_1602dat(' ');
			   break;
		
		case 4:write_1602dat('T');//星期数据为4是显示
			   write_1602dat('H');
			   write_1602dat('U');
            write_1602dat(' ');
			   break;
		
		case 5:write_1602dat('F');//星期数据为5时显示
			   write_1602dat('R');
			   write_1602dat('I');
            write_1602dat(' ');
			   break;
		
		case 6:write_1602dat('S');//星期数据为6时显示
			   write_1602dat('T');
			   write_1602dat('A');
            write_1602dat(' ');
			   break;
		
		case 0:write_1602dat('S');//星期数据为7时显示
			   write_1602dat('U');
			   write_1602dat('N');
            write_1602dat(' ');
			   break;
      
	}
   
}


//****************键盘扫描有关函数**********************
void keyscan()
{
	if(seeNL_NZ==0)
	{
		delay(9);
		if(seeNL_NZ==0)
		{
			led1=0;
			if((setn==0)&&(setNZn==0))								//在没有进入调时模式时才可按动
			{
				buzzer=0;//蜂鸣器短响一次
	    		delay(20);
	    		buzzer=1;
	
				if(fAlarm==1)
				{
					fAlarm=0;
               TR1=0;
               buzzer=1;   
				}
				else
				{			
					T_NL_NZ++;
               if(T_NL_NZ==2) TR1=1;
					if(T_NL_NZ==3)
					{
						setn=0;
						setNZn=0;
						T_NL_NZ=0;	
                  STPPause=0;STPRun=0;STPMin=0;STPSec=0;STPtime=0;
                  write_1602com(er+8);
                  write_1602dat(' ');
					}
				}			
			}
			while(seeNL_NZ==0);
         
         
		}
	}
			

	if((set==0)&&(T_NL_NZ!=2))//---------------set为功能键（设置键）--------------------
	{
		delay(9);//延时，用于消抖动
		if(set==0)//延时后再次确认按键按下
		{
			led1=0;


    		buzzer=0;//蜂鸣器短响一次
    		delay(20);
    		buzzer=1;
			while(!set);
			if(T_NL_NZ==0x01)			//证明是对闹钟进行设置
			{
				setNZn++;
				if(setNZn==4)			//闹钟设定成功，退回到正常显示并开启闹钟
				{
					setNZn=0;
					setn=0;TR1=0;
					timerOn=1;				
				}
				switch(setNZn)
				{
					case 0:						//正常显示日期时间
						write_1602com(0x0c);	//柚霉獗瓴簧了?
						write_1602com(ner);		//时间显示固定符号写入位置?
						for(a=0;a<16;a++)
						write_1602dat(NZd[a]);	//写显示时间固定符号，两个冒号
						
						write_nsfm(8,nz_shi);	//闹钟 时
						write_nsfm(11,nz_fen);	//闹钟 分
						write_nsfm(14,nz_miao);	//闹钟 秒
						break;
					case 1:				   		//闹钟秒光标闪烁		
						write_1602com(ner+15);	//设置按键按动一次，秒位置显示光标   //er+0x09;
				  	 	write_1602com(0x0f);	//设置光标为闪烁
						break;
					case 2:						//闹钟分光标闪烁	
						write_1602com(ner+12);	//设置按键按动一次，秒位置显示光标   //er+0x09;
				  	 	write_1602com(0x0f);	//设置光标为闪烁
						break;
					case 3:						//闹钟时光标闪烁	
						write_1602com(ner+9);	//设置按键按动一次，秒位置显示光标   //er+0x09;
				  	 	write_1602com(0x0f);	//设置光标为闪烁
						break;	
				}	
			}

			else								//证明是对时间及日期进行设置
			{
				if(T_NL_NZ==0)
				{
					setn++;
					if(setn==7)
						setn=0;			//设置按键共有秒、分、时、星期、日、月、年、返回，8个功能循环
					switch(setn)
					{
			
						case 1: TR0=0;//关闭定时器
						//TR1=0;
						write_1602com(er+7);//设置按键按动一次，秒位置显示光标   //er+0x09;
				  	 	write_1602com(0x0f);//设置光标为闪烁
				  	 	temp=(miao)/10*16+(miao)%10;//秒数据写入DS1302
				  	 	write_1302(0x8e,0x00);
				  	 	write_1302(0x80,0x80|temp);//miao
				  	    write_1302(0x8e,0x80);
				  	 	break;
						case 2:  
						write_1602com(er+4);  //按2次fen位置显示光标   //er+0x06	
				   	 	 //write_1602com(0x0f);
						break;
						case 3: 
						write_1602com(er+1);   //按动3次，shi
				     	//write_1602com(0x0f);
						break;
					//	case 4: write_1602com(yh+0x0e);//按动4次，week
				 	    //write_1602com(0x0f);
					//	break;
						case 4: write_1602com(yh+0x0a);//按动4次，ri
					     //write_1602com(0x0f);
						break;
						case 5: write_1602com(yh+0x07);//按动5次，yue
					     //write_1602com(0x0f);
						break;
						case 6: write_1602com(yh+0x04);//按动6次，nian
				  	   //write_1602com(0x0f);
						break;
						case 0:
						write_1602com(0x0c);//按动到第7次，设置光标不闪烁
						TR0=1;//打开定时器
		        	 	temp=(miao)/10*16+(miao)%10;
					   	write_1302(0x8e,0x00);
				   		write_1302(0x80,0x00|temp);//miao数据写入DS1302
				   		write_1302(0x8e,0x80);
		            	break;	
					}													
				}
			}
		}
	}
//------------------------------加键add----------------------------		
	if((setn!=0)&&(setNZn==0))//当set按下以下。再按以下键才有效（按键次数不等于零）
	{
		if(add==0)  //上调键
		{
			delay(10);
			if(add==0)
			{
				led1=0;


			    buzzer=0;//蜂鸣器短响一次
			    delay(20);
			    buzzer=1;
				while(!add);
				switch(setn)
				{
					case 1:miao++;//设置键按动1次，调秒
							if(miao==60)
								miao=0;//秒超过59，再加1，就归零
							write_sfm(0x06,miao);//令LCD在正确位置显示"加"设定好的秒数
							temp=(miao)/10*16+(miao)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00); //允许写，禁止写保护 
						   	write_1302(0x80,temp); //向DS1302内写秒寄存器80H写入调整后的秒数据BCD码
						   	write_1302(0x8e,0x80); //打开写保护
							write_1602com(er+7);//因为设置液晶的模式是写入数据后，光标自动右移，所以要指定返回
							//write_1602com(0x0b);
							break;
					case 2:fen++;
							if(fen==60)
								fen=0;
							write_sfm(0x03,fen);//令LCD在正确位置显示"加"设定好的分数据
							temp=(fen)/10*16+(fen)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护 
						   	write_1302(0x82,temp);//向DS1302内写分寄存器82H写入调整后的分数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(er+4);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
							break;
					case 3:shi++;
							if(shi==24)
								shi=0;
							write_sfm(0x00,shi);//令LCD在正确的位置显示"加"设定好的小时数据
							temp=(shi)/10*16+(shi)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护 
						   	write_1302(0x84,temp);//向DS1302内写小时寄存器84H写入调整后的小时数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(er+1);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
							break;
					/*
					case 4:week++;
							if(week==8)
								week=1;
				            write_1602com(yh+0x0C);//指定'加'后的周数据显示位置
								write_week(week);//指定周数据显示内容
				            temp=(week)/10*16+(week)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护 
						   	write_1302(0x8a,temp);//向DS1302内写周寄存器8aH写入调整后的周数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							   write_1602com(yh+0x0e);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
							break;
					*/
					case 4:ri++;
							if((yue==12)||(yue==10)||(yue==8)||(yue==7)||(yue==5)||(yue==3)||(yue==1))
                  {
                     if(ri>=32)  ri=1;
                  }  
                  else if(yue!=2)
                  {
                     if(ri>=31)  ri=1;
                  }
						else
                  {
                     if(((nian%4)==0))
							{
                        if(nian%100==0)
                        {
                           if((nian%400)==0) if(ri==30)  ri=1;
                           else if(ri>=29)  ri=1;
                        }
                        else if(ri>29)  ri=1;
                     }
                     else if(ri>=29)  ri=1;
                  }
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(9,ri);//令LCD在正确的位置显示"加"设定好的日期数据
							temp=(ri)/10*16+(ri)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护
						   	write_1302(0x86,temp);//向DS1302内写日期寄存器86H写入调整后的日期数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(yh+10);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
				
							break;
					case 5:yue++;
							if(yue==13)	yue=1;
                     if((yue==11)||(yue==9)||(yue==6)||(yue==4))
                     {
                        if(ri>30)  {ri=30;write_nyr(9,ri);}//令LCD在正确的位置显示"加"设定好的日期数据
                     }  
                     else if(yue==2)
                     {
                        if(((nian%4)==0))
                        {
                           if(nian%100==0)
                           {
                              if((nian%400)==0) 
                              {
                                 if(ri>29)
                                 {ri=29;write_nyr(9,ri);}
                              }
                           }
                           else if(ri>29) {ri=29;write_nyr(9,ri);}
                        }
                        else if(ri>28)  {ri=28;write_nyr(9,ri);}
                        
                     }
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(6,yue);//令LCD在正确的位置显示"加"设定好的月份数据
							temp=(yue)/10*16+(yue)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护
						   	write_1302(0x88,temp);//向DS1302内写月份寄存器88H写入调整后的月份数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(yh+7);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
				
							break;
					case 6:nian++;
						 if(nian==100)
							nian=0;
                     if(yue==2)
                     {
                        if(((nian%4)==0))
                        {
                           if(nian%100==0)
                           {
                              if((nian%400)==0) 
                              {
                                 if(ri>29)
                                 {ri=29;write_nyr(9,ri);}
                              }
                           }
                           else if(ri>29) {ri=29;write_nyr(9,ri);}
                        }
                        else if(ri>28)  {ri=28;write_nyr(9,ri);}
                        
                     }
							Conver_week(nian,yue,ri);
							write_week(week);
							write_nyr(3,nian);//令LCD在正确的位置显示"加"设定好的年份数据
				            temp=(nian)/10*16+(nian)%10;//十进制转换成DS1302要求的DCB码
						   	write_1302(0x8e,0x00);//允许写，禁止写保护
						   	write_1302(0x8c,temp);//向DS1302内写年份寄存器8cH写入调整后的年份数据BCD码
						   	write_1302(0x8e,0x80);//打开写保护
							write_1602com(yh+4);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
				
							break;
				}
			}
	
		}
		//------------------减键dec，各句功能参照'加键'注释---------------
		if(dec==0)
		{
			delay(10);//调延时，消抖动
			if(dec==0)
			{
				led1=0;


		    	buzzer=0;//蜂鸣器短响一次
			    delay(20);
			    buzzer=1;
				while(!dec);
				switch(setn)
				{
					case 1:
						miao--;
						if(miao==-1)
							miao=59;//秒数据减到-1时自动变成59
						write_sfm(0x06,miao);//在LCD的正确位置显示改变后新的秒数
			            temp=(miao)/10*16+(miao)%10;//十进制转换成DS1302要求的DCB码
					   	write_1302(0x8e,0x00); //允许写，禁止写保护 
					   	write_1302(0x80,temp); //向DS1302内写秒寄存器80H写入调整后的秒数据BCD码
					   	write_1302(0x8e,0x80); //打开写保护
						write_1602com(er+7);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
						//write_1602com(0x0b);
						break;
					case 2:
						fen--;
						if(fen==-1)
						fen=59;
						write_sfm(3,fen);
						temp=(fen)/10*16+(fen)%10;//十进制转换成DS1302要求的DCB码
					   	write_1302(0x8e,0x00);//允许写，禁止写保护 
					   	write_1302(0x82,temp);//向DS1302内写分寄存器82H写入调整后的分数据BCD码
					   	write_1302(0x8e,0x80);//打开写保护
						write_1602com(er+4);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
						break;
	
					case 3:
						shi--;
				  		if(shi==-1)
						shi=23;
						write_sfm(0,shi);
						temp=(shi)/10*16+(shi)%10;//十进制转换成DS1302要求的DCB码
					   	write_1302(0x8e,0x00);//允许写，禁止写保护 
					   	write_1302(0x84,temp);//向DS1302内写小时寄存器84H写入调整后的小时数据BCD码
					   	write_1302(0x8e,0x80);//打开写保护
						write_1602com(er+1);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
						break;
			
					case 4:
						ri--;
						if((yue==12)||(yue==10)||(yue==8)||(yue==7)||(yue==5)||(yue==3)||(yue==1))
                  {
                     if(ri==0)  ri=31;
                  }  
                  else if(yue!=2)
                  {
                     if(ri==0)  ri=30;
                  }
						else
                  {
                     if(((nian%4)==0))
							{
                        if(nian%100==0)
                        {
                           if((nian%400)==0) if(ri==0)  ri=29;
                           else if(ri==0)  ri=28;
                        }
                        else if(ri==0)  ri=29;
                     }
                     else if(ri==0)  ri=28;
                  }
						Conver_week(nian,yue,ri);
						write_week(week);
						write_nyr(9,ri);
						temp=(ri)/10*16+(ri)%10;//十进制转换成DS1302要求的DCB码
					   	write_1302(0x8e,0x00);//允许写，禁止写保护
					   	write_1302(0x86,temp);//向DS1302内写日期寄存器86H写入调整后的日期数据BCD码
					   	write_1302(0x8e,0x80);//打开写保护
						write_1602com(yh+10);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位		
						break;
					case 5:
						yue--;
                  if(yue==0)
						yue=12;

                  if((yue==11)||(yue==9)||(yue==6)||(yue==4))
                     {
                        if(ri>30)  {ri=30;write_nyr(9,ri);}//令LCD在正确的位置显示"加"设定好的日期数据
                     }  
                     else if(yue==2)
                     {
                        if(((nian%4)==0))
                        {
                           if(nian%100==0)
                           {
                              if((nian%400)==0) 
                              {
                                 if(ri>29)
                                 {ri=29;write_nyr(9,ri);}
                              }
                           }
                           else if(ri>29) {ri=29;write_nyr(9,ri);}
                        }
                        else if(ri>28)  {ri=28;write_nyr(9,ri);}
                        
                     }
						Conver_week(nian,yue,ri);
						write_week(week);
						write_nyr(6,yue);
						temp=(yue)/10*16+(yue)%10;//十进制转换成DS1302要求的DCB码
					   	write_1302(0x8e,0x00);//允许写，禁止写保护
					   	write_1302(0x88,temp);//向DS1302内写月份寄存器88H写入调整后的月份数据BCD码
					   	write_1302(0x8e,0x80);//打开写保护
						write_1602com(yh+7);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
			
						break;	
					case 6:
						nian--;
			 			if(nian==-1)
						nian=99;
                  if(nian==100)
							nian=0;
                     if(yue==2)
                     {
                        if(((nian%4)==0))
                        {
                           if(nian%100==0)
                           {
                              if((nian%400)==0) 
                              {
                                 if(ri>29)
                                 {ri=29;write_nyr(9,ri);}
                              }
                           }
                           else if(ri>29) {ri=29;write_nyr(9,ri);}
                        }
                        else if(ri>28)  {ri=28;write_nyr(9,ri);}
                        
                     }
						Conver_week(nian,yue,ri);
						write_week(week);
						write_nyr(3,nian);
			         	temp=(nian)/10*16+(nian)%10;//十进制转换成DS1302要求的DCB码
					   	write_1302(0x8e,0x00);//允许写，禁止写保护
					   	write_1302(0x8c,temp);//向DS1302内写年份寄存器8cH写入调整后的年份数据BCD码
					   	write_1302(0x8e,0x80);//打开写保护
						write_1602com(yh+4);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
						break;
								
				}
			}
		}
	}
	if((setNZn!=0)&&(setn==0))
	{
		if(add==0)  //上调键
		{
			delay(10);
			if(add==0)
			{
				led1=0;


			    buzzer=0;//蜂鸣器短响一次
			    delay(20);
			    buzzer=1;
				while(!add);
				switch(setNZn)
				{
					case 1:
						nz_miao++;				//设置键按动1次，调秒
						if(nz_miao==60)
							nz_miao=0;//秒超过59，再加1，就归零
						write_nsfm(14,nz_miao);//令LCD在正确位置显示"加"设定好的秒数
						write_1602com(ner+15);//因为设置液晶的模式是写入数据后，光标自动右移，所以要指定返回
						break;
					case 2:
						nz_fen++;
						if(nz_fen==60)
							nz_fen=0;
						write_nsfm(11,nz_fen);//令LCD在正确位置显示"加"设定好的分数据
						write_1602com(ner+12);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
						break;
					case 3:
						nz_shi++;
						if(nz_shi==24)
							nz_shi=0;
						write_nsfm(8,nz_shi);//令LCD在正确的位置显示"加"设定好的小时数据
						write_1602com(ner+9);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
						break;
				}
			}
	
		}
		//------------------减键dec，各句功能参照'加键'注释---------------
		if(dec==0)
		{
			delay(10);//调延时，消抖动
			if(dec==0)
			{
				led1=0;


		    	buzzer=0;//蜂鸣器短响一次
			    delay(20);
			    buzzer=1;
				while(!dec);
				switch(setNZn)
				{
					case 1:
						nz_miao--;
						if(nz_miao==-1)
							nz_miao=59;//秒数据减到-1时自动变成59
						write_nsfm(14,nz_miao);//在LCD的正确位置显示改变后新的秒数
						write_1602com(ner+15);
						break;
					case 2:
						nz_fen--;
						if(nz_fen==-1)
							nz_fen=59;
						write_nsfm(11,nz_fen);
						write_1602com(ner+12);//因为设置液晶的模式是写入数据后，指针自动加一，在这里是写回原来的位置
						break;
	
					case 3:
						nz_shi--;
				  		if(nz_shi==-1)
						nz_shi=23;
						write_nsfm(8,nz_shi);
						write_1602com(ner+9);//因为设置液晶的模式是写入数据后，指针自动加一，所以需要光标回位
						break;
								
				}
			}
		}
	}	
   
   if((T_NL_NZ==2))
   {
      
		if(set==0)
      {
         delay(10);//调延时，消抖动
         if(set==0)  
         {
            STPRun=!STPRun;
            if(STPRun==0)  {STPPause=0;STPMin=0;STPSec=0;STPtime=0;}
            else STPPause=1;
         }
      }
      while(set==0);
      
      if(add==0)
      {
         delay(10);//调延时，消抖动
         if(add==0)  
         {
            if(STPPause==1)  STPRun=!STPRun;
         }
      }
      while(add==0);
      
      if(dec==0)
      {
         delay(10);//调延时，消抖动
         if(dec==0)  {STPMin=0;STPSec=0;STPtime=0;}
      }
      while(dec==0);
   }
}

//-------------------------------
void init(void)   //定时器、计数器设置函数
{
	TMOD=0x11; 		//指定定时/计数器的工作方式为3
	TH0=0;//(65535-50000)/256; 			//定时器T0的高四位=0
	TL0=0;//(65535-50000)%256;  		//定时器T0的低四位=0
	TH1=(65535-10000)/256; //TH0T;//0x3C;
	TL1=(65535-10000)%256; //TL0T;//0xB0;
	EA=1;  			//系统允许有开放的中断
	ET0=1; 			//允许T0中断
	ET1=1;
	IT1=1;
	IT0=0;
	TR0=1; 			//开启中断，启动定时器
	TR1=0;
}


void alarm(void)
{
	if((shi==nz_shi)&&(fen==nz_fen)&&(miao==0))
	{
	    fAlarm=1;TR1=1;
	}
	if((shi==nz_shi)&&(fen==(nz_fen+1)))
	{
		fAlarm=0;TR1=0;
		buzzer=1;
	}
}


void ZD_baoshi(void)
{
	buzzer=0;
	delay(5);
	buzzer=1;
	bsn++;
	if(bsn==temp_hour)
	{
		baoshi=0;
	}
}

//*******************主函数**************************
//***************************************************
void main()
{
	P1=0xff;
	lcd_init();      //调用液晶屏初始化子函数
	ds1302_init();   //调用DS1302时钟的初始化子函数
	init();          //调用定时计数器的设置子函数
	led=1;           //打开LCD的背光电源
    buzzer=0;		 //蜂鸣器长响一次
    delay(80);
    buzzer=1;
	while(1)  //无限循环下面的语句：
	{		 
   	 	keyscan();      //调用键盘扫描子函数	
         led=led1;
         if(timerOn==1)
			alarm();	//闹钟输出
         if((fen==0)&&(miao==0))
         {
            if(shi>12)
               temp_hour=shi-12;
            else
            {
               if(shi==0)  temp_hour=12;
               else  temp_hour=shi;
            }
            shangyimiao=miao;
            baoshi=1;
         }
         if(baoshi==1)
         {
            ZD_baoshi();
            do 
               keyscan();
            while(shangyimiao==miao);	
            shangyimiao=miao;
         }
    }
}


void timer0() interrupt 1  //取得并显示日历和时间
{
  //读取秒时分周日月年七个数据（DS1302的读寄存器与写寄存器不一样）：
      if(T_NL_NZ!=2)
      {
      miao = BCD_Decimal(read_1302(0x81));
      fen = BCD_Decimal(read_1302(0x83));
      shi  = BCD_Decimal(read_1302(0x85));
      ri  = BCD_Decimal(read_1302(0x87));
      yue = BCD_Decimal(read_1302(0x89));
      nian=BCD_Decimal(read_1302(0x8d));
	//week=BCD_Decimal(read_1302(0x8b));     //不读取，直接通过日期计算得到
      if((led1==0))
      {
         if(temp_miao!=miao)
         {
            temp_miao=miao;

         }
      }


      if(T_NL_NZ==1)								//显示闹钟时间，
      {
         write_1602com(ner);//时间显示固定符号写入位置?
         for(a=0;a<16;a++)
            write_1602dat(NZd[a]);//写显示时间固定符号，两个冒号

         write_nsfm(8,nz_shi);//农历 年
         write_nsfm(11,nz_fen);//农历 月
         write_nsfm(14,nz_miao);//农历 日

         do
            keyscan();
         while(T_NL_NZ==1);

         write_1602com(ner);//时间显示固定符号写入位置，从第2个位置后开始显示
         for(a=0;a<16;a++)
         {
            write_1602dat(qk[a]);//写显示时间固定符号，两个冒号
         }

         write_1602com(er);//时间显示固定符号写入位置，从第2个位置后开始显示
         for(a=0;a<8;a++)
         {
            write_1602dat(tab2[a]);//写显示时间固定符号，两个冒号
         }
      }

      else //if(T_NL_NZ==0)
      {	
         setn=0;
			setNZn=0;
			T_NL_NZ=0;
         //显示秒、时、分数据： 
         
         write_sfm(6,miao);//秒，从第二行第8个字后开始显示（调用时分秒显示子函数）
         write_sfm(3,fen);//分，从第二行第5个字符后开始显示
         write_sfm(0,shi);//小时，从第二行第2个字符后开始显示
      }	
		//显示日、月、年数据：
         write_nyr(9,ri);//日期，从第二行第9个字符后开始显示
	   	write_nyr(6,yue);//月份，从第二行第6个字符后开始显示
         write_nyr(3,nian);//年，从第二行第3个字符后开始显示
         Conver_week(nian,yue,ri);
         write_week(week);
   }
   else 
   {
      
		//if(STPtime!=oSTPtime)
      {
         oSTPtime=STPtime;
         write_1602com(er);
         write_1602dat(STPMin/10+'0');
         write_1602dat(STPMin%10+'0');
         write_1602dat(':');
         write_1602dat(STPSec/10+'0');
         write_1602dat(STPSec%10+'0');
         write_1602dat(':');
         //write_1602dat(STPtime/100+'0');
         write_1602dat(STPtime%100/10+'0');
         write_1602dat(STPtime%10+'0');
      }
      
   }
}



unsigned char count1;

void timer1() interrupt 3  //取得并显示日历和时间
{
	//TH1=(65535-1000)/256; //TH0T;//0x3C;
	//TL1=(65535-1000)%256; //TL0T;//0xB0;

   TH1=(65535-10000)/256; //TH0T;//0x3C;
	TL1=(65535-10000)%256; //TL0T;//0xB0;
   //P37=!P37;
	TR1=1;
   if(fAlarm==1)
   {
      count1++;
      if(count1==10)
      {
         count1=0;
         buzzer=!buzzer;
      }
   }
   if(STPRun==1)
   {
      if(STPtime<100)//if(STPtime<1000)
      {
         STPtime++;
      }
      else
      {
         STPtime=0;
         if(STPSec<59) STPSec++;
         else 
         {
            STPSec=0;
            if(STPMin<59) STPMin++;
            else STPMin=0;
         }
      }
   }
   
}
