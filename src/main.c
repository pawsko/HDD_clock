/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stdio.h"
#include "stm32f10x.h"
//#include "stm32f1xx_nucleo.h"
			
uint32_t SysTick_Config_Mod(uint32_t SysTick_CLKSource, uint32_t Ticks);
//void send_char(char c);
//int __io_putchar(int c);

uint16_t delay = 24;					//to daje 25,48us
uint16_t strob = 7306;
uint32_t step = 0;
uint16_t FullSpin;

//domyœlna godzina od której startuje po reset

uint8_t n = 0;			//H10
uint8_t m = 0;			//H1
uint8_t l = 0;			//M10
uint8_t k = 0;			//M1
uint8_t j = 0;			//S10
uint8_t i = 0;			//S1

char z = 0;			//odpowiedzialny za symbol ":"

uint16_t H10 = 9133;
uint16_t H1 = 8220;
uint16_t HH = 1142;
uint16_t M10 = 6850;
uint16_t M1 = 5937;
uint16_t MM = 9818;
uint16_t S10 = 4567;
uint16_t S1 = 3653;

int main(void)
{
	init_UART();
	init_I2C_for_RTC();
	read_RTC();

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);							//uruchomienie USART
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);		//uruchomienie TIMER 2, 3 i 4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);		//uruchomienie portu A (przerwanie, uart, LED)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);							//uruchomienie portu C Stroby do LED

	GPIO_InitTypeDef gpio;
	//USART_InitTypeDef uart;
	NVIC_InitTypeDef nvic;
	TIM_TimeBaseInitTypeDef tim;
	TIM_ICInitTypeDef ictim;
	TIM_OCInitTypeDef octim;

	GPIO_StructInit(&gpio);						//Przerwanie od tarcza po³o¿enie ZERO
	gpio.GPIO_Pin = GPIO_Pin_1;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);


	//GPIO_StructInit(&gpio);					//UART2 - TX
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio);

	//GPIO_StructInit(&gpio);					//UART2 - RX
	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);

	/*//GPIO_StructInit(&gpio);					//Strob tarczy zegara pierwszy ":"
	gpio.GPIO_Pin = GPIO_Pin_4;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &gpio);*/

	//GPIO_StructInit(&gpio);					//Dioda LED zielona on board
	gpio.GPIO_Pin = GPIO_Pin_5;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &gpio);

												// Diody strobowane LED
	gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &gpio);

	/*USART_StructInit(&uart);
	uart.USART_BaudRate = 115200;
	USART_Init(USART2, &uart);
	USART_Cmd(USART2, ENABLE);*/

	nvic.NVIC_IRQChannel = TIM2_IRQn;							//konfiguraja przerwania ogólnego od Timera 2
	nvic.NVIC_IRQChannelPreemptionPriority = 0;					//liczy czas obrotu tarczy
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	nvic.NVIC_IRQChannel = TIM3_IRQn;							//konfiguraja przerwania ogólnego od Timera 3
	nvic.NVIC_IRQChannelPreemptionPriority = 1;					//strobowanie H10 H1 HH M10
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	nvic.NVIC_IRQChannel = TIM4_IRQn;							//konfiguracja przerwania ogolnego od Timera 4
	nvic.NVIC_IRQChannelPreemptionPriority = 1;					//strobowanie M1 MM S10 S1
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	TIM_TimeBaseStructInit(&tim);								//ten counter liczy czas obrotu tarczy zegara
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Period = 65535; //1us * 65535 = 65ms 535us
	tim.TIM_Prescaler = 64; //fclk=64Mhz/64000=1MHz
	tim.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM2, &tim);

	TIM_ICStructInit(&ictim);									//konfiguracja wejscia do w³aczenia start/stop zegara 2 czyli przejœcie przez zero na tarczy
 	ictim.TIM_Channel = TIM_Channel_2;
	ictim.TIM_ICPolarity = TIM_ICPolarity_Rising;
	ictim.TIM_ICSelection = TIM_ICSelection_DirectTI;
	ictim.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	ictim.TIM_ICFilter = 0;
	TIM_ICInit(TIM2, &ictim);

	tim.TIM_CounterMode = TIM_CounterMode_Up;					//konfiguracja Timera 3 do strobowania LED
	tim.TIM_Prescaler = 64;
	tim.TIM_Period = 65535;
	TIM_TimeBaseInit(TIM3, &tim);

	tim.TIM_CounterMode = TIM_CounterMode_Up;					//konfiguracja Timera 4 do strobowania LED
	tim.TIM_Prescaler = 64;
	tim.TIM_Period = 65535;
	TIM_TimeBaseInit(TIM4, &tim);

	//Konfiguracja kanalu 1 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = H10;                         //H10 taktów
	TIM_OC1Init(TIM3, &octim);                     //Inicjalizacja kanalu 1

	//Konfiguracja kanalu 2 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = H1;                         //H1 taktów
	TIM_OC2Init(TIM3, &octim);                     //Inicjalizacja kanalu 2

	//Konfiguracja kanalu 3 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = HH;                         //HH taktów
	TIM_OC3Init(TIM3, &octim);                     //Inicjalizacja kanalu 3

	//Konfiguracja kanalu 4 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = M10;                         //M10 taktów
	TIM_OC4Init(TIM3, &octim);                     //Inicjalizacja kanalu 4

	//Konfiguracja kanalu 1 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = M1;                         //M1 taktów
	TIM_OC1Init(TIM4, &octim);                     //Inicjalizacja kanalu 1

	//Konfiguracja kanalu 2 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = MM;                         //MM taktów
	TIM_OC2Init(TIM4, &octim);                     //Inicjalizacja kanalu 2

	//Konfiguracja kanalu 3 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = S10;                         //S10 taktów
	TIM_OC3Init(TIM4, &octim);                     //Inicjalizacja kanalu 3

	//Konfiguracja kanalu 4 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = S1;                         //S1 taktów
	TIM_OC4Init(TIM4, &octim);                     //Inicjalizacja kanalu 4

	//TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);
	TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
	TIM_Cmd(TIM2, ENABLE); //Start Timer TIM2

	TIM_ITConfig(TIM3, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE); //Enable interrupt from Timer 2 from Clock Capture 1, 2, 3, 4
	TIM_Cmd(TIM3, ENABLE); //Start Timer TIM3

	TIM_ITConfig(TIM4, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE); //Enable interrupt from Timer 3 from Clock Capture 1, 2, 3, 4
	TIM_Cmd(TIM4, ENABLE); //Start Timer TIM4


	//GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

	if (SysTick_Config_Mod(SysTick_CLKSource_HCLK_Div8, 8000000ul))
		{
			while(1);		// W razie bledu petla nieskonczona
		}

	while(1){
		;
	}

}

