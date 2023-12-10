#include "TM4C129.h"
#include <stdio.h>
#include <stdlib.h>
//#include "Lib4.h"
#include "ES_Lib.h"
#include <string.h>
#define MAX0 60
#define MAX 10
#define MAX2 6
#define MAX3 2
#define MAX4 24
#define MAX5 12
#define CMAX 3

//For SPI
void setupPA();
void setupPD(); //SPI 2
void setupPJ();
void setupSSI();
void Setup_NVIC();
void start_timer_0A();
void start_timer_1A();
void start_timer_2A();
void start_timer_3A(); //for 1kHz
void delayTIA();
void use_clock(int toggle,int hrtoggle);
void alarm_setting(int toggle,int hrtoggle);
void algorithim();

//For Keypad
void setupGPIOKE();
void GPIOE_Handler();
void CheckBtn(int n);
void keypad_handler();

//For LED
void setupGPIOF();
void AlarmCol(int v);
void AlarmCheck();

//For Buzzer
void setupGPIOL();
void Buzz_Buzzer();
//For Joystick
void joychecker();


//For UART
void print_time();
void setupADC2();
void checker2();

//Analog (Potentiometer)
void setupGPIOE();
void setupADC();
void checker();


int digit[MAX]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90}; //0,1,2,3,4,5,6,7,8,9
int digitdot[MAX]={64,121,36,48,25,18,0x2,120,0,16}; //0,1,2,3,4,5,6,7,8,9
int digit12[MAX5]={0xa4,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xc0,0xf9}; //2,1,2,3,4,5,6,7,8,9,0,1
int chars[CMAX]={0xBF,0xff,0x7f};//'-','','.'
//Global counts for timer mode
int x = 0; //seconds
int x2 = 0;
int x3=0;
int x4=0;
int x5=13; //hrs

//Global counts for alarm mode
int ax = 10;
int ax2=0;
int ax3=0;
int ax4=0;
int ax5=13;

int counter=0;
int canclick=1;
int candash=0;
int toggle=1; //1 for timer mode | 0 for timer setting mode
int hrtoggle=1; //1 for 24 hr time | 0 for 12 hr
int soundalarm=0;
int alarmtoggle=0; //for alternating blue and red alarm
int val=0; //For button Click
int canchangemins=0;
int canchangehrs=0;
int usejoy=1;

int main()
{
	setSysClk(16E6);
	__disable_irq();
	//For SSI
	setupPJ();
	//setupPA();
	setupPD();
	setupSSI();
	start_timer_0A();
	start_timer_1A();
	start_timer_2A();
	start_timer_3A();
	//For Keypad
	setupGPIOKE();
	//For LED
	setupGPIOF();
	AlarmCol(0);
	//For Buzzer
	setupGPIOL();
	//For Potentiometer (Analog)
	ES_Serial(0, "115200,8,N,1");
	//ES_printf("Hi\r\n");
	setupGPIOE();
	setupADC();
	setupADC2();
	//For Interrupts
	Setup_NVIC();
	__enable_irq();
	while(1)
	{
		delayTIA();
		keypad_handler();
		joychecker();
		if (toggle)
		{
			use_clock(toggle,hrtoggle);
		}
		else
		{
			alarm_setting(toggle,hrtoggle);
		}
	}
	return 0;
}

//For UART
void print_time()
{
	ES_printf("%i%i-%i%i-%i%i\r\n",x5/10,x5%10,x3/10,x3%10,x/10,x%10);
	/*serialWrite((char)x5/10+48); //for hrs
	serialWrite((char)x5%10+48); //for hrs
	serialWrite('-'); //-
	serialWrite((char)x3/10+48); //for mins
	serialWrite((char)x3%10+48); //for mins
	serialWrite('-'); //-
	serialWrite((char)x/10+48); //for seconds
	serialWrite((char)x%10+48); //for seconds
	serialWriteString("\r\n"); //new line*/
}

//Port Setups
void setupPA()
{
	SYSCTL->RCGCGPIO|=(1<<0);
	while((SYSCTL->PRGPIO&(1<<0))!=(1<<0))
	{
		__ASM("nop");
	}
	GPIOA_AHB->DIR|=(1<<2)|(1<<3)|(1<<4); //Pin 2 = Clock | Pin 3 = Latch | Pin 4 = Data
	//GPIOA_AHB->DIR&=~(1<<5);
	GPIOA_AHB->AFSEL|=(1<<2)|(1<<4);
	GPIOA_AHB->AFSEL&=~(1<<3);
	GPIOA_AHB->PCTL=(0xFFFF00);
	GPIOA_AHB->DEN|=(1<<2)|(1<<3)|(1<<4);
	GPIOA_AHB->PUR|=(1<<2);
}

void setupPD() //SPI 2
{
	SYSCTL->RCGCGPIO|=(1<<3);
	while((SYSCTL->PRGPIO&(1<<3))!=(1<<3))
	{
		__ASM("nop");
	}
	GPIOD_AHB->DIR|=(1<<0)|(1<<1)|(1<<3); //Pin 0 = Latch | Pin 1 = Data (Tx) | Pin 3 = Clock
	GPIOD_AHB->AFSEL|=(1<<1)|(1<<3);
	GPIOD_AHB->AFSEL&=~(1<<0);
	GPIOD_AHB->PCTL=(0xFFF0);
	GPIOD_AHB->DEN|=(1<<0)|(1<<1)|(1<<2);
	GPIOD_AHB->PUR|=(1<<3);
}

void setupPJ()
{
	SYSCTL->RCGCGPIO|=(1<<8);
	while((SYSCTL->PRGPIO&(1<<8))!=(1<<8))
	{
		__ASM("nop");
	}
	GPIOJ_AHB->DIR&=~(1<<0)|(1<<1);
	GPIOJ_AHB->AFSEL&=~(1<<0)&~(1<<1);
	GPIOJ_AHB->AMSEL&=~(1<<0)&~(1<<1);
	GPIOJ_AHB->PUR|=(1<<0)|(1<<1);
	GPIOJ_AHB->DEN|=(1<<0)|(1<<1);
	
	GPIOJ_AHB->ICR |= (1<<0)|(1<<1);
	GPIOJ_AHB->IM|=(1<<0)|(1<<1);
}

void setupSSI()
{
	SYSCTL->RCGCSSI|=(1<<2);
	while((SYSCTL->PRSSI&(1<<2))!=(1<<2))
	{
		__ASM("nop");
	}
	SSI2->CR1&=~(1<<1);
	SSI2->CR1&=~(1<<2);
	SSI2->CPSR=40;
	SSI2->CR0&=~(0xFFFF);
	SSI2->CR0|=0x7;
	SSI2->CR1|=(1<<1);
	SSI2->IM|=(1<<3);
}

void Setup_NVIC()
{
	/*//For SSI
	NVIC->IP[7]=3;
	NVIC->ISER[1]|=(1<<7);*/
	//For GPIOA
	NVIC->IP[51]=3;
	NVIC->ISER[1]|=(1<<19);
	//For Timer1A
	NVIC->IP[21]=3;
	NVIC->ISER[0]|=(1<<21);
	//For Timer2A
	NVIC->IP[23]=3;
	NVIC->ISER[0]|=(1<<23);
	//For Timer3A
	NVIC->IP[35]=3;
	NVIC->ISER[1]|=(1<<3);
	//For Keypad
	//NVIC->IP[4]=3;
	//NVIC->ISER[0]|=(1<<4);
}