/*void send_char(char c)
{
 while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
 USART_SendData(USART2, c);
}

int __io_putchar(int c)
{
    send_char(c);
    return c;

}*/


void SysTick_Handler(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_5)));
  //printf("Krok: %lu\r\n", step);

  //step++;

// n-H10		m-H1	 l-M10	 k-M1		j-S10	i-S1



	if ((j == 5) & (i == 10)) // jeœli jest 60 sekunda
	{
		i = 0;
		j = 0;
		k++;					// dodanie minuty
	}

	if (i == 10)				// jesli jest 10 sekunda
	{
		i = 0;
		j++;					// dodanie dziesi¹tki sekund
	}

	if ((l == 5) & (k == 10))		// jesli jest 60 minuta
	{
		//i = 0;
		//j = 0;
		k = 0;
		l = 0;
		m++;						// dodanie godziny
	}

	if (k == 10)					// jesli jest 10 numuta
	{
		k = 0;
		l++;						// dodanie dziesi¹tki minut
	}

	if ((n == 2) & (m == 4))		// jesli jest 24 godzina zeruj wszystko
	{
		//i = 0;
		//j = 0;
		//k = 0;
		//l = 0;
		m = 0;
		n = 0;
	}

	if (m == 10)					// jesli jest 10 godzina
	{
		m = 0;
		n++;						// dodaj dziesi¹tkê godzin
	}



	switch (n){							//godziny dziesi¹tki
	case 0: H10 = 9133;
	break;
	case 1: H10 = 10047;
	break;
	case 2: H10 = 0;
	break;
	case 3: H10 = 913;
	break;
	case 4: H10 = 1827;
	break;
	case 5: H10 = 3653;
	break;
	case 6: H10 = 4567;
	break;
	case 7: H10 = 5480;
	break;
	case 8: H10 = 6393;
	break;
	case 9: H10 = 7307;
	break;
	default:
	break;
	}

	switch (m){					//godziny jednostki
	case 0: H1 = 8220;
	break;
	case 1: H1 = 9133;
	break;
	case 2: H1 = 10047;
	break;
	case 3: H1 = 0;
	break;
	case 4: H1 = 913;
	break;
	case 5: H1 = 2740;
	break;
	case 6: H1 = 3653;
	break;
	case 7: H1 = 4567;
	break;
	case 8: H1 = 5480;
	break;
	case 9: H1 = 6393;
	break;
	default:
	break;
	}

	switch (l){								//minuty dziesi¹tki
	case 0: M10 = 6850;
	break;
	case 1: M10 = 7763;
	break;
	case 2: M10 = 8677;
	break;
	case 3: M10 = 9590;
	break;
	case 4: M10 = 10503;
	break;
	case 5: M10 = 1370;
	break;
	case 6: M10 = 2283;
	break;
	case 7: M10 = 3197;
	break;
	case 8: M10 = 4110;
	break;
	case 9: M10 = 5023;
	break;
	default:
	break;
	}

	switch (k){						//minuty jednostki
	case 0: M1 = 5937;
	break;
	case 1: M1 = 6850;
	break;
	case 2: M1 = 7763;
	break;
	case 3: M1 = 8677;
	break;
	case 4: M1 = 9590;
	break;
	case 5: M1 = 457;
	break;
	case 6: M1 = 1370;
	break;
	case 7: M1 = 2283;
	break;
	case 8: M1 = 3197;
	break;
	case 9: M1 = 4110;
	break;
	default:
	break;
	}

	switch (j){						//sekundy dziesi¹tki
	case 0: S10 = 4567;
	break;
	case 1: S10 = 5480;
	break;
	case 2: S10 = 6393;
	break;
	case 3: S10 = 7307;
	break;
	case 4: S10 = 8220;
	break;
	case 5: S10 = 10047;
	break;
	case 6: S10 = 0;
	break;
	case 7: S10 = 913;
	break;
	case 8: S10 = 1827;
	break;
	case 9: S10 = 2740;
	break;
	default:
	break;
	}


	switch (i){							//sekundy jednostki
	case 0: S1 = 3653;
	break;
	case 1: S1 = 4567;
	break;
	case 2: S1 = 5480;
	break;
	case 3: S1 = 6393;
	break;
	case 4: S1 = 7307;
	break;
	case 5: S1 = 9133;
	break;
	case 6: S1 = 10047;
	break;
	case 7: S1 = 0;
	break;
	case 8: S1 = 913;
	break;
	case 9: S1 = 1827;
	break;
	default:
	break;
	}


	if (z == 0)
	{
		TIM_ITConfig(TIM4, TIM_IT_CC2, DISABLE); //Disable interrupt from Timer 4 from Clock Capture 2
		TIM_ITConfig(TIM3, TIM_IT_CC3, DISABLE); //Disable interrupt from Timer 3 from Clock Capture 3
		z++;
	}
	else
	{
		TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE); //Enable interrupt from Timer 4 from Clock Capture 2
		TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE); //Enable interrupt from Timer 3 from Clock Capture 3
		z = 0;
	}

	TIM_OCInitTypeDef octim;

	//Konfiguracja kanalu 1 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = H10;                         //H10 taktów
	TIM_OC1Init(TIM3, &octim);                     //Inicjalizacja kanalu 1

	//Konfiguracja kanalu 2 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = H1;                          //H1 taktów
	TIM_OC2Init(TIM3, &octim);                     //Inicjalizacja kanalu 2

	//Konfiguracja kanalu 3 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = HH;                          //HH taktów
	TIM_OC3Init(TIM3, &octim);                     //Inicjalizacja kanalu 3

	//Konfiguracja kanalu 4 Timer 3
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = M10;                         //M10 taktów
	TIM_OC4Init(TIM3, &octim);                     //Inicjalizacja kanalu 4

	//Konfiguracja kanalu 1 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = M1;                          //M1 taktów
	TIM_OC1Init(TIM4, &octim);                     //Inicjalizacja kanalu 1

	//Konfiguracja kanalu 2 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = MM;                          //MM taktów
	TIM_OC2Init(TIM4, &octim);                     //Inicjalizacja kanalu 2


	//Konfiguracja kanalu 3 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = S10;                         //HH taktów
	TIM_OC3Init(TIM4, &octim);                     //Inicjalizacja kanalu 3

	//Konfiguracja kanalu 4 Timer 4
	octim.TIM_OCMode = TIM_OCMode_Timing;          //tryb pracy kanalu
	octim.TIM_OutputState = TIM_OutputState_Enable;//wlaczenie generowania sygnalu na wyjsciu licznika
	octim.TIM_Pulse = S1;                          //S10 taktów
	TIM_OC4Init(TIM4, &octim);                     //Inicjalizacja kanalu 4

	printf ("Time: %d%d:%d%d:%d%d\r\n", n, m, l, k, j, i);

	i++;

}