void start_timer_0A() //for spi display
{
	SYSCTL->RCGCTIMER|=(1<<0);
	while((SYSCTL->PRTIMER&(1<<0))!=(1<<0))
	{
		__ASM("nop");
	}
	TIMER0->CTL&=~(1<<0);
	TIMER0->CFG=0x4;
	TIMER0->TAMR&=~0x03;
	TIMER0->TAMR|=0x02;
	TIMER0->TAILR = 59260;
	TIMER0->TAPR=2; //30
	TIMER0->ICR|=(1<<0);
	
	//TIMER0->TAPMR|=(1<<1);
	TIMER0->CTL|=(1<<0);
	
}

void start_timer_1A() //for 1 second
{
	SYSCTL->RCGCTIMER|=(1<<1);
	while((SYSCTL->PRTIMER&(1<<1))!=(1<<1))
	{
		__ASM("nop");
	}
	TIMER1->CTL&=~(1<<0);
	TIMER1->CFG=0x4;
	TIMER1->TAMR&=~0x03;
	TIMER1->TAMR|=0x02;
	TIMER1->TAILR = 0xFFFF; //59260
	TIMER1->TAPR=243; //30
	TIMER1->ICR|=(1<<0);
	TIMER1->IMR|=(1<<0);
	//TIMER1->TAPMR|=(1<<1);
	TIMER1->CTL|=(1<<0);
	
}

void start_timer_2A() //for 0.5 seconds
{
	SYSCTL->RCGCTIMER|=(1<<2);
	while((SYSCTL->PRTIMER&(1<<2))!=(1<<2))
	{
		__ASM("nop");
	}
	TIMER2->CTL&=~(1<<0);
	TIMER2->CFG=0x4;
	TIMER2->TAMR&=~0x03;
	TIMER2->TAMR|=0x02;
	TIMER2->TAILR = 0xFFFF; //59260
	TIMER2->TAPR=121; //30
	TIMER2->ICR|=(1<<0);
	TIMER2->IMR|=(1<<0);
	//TIMER2->TAPMR|=(1<<1);
	TIMER2->CTL|=(1<<0);
	
}

void start_timer_3A() //for 1kHz
{
	SYSCTL->RCGCTIMER|=(1<<3);
	while((SYSCTL->PRTIMER&(1<<3))!=(1<<3))
	{
		__ASM("nop");
	}
	TIMER3->CTL&=~(1<<0);
	TIMER3->CFG=0x4;
	TIMER3->TAMR&=~0x03;
	TIMER3->TAMR|=0x02;
	TIMER3->TAILR = 0x5001; //5001
	TIMER3->TAPR=4; //4
	TIMER3->ICR|=(1<<0);
	TIMER3->IMR|=(1<<0);
	//TIMER3->TAPMR|=(1<<1);
	TIMER3->CTL|=(1<<0);
	
}
void delayTIA()
{
	//TIMER0->CTL|=(1<<0);
	while((TIMER0->RIS&(1<<0))==0)
	{
		__ASM("nop");
	}
	TIMER0->ICR|=0x01;
}

void algorithim()
{
	if (x>=MAX0)
	{
		x=0;
		x3++;
	}
	if (x3==MAX0)
	{
		x3=0;
		x5++;
	}
	if (x5==MAX4)
	{
		x5=0;
	}
	
	if (ax>=MAX0)
	{
		ax=0;
		ax3++;
	}
	if (ax3==MAX0)
	{
		ax3=0;
		ax5++;
	}
	if (ax5==MAX4)
	{
		ax5=0;
	}
}


void use_clock(int toggle,int hrtoggle)
{
		if ((SSI2->SR&(1<<1))==(1<<1))
		{
			if(counter==0) //Digit
			{
				if (!hrtoggle&&x5>12) //12 hr pm
				{
					SSI2->DR=(digitdot[x%10]);
				}
				else //12 hr am
				{
					SSI2->DR=(digit[x%10]);
				}
			}
			else if (counter==1)
			{
				SSI2->DR=(digit[x/10]);
			}
			else if(counter==3)
			{
				SSI2->DR=(digit[x3%10]);
			}
			else if(counter==4)
			{
				SSI2->DR=(digit[x3/10]);
			}
			else if(counter==6)
			{
				if (x5==MAX4)
				{
					SSI2->DR=(digit[0]);
				}
				else if (hrtoggle) //for 24 hr
				{
					SSI2->DR=(digit[x5%10]);
				}
				else //for 12 hr
				{
					SSI2->DR=(digit12[x5%12]);
				}
			}
			else if(counter==7)
			{
				if (hrtoggle) //for 24 hr
				{
					SSI2->DR=(digit[x5/10]);
				}
				else //for 12 hr
				{
					if (x5>9&&x5<13||x5>21||x5==0)
					{
						SSI2->DR=(digit12[1]);
					}
					else
					{
						SSI2->DR=(digit12[10]);
					}
				}
			}
			else if(counter==5||counter==2)//- char
			{
				if (candash)
				{
					SSI2->DR=(chars[0]);
				}
				else
				{
					SSI2->DR=(chars[1]);
				}
			}
			else//0 digit
			{
				SSI2->DR=(digit[0]);
			}
			counter++;
			if (counter==9) //latch
			{
				counter=0;
				GPIOD_AHB->DATA|=(1<<0);
			}
			else
			{
				GPIOD_AHB->DATA&=~(1<<0);
			}
		}
}


void alarm_setting(int toggle,int hrtoggle)
{
		if ((SSI2->SR&(1<<1))==(1<<1))
		{
			if(counter==0) //Digit
			{
				if (!hrtoggle&&ax5>12) //12 hr pm
				{
					SSI2->DR=(digitdot[ax%10]);
				}
				else //12 hr am
				{
					SSI2->DR=(digit[ax%10]);
				}
			}
			else if (counter==1)
			{
				SSI2->DR=(digit[ax/10]);
			}
			else if(counter==3)
			{
				SSI2->DR=(digit[ax3%10]);
			}
			else if(counter==4)
			{
				SSI2->DR=(digit[ax3/10]);
			}
			else if(counter==6)
			{
				if (ax5==MAX4)
				{
					SSI2->DR=(digit[0]);
				}
				else if (hrtoggle) //for 24 hr
				{
					SSI2->DR=(digit[ax5%10]);
				}
				else //for 12 hr
				{
					SSI2->DR=(digit12[ax5%12]);
				}
			}
			else if(counter==7)
			{
				if (hrtoggle) //for 24 hr
				{
					SSI2->DR=(digit[ax5/10]);
				}
				else //for 12 hr
				{
					if (ax5>9&&ax5<13||ax5>21||ax5==0)
					{
						SSI2->DR=(digit12[1]);
					}
					else
					{
						SSI2->DR=(digit12[10]);
					}
				}
			}
			else if(counter==5||counter==2)//- char
			{
				if (candash)
				{
					SSI2->DR=(chars[0]);
				}
				else
				{
					SSI2->DR=(chars[1]);
				}
			}
			else//0 digit
			{
				SSI2->DR=(digit[0]);
			}
			counter++;
			if (counter==9)
			{
				counter=0;
				GPIOD_AHB->DATA|=(1<<0);
			}
			else
			{
				GPIOD_AHB->DATA&=~(1<<0);
			}
		}
}

void GPIOJ_Handler()
{
	if (!canclick)
	{
		GPIOJ_AHB->ICR|=(1<<0)|(1<<1);
		return;
	}
	canclick=0;
	if (GPIOJ_AHB->MIS&(1<<0))
	{
		hrtoggle^=(1<<0);
		//toggle^=(1<<0); //for changing to timer mode
	}
	else if (GPIOJ_AHB->MIS&(1<<1))
	{
		toggle^=(1<<0);
		/*if (!toggle) //for incrementing seconds
		{
			x++;
		}
		*/
	}
	GPIOJ_AHB->ICR|=(1<<0)|(1<<1);
	canclick=1;
}