uint32_t  SysTick_Config_Mod(uint32_t SysTick_CLKSource, uint32_t Ticks)
{
  //inicjalizacja licznika SysTick
  //zastepuje funkcje z bibliotek STM w zwiazku z bledem w funcji SysTick_Config
  uint32_t Settings;

  assert_param(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource));

  if (Ticks > SysTick_LOAD_RELOAD_Msk)  return (1);             //Kontrola, czy wartosc poczatkowa nie przekracza max

  SysTick->LOAD = (Ticks & SysTick_LOAD_RELOAD_Msk) - 1;        //Ustaw wartosc poczatkowa licznika
  NVIC_SetPriority (SysTick_IRQn, 0);                           //Ustaw priorytet przerwania
  SysTick->VAL  = 0;                                            //Ustaw wartosc aktualna licznika
  Settings=SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;  //Ustaw flagi wlaczenia SysTick IRQ  i samego licznika
  if (SysTick_CLKSource == SysTick_CLKSource_HCLK){             //Wybierz flage ustawien zrodla sygnalu zegarowego
    Settings |= SysTick_CLKSource_HCLK;
  } else {
    Settings &= SysTick_CLKSource_HCLK_Div8;
  }
  SysTick->CTRL = Settings;                                     //Zapisz ustawienia do rejestru sterujacego SysTick (i wlacz licznik)
  return (0);
}