void TIMER1A_Handler()
{
	TIMER1->ICR|=(1<<0);
	x++;
	algorithim();
	print_time();
	AlarmCheck();
	if (soundalarm)
	{
			AlarmCol(1);
	}
}

void TIMER2A_Handler()
{
	canclick=1;
	TIMER2->ICR|=(1<<0);
	checker();
	checker2();
	candash^=(1<<0);
}

void TIMER3A_Handler()
{
	TIMER3->ICR|=(1<<0);
	if (soundalarm)
	{
		Buzz_Buzzer();
	}
}


//Keypad
void setupGPIOKE()
{
	SYSCTL->RCGCGPIO|=(1<<4)|(1<<9);
	while(((SYSCTL->PRGPIO&(1<<4))!=(1<<4))&&((SYSCTL->PRGPIO&(1<<9))!=(1<<9)))
	{
		__ASM("nop");
	}
	//Configure Port K for Output
	GPIOK->DIR|=(1<<0)|(1<<1)|(1<<2);
	GPIOK->AFSEL&=~(1<<0)&~(1<<1)&~(1<<2);
	GPIOK->DR2R|=(1<<0)|(1<<1)|(1<<2);
	GPIOK->DEN|=(1<<0)|(1<<1)|(1<<2);
	GPIOK->AMSEL&=~(1<<0)&~(1<<1)&~(1<<2);
	//Configure Port E for Input
	GPIOE_AHB->DIR&=~(1<<0)&~(1<<1)&~(1<<2)&~(1<<3);
	GPIOE_AHB->AFSEL&=~(1<<0)&~(1<<1)&~(1<<2)&~(1<<3);
	GPIOE_AHB->PDR|=(1<<0)|(1<<1)|(1<<2)|(1<<3);
	GPIOE_AHB->DEN|=(1<<0)|(1<<1)|(1<<2)|(1<<3);
	GPIOE_AHB->AMSEL&=~(1<<0)&~(1<<1)&~(1<<2)&~(1<<3);
	
	GPIOE_AHB->IM&=~(0xFF);
	GPIOE_AHB->IS&=~(1<<0);
	GPIOE_AHB->IEV|=(1<<0);
	GPIOE_AHB->IBE|=(1<<0);
	GPIOE_AHB->IM|=(1<<0);
}

void CheckBtn(int n)
{
	val=GPIOE_AHB->DATA;
	if (canclick)
	{
		if ((GPIOK->DATA&(n))==((1<<0))) ////Column 1
		{
			if (val==(1<<0)) //change to timer mode
			{
				toggle|=(1<<0);
			}
			else if(val==(1<<1))//change hrs
			{
				canchangemins=0;
				canchangehrs=1;
			}
			else if (val==(1<<2))//use joystick
			{
				usejoy=1;
			}
			else if (val==(1<<3)) //accept hrs and mins
			{
				canchangehrs=0;
				canchangemins=0;
			}
		}
		else if ((GPIOK->DATA&(n))==((1<<1))) ////Column 2
		{
			if (val==(1<<3)) //toggle hrs
			{
				canclick=0;
				hrtoggle^=(1<<0);
			}
			else if(val == (1<<1)) //change mins
			{
				canchangehrs=0;
				canchangemins=1;
			}
			else if(val==(1<<2)) //use potentiometer
			{
				usejoy=0;
			}
		}
		else if ((GPIOK->DATA&(n))==((1<<2))) //Column 3
		{
			if ((val)==((1<<0))) //change to alarm mode
			{
				toggle&=~(1<<0);
			}
			else if (val == (1<<1)) //increment seconds
			{
				canclick=0;
				if (toggle)
				{
					x++;
				}
				else
				{
					ax++;
				}
			}
			else if ((val)==((1<<3))) //Turn Off Alarm
			{
				soundalarm=0;
				AlarmCol(0);
			}
		}
	}
}