void TIM2_IRQHandler(void)
{
if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Trigger);
	}
if (TIM_GetITStatus(TIM2, TIM_IT_CC2) == SET)	 // Jezeli przerwanie od kanalu 2
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);	 // Zeruj flage przerwania

    FullSpin = TIM_GetCapture2(TIM2);				 // Odczytaj wartosc rejetru CCR kanalu 2
    TIM_SetCounter(TIM2, 0);
    TIM_SetCounter(TIM3, 0);							//zerowanie licznika 2
    TIM_SetCounter(TIM4, 0);
    GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
    //GPIO_SetBits(GPIOC, GPIO_Pin_2 | GPIO_Pin_5);
    //GPIO_WriteBit(GPIOC, GPIO_Pin_2, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_2)));  	//LED4
  	//GPIO_WriteBit(GPIOC, GPIO_Pin_5, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_5)));  	//LED4
  }
}

void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)	 // Jezeli przerwanie od zdarzenia w liczniku 3
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	 // Zeruj flage przerwania
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)	{     //osiagnienie wartosci z CC1
  	TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
  	GPIO_SetBits(GPIOC, GPIO_Pin_0);
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)	{     //osiagnienie wartosci z CC2
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
    GPIO_SetBits(GPIOC, GPIO_Pin_1);
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)	{     //osiagnienie wartosci z CC3
  	TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
  	GPIO_SetBits(GPIOC, GPIO_Pin_2);
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)	{     //osiagnienie wartosci z CC4
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
    GPIO_SetBits(GPIOC, GPIO_Pin_3);
  }
}

void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)	 // Jezeli przerwanie od zdarzenia w liczniku 4
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);	 // Zeruj flage przerwania
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)	{     //osiagnienie wartosci z CC1
  	TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
  	GPIO_SetBits(GPIOC, GPIO_Pin_4);
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)	{     //osiagnienie wartosci z CC2
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);
    GPIO_SetBits(GPIOC, GPIO_Pin_5);
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)	{     //osiagnienie wartosci z CC3
  	TIM_ClearITPendingBit(TIM4, TIM_IT_CC3);
  	GPIO_SetBits(GPIOC, GPIO_Pin_6);
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC4) != RESET)	{     //osiagnienie wartosci z CC4
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);
    GPIO_SetBits(GPIOC, GPIO_Pin_7);
  }
}