void keypad_handler()
{
		GPIOK->DATA|=(1<<0);
		CheckBtn((1<<0));
		GPIOK->DATA&=~(1<<0);
	
		GPIOK->DATA|=(1<<1);
		CheckBtn((1<<1));
		GPIOK->DATA&=~(1<<1);
	
		GPIOK->DATA|=(1<<2);
		CheckBtn((1<<2));
		GPIOK->DATA&=~(1<<2);
}

void GPIOE_Handler()
{
	GPIOE_AHB->ICR|=(1<<0);
	//canclick=0;
	//CheckBtn(GPIOK->DATA);
	//GPIOF_AHB->DATA^=(1<<0);
}
//For LED
void setupGPIOF()
{
	SYSCTL->RCGCGPIO|=(1<<5);
	while(((SYSCTL->PRGPIO&(1<<5))!=(1<<5)))
	{
		__ASM("nop");
	}
	GPIOF_AHB->DIR|=(1<<1)|(1<<2)|(1<<3);
	GPIOF_AHB->AFSEL&=~(1<<1)&~(1<<2)&~(1<<3);
	GPIOF_AHB->DEN|=(1<<1)|(1<<2)|(1<<3);
	GPIOF_AHB->AMSEL&=~(1<<1)&~(1<<2)&~(1<<3);
}


void AlarmCol(int v)
{
	//1 for green | 2 for red | 3 for blue
	if (!v)
	{
		GPIOF_AHB->DATA&=~(1<<3);
		GPIOF_AHB->DATA|=(1<<1)|(1<<2);
	}
	else if (alarmtoggle)
	{
		alarmtoggle=0;
		GPIOF_AHB->DATA|=(1<<2);
		GPIOF_AHB->DATA&=~(1<<1)&~(1<<3);
	}
	else
	{
		alarmtoggle=1;
		GPIOF_AHB->DATA|=(1<<3);
		GPIOF_AHB->DATA&=~(1<<1)&~(1<<2);
	}
}

void AlarmCheck()
{
	if (x==ax&&x2==ax2&&x3==ax3&&x4==ax4&&x5==ax5)
	{
		soundalarm=1;
	}
}

//For Buzzer && Joystick
void setupGPIOL()
{
	SYSCTL->RCGCGPIO|=(1<<10);
	while(((SYSCTL->PRGPIO&(1<<10))!=(1<<10)))
	{
		__ASM("nop");
	}
	GPIOL->DIR|=(1<<4);
	GPIOL->DIR&=~(1<<1)&~(1<<2)&~(1<<3);
	GPIOL->PUR|=(1<<3);
	GPIOL->AFSEL&=~(1<<1)&~(1<<2)&~(1<<3)&~(1<<4);
	GPIOL->DEN|=(1<<1)|(1<<2)|(1<<3)|(1<<4);
	GPIOL->AMSEL&=~(1<<1)&~(1<<2)&~(1<<3)&~(1<<4);
}

void Buzz_Buzzer()
{
	GPIOL->DATA^=(1<<4);
}

void joychecker()
{
	if ((GPIOL->DATA&(1<<3))!=(1<<3)) //switch pressed | increment seconds
	{
		if (canclick)
		{
			canclick =0;
			if (toggle) //for time mode
			{
				x++;
			}
			else //for alarm mode
			{
				ax++;
			}
		}
	}
}

void setupADC2()
{
	SYSCTL->RCGCADC|=(1<<1);
	while((SYSCTL->PRADC&(1<<1))!=(1<<1))
	{
		__ASM("nop");
	}
	ADC1->PC|=(0x1);
	ADC1->ACTSS&=~(1<<3);//using sample sequencer 3
	//ADC0->SSPRI|=(1<<3); //Don't Need! Smallest num = highest priority
	ADC1->EMUX&=~(0xF000); //clear for software
	ADC1->SSMUX3 = 8; //PE0 for AIN3 | PE4 For AIN9 |PE5 For AIN8
	ADC1->SSCTL3|=(1<<1)|(1<<2);
	ADC1->SSCTL3&=~(1<<3);
	ADC1->ACTSS|=(1<<3);
	ADC1->CC=1;
}

void checker2()
{
	ADC1->PSSI|=(1<<3);
	while((ADC1->RIS&(1<<3))==0)
	{
		__ASM("nop");
	}
	int thex = ADC1->SSFIFO3/174;
	//int thex2 = ADC1->SSFIFO3/174; //for hrs
	//ES_printf("%i\n",thex);
	if (canchangemins&&usejoy)
	{
		if (toggle)
		{
			if (thex>=20)
			{
				if (x3<MAX0-1)
				{
					x3++;
				}
			}
			else if (thex<=3)
			{
				if (x3>0)
				{
					x3--;
				}
			}
		}
		else
		{
			if (thex>=20)
			{
				if (ax3<MAX0-1)
				{
					ax3++;
				}
			}
			else if (thex<=3)
			{
				if (ax3>0)
				{
					ax3--;
				}
			}
		}
	}
	else if (canchangehrs&&usejoy)
	{
		//ES_printf("%i\n",thex2);
		if (toggle)
		{
			if (thex>=20)
			{
				if (x5<MAX4-1)
				{
					x5++;
				}
			}
			else if (thex<=3)
			{
				if (x5>0)
				{
					x5--;
				}
			}
		}
		else
		{
			if (thex>=20)
			{
				if (ax5<MAX4-1)
				{
					ax5++;
				}
			}
			else if (thex<=3)
			{
				if (ax5>0)
				{
					ax5--;
				}
			}
		}
	}
	ADC1->ISC=(1<<3);
}

//For Analog
void setupGPIOE()
{
	GPIOE_AHB->DIR&=~(1<<4)&~(1<<5);
	GPIOE_AHB->AMSEL|=(1<<4)|(1<<5);
	GPIOE_AHB->AFSEL|=(1<<4)|(1<<5);
	GPIOE_AHB->DEN&=~(1<<4)&~(1<<5);
}
void setupADC()
{
	SYSCTL->RCGCADC|=(1<<0);
	while((SYSCTL->PRADC&(1<<0))!=(1<<0))
	{
		__ASM("nop");
	}
	ADC0->PC|=(0x1);
	ADC0->ACTSS&=~(1<<3);//using sample sequencer 3
	//ADC0->SSPRI|=(1<<3); //Don't Need! Smallest num = highest priority
	ADC0->EMUX&=~(0xF000); //clear for software
	ADC0->SSMUX3 = 9; //PE0 for AIN3 | PE4 For AIN9
	ADC0->SSCTL3|=(1<<1)|(1<<2);
	ADC0->SSCTL3&=~(1<<3);
	ADC0->ACTSS|=(1<<3);
	ADC0->CC=1;
}

void checker()
{
	ADC0->PSSI|=(1<<3);
	while((ADC0->RIS&(1<<3))==0)
	{
		__ASM("nop");
	}
	int thex = ADC0->SSFIFO3/69; //for mins
	int thex2 = ADC0->SSFIFO3/174; //for hrs
	if (canchangemins&&!usejoy)
	{
		//ES_printf("%i\n",thex);
		if (toggle)
		{
			x3=thex;
		}
		else
		{
			ax3=thex;
		}
	}
	else if (canchangehrs&&!usejoy)
	{
		//ES_printf("%i\n",thex2);
		if (toggle)
		{
			x5=thex2;
		}
		else
		{
			ax5=thex2;
		}
	}
	ADC0->ISC=(1<<3);
}

/*void SSI0_Handler()
{
	SSI0->ICR|=(1<<3);
	if (soundalarm)
	{
			AlarmCol(1);
	}
}*/
