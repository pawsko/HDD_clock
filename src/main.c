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

//dynamic rotated count

uint32_t SysTick_Config_Mod(uint32_t SysTick_CLKSource, uint32_t Ticks);

uint16_t delay = 24;    //this value gives 25,48us
uint16_t strob = 7306;
uint16_t SysTick1ms = 0;
uint8_t SysTick100ms = 0;
uint32_t step = 0;
uint16_t FullSpin;
uint8_t Task100ms = 0;    //0-ready to do; 1-done
uint8_t Task1ms = 0;    //0-ready to do; 1-done
uint8_t NewSpin = 0;    //0-end spin detection

//default displayed hour after reset

uint8_t h_decade = 0;    //H10
uint8_t h_unit = 0;    //H1
uint8_t m_decade = 0;    //M10
uint8_t m_unit = 0;    //M1
uint8_t s_decade = 0;    //S10
uint8_t s_unit = 0;    //S1

char colon_toggle = 0;    //respond for symbol ":"

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

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);    //start USART
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);    //TIMER 2, 3 i 4 started
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);    //port A started (interrupt, uart, LED)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);    //Port C started: Strobs for LED

  GPIO_InitTypeDef gpio;
  //USART_InitTypeDef uart;
  NVIC_InitTypeDef nvic;
  TIM_TimeBaseInitTypeDef tim;
  TIM_ICInitTypeDef ictim;
  TIM_OCInitTypeDef octim;

  GPIO_StructInit(&gpio);    //Interrupt from  clock table ZERO position
  gpio.GPIO_Pin = GPIO_Pin_1;
  gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &gpio);

  //GPIO_StructInit(&gpio);    //UART2 - TX
  gpio.GPIO_Pin = GPIO_Pin_2;
  gpio.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &gpio);

  //GPIO_StructInit(&gpio);    //UART2 - RX
  gpio.GPIO_Pin = GPIO_Pin_3;
  gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &gpio);

  //GPIO_StructInit(&gpio);    //Green LED on Nucleo board "hearbeat"
  gpio.GPIO_Pin = GPIO_Pin_5;
  gpio.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &gpio);

  //Strobed LED
  gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  gpio.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &gpio);

  nvic.NVIC_IRQChannel = TIM2_IRQn;    //Timer_2 general interrupt config
  nvic.NVIC_IRQChannelPreemptionPriority = 0;    //count one rotate time
  nvic.NVIC_IRQChannelSubPriority = 1;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);

  nvic.NVIC_IRQChannel = TIM3_IRQn;    //Timer_3 general interrupt config
  nvic.NVIC_IRQChannelPreemptionPriority = 1;    //Strob for: 10 hours; 1 hour; ":" between hours and minutes; ten minutes
  nvic.NVIC_IRQChannelSubPriority = 1;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);

  nvic.NVIC_IRQChannel = TIM4_IRQn;    // Timer_4 general intterupt config
  nvic.NVIC_IRQChannelPreemptionPriority = 1;    //Strob for 1 minute; ":" between minutes and seconds; 10 seconds; 1 second
  nvic.NVIC_IRQChannelSubPriority = 1;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);

  TIM_TimeBaseStructInit(&tim);    //This counter count one rotate of clock table
  tim.TIM_CounterMode = TIM_CounterMode_Up;
  tim.TIM_Period = 65535;    //1us * 65535 = 65ms 535us
  tim.TIM_Prescaler = 64;    //fclk=64Mhz/64000=1MHz
  tim.TIM_ClockDivision = 0;
  TIM_TimeBaseInit(TIM2, &tim);

  TIM_ICStructInit(&ictim);    //Timer_2 config input capture, zero position detection
  ictim.TIM_Channel = TIM_Channel_2;
  ictim.TIM_ICPolarity = TIM_ICPolarity_Rising;
  ictim.TIM_ICSelection = TIM_ICSelection_DirectTI;
  ictim.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  ictim.TIM_ICFilter = 0;
  TIM_ICInit(TIM2, &ictim);

  tim.TIM_CounterMode = TIM_CounterMode_Up;    //Timer_3 config for LED strobes
  tim.TIM_Prescaler = 64;
  tim.TIM_Period = 65535;
  TIM_TimeBaseInit(TIM3, &tim);

  tim.TIM_CounterMode = TIM_CounterMode_Up;    //Timer_4 config for LED strobes
  tim.TIM_Prescaler = 64;
  tim.TIM_Period = 65535;
  TIM_TimeBaseInit(TIM4, &tim);

  //Timer_3 channel 1 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = H10;    //H10 pulses
  TIM_OC1Init(TIM3, &octim);   //Channel 1 initialization

  //Timer_3 channel 2 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = H1;    //H1 pulses
  TIM_OC2Init(TIM3, &octim);   //Channel 2 initialization

  //Timer_3 channel 3 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = HH;    //HH pulses
  TIM_OC3Init(TIM3, &octim);   //Channel 3 initialization

  //Timer_3 channel 4 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = M10;    //M10 pulses
  TIM_OC4Init(TIM3, &octim);   //Channel 4 initialization

  //Timer_4 channel 1 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = M1;    //M1 pulses
  TIM_OC1Init(TIM4, &octim);   //Channel 1 initialization

  //Timer_4 channel 2 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = MM;    //MM pulses
  TIM_OC2Init(TIM4, &octim);   //Channel 2 initialization

  //Timer_4 channel 3 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = S10;    //S10 pulses
  TIM_OC3Init(TIM4, &octim);   //Channel 3 initialization

  //Timer_4 channel 4 config
  octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
  octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable output
  octim.TIM_Pulse = S1;    //S1 pulses
  TIM_OC4Init(TIM4, &octim);   //Channel 4 initialization

  TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
  TIM_Cmd(TIM2, ENABLE);    //Start Timer TIM2

  TIM_ITConfig(TIM3, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);    //Enable interrupt from Timer 2 from Clock Capture 1, 2, 3, 4
  TIM_Cmd(TIM3, ENABLE);    //Start Timer TIM3

  TIM_ITConfig(TIM4, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, ENABLE);    //Enable interrupt from Timer 3 from Clock Capture 1, 2, 3, 4
  TIM_Cmd(TIM4, ENABLE);    //Start Timer TIM4

  GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

  if (SysTick_Config_Mod(SysTick_CLKSource_HCLK_Div8, 8000ul))
  {
    while(1)    // infinity loop in error case
    {
    }
  }

  while(1)
  {

    if (SysTick100ms == 0 && Task100ms == 0)    //every seconds
    {
      if ((s_decade == 5) && (s_unit == 10))    //If seconds = 60
      {
        s_unit = 0;
        s_decade = 0;
        m_unit++;    //increment minutes units
      }

      if (s_unit == 10)    //If seconds = 10
      {
        s_unit = 0;
        s_decade++;          //increment seconds decade
      }

      if ((m_decade == 5) && (m_unit == 10))    //If minutes = 60
      {
        m_unit = 0;
        m_decade = 0;
        h_unit++;    //increment hours units
      }

      if (m_unit == 10)    //If minutes = 10
      {
        m_unit = 0;
        m_decade++;    //increment minutes decade
      }

      if ((h_decade == 2) && (h_unit == 4))    //if hours = 24 reset decade and unit hours
      {
        h_unit = 0;
        h_decade = 0;
      }

      if (h_unit == 10)    //If hours = 10
      {
        h_unit = 0;
        h_decade++;    //increment hours decade
      }
      if (colon_toggle == 0)
        {
          colon_toggle++;
        }
        else
        {
          colon_toggle = 0;
        }
      Task100ms = 1;
    }

    if (NewSpin == 0)    //every each finished rotate
    {
      switch (h_decade){    //hours decade
      case 0:
      {
        H10 = FullSpin * 10 / 12;
        break;
      }
      case 1:
      {
        H10 = FullSpin * 11 / 12;
        break;
      }
      case 2:
      {
        H10 = 0;
        break;
      }
      default:
      {
      break;
      }
      }

      switch (h_unit){    //hours units
      case 0:
      {
        H1 = FullSpin * 9 / 12;
        break;
      }
      case 1:
      {
        H1 = FullSpin * 10 / 12;
        break;
      }
      case 2:
      {
        H1 = FullSpin * 11 / 12;
        break;
      }
      case 3:
      {
        H1 = 0;
        break;
      }
      case 4:
      {
        H1 = FullSpin * 1 / 12;
        break;
      }
      case 5:
      {
        H1 = FullSpin * 3 / 12;
        break;
      }
      case 6:
      {
        H1 = FullSpin * 4 / 12;
        break;
      }
      case 7:
      {
        H1 = FullSpin * 5 / 12;
        break;
      }
      case 8:
      {
        H1 = FullSpin * 6 / 12;
        break;
      }
      case 9:
      {
        H1 = FullSpin * 7 / 12;
        break;
      }
      default:
      {
        break;
      }
      }

      switch (m_decade){    //minutes decades
      case 0:
      {
        M10 = FullSpin * 15 / 24;
        break;
      }
      case 1:
      {
        M10 = FullSpin * 17 / 24;
        break;
      }
      case 2:
      {
        M10 = FullSpin * 19 / 24;
        break;
      }
      case 3:
      {
        M10 = FullSpin * 21 / 24;
        break;
      }
      case 4:
      {
        M10 = FullSpin * 23 / 24;
        break;
      }
      case 5:
      {
        M10 = FullSpin * 3 / 24;
        break;
      }
      case 6:
      {
        M10 = FullSpin * 5 / 24;
        break;
      }
      case 7:
      {
        M10 = FullSpin * 71 / 24;
        break;
      }
      case 8:
      {
        M10 = FullSpin * 9 / 24;
        break;
      }
      case 9:
      {
        M10 = FullSpin * 11 / 24;
        break;
      }
      default:
      {
        break;
      }
      }

      switch (m_unit){    //minutes units
      case 0:
      {
        M1 = FullSpin * 13 / 24;
        break;
      }
      case 1:
      {
        M1 = FullSpin * 15 / 24;
        break;
      }
      case 2:
      {
        M1 = FullSpin * 17 / 24;
        break;
      }
      case 3:
      {
        M1 = FullSpin * 19 / 24;
        break;
      }
      case 4:
      {
        M1 = FullSpin * 21 / 24;
        break;
      }
      case 5:
      {
        M1 = FullSpin * 1 / 24;
        break;
      }
      case 6:
      {
        M1 = FullSpin * 3 / 24;
        break;
      }
      case 7:
      {
        M1 = FullSpin * 5 / 24;
        break;
      }
      case 8:
      {
        M1 = FullSpin * 7 / 24;
        break;
      }
      case 9:
      {
        M1 = FullSpin * 9 / 24;
        break;
      }
      default:
      {
        break;
      }
      }

      switch (s_decade){    //seconds decades
      case 0:
      {
        S10 = FullSpin * 5 / 12;
        break;
      }
      case 1:
      {
        S10 = FullSpin * 6 / 12;
        break;
      }
      case 2:
      {
        S10 = FullSpin * 7 / 12;
        break;
      }
      case 3:
      {
        S10 = FullSpin *8 / 12;
        break;
      }
      case 4:
      {
        S10 = FullSpin * 9 / 12;
        break;
      }
      case 5:
      {
        S10 = FullSpin * 11 / 12;
        break;
      }
      case 6:
      {
        S10 = 0;
        break;
      }
      case 7:
      {
        S10 = FullSpin * 1 / 12;
        break;
      }
      case 8:
      {
        S10 = FullSpin * 2 / 12;
        break;
      }
      case 9:
      {
        S10 = FullSpin * 3 / 12;
        break;
      }
      default:
      {
        break;
      }
      }

      switch (s_unit){    //seconds units
      case 0:
      {
        S1 = FullSpin * 4 / 12;
        break;
      }
      case 1:
      {
        S1 = FullSpin * 5 / 12;
        break;
      }
      case 2:
      {
        S1 = FullSpin * 6 / 12;
        break;
      }
      case 3:
      {
        S1 = FullSpin * 7 / 12;
        break;
      }
      case 4:
      {
        S1 = FullSpin * 8 / 12;
        break;
      }
      case 5:
      {
        S1 = FullSpin * 10 / 12;
        break;
      }
      case 6:
      {
        S1 = FullSpin * 11 / 12;
        break;
      }
      case 7:
      {
        S1 = 0;
        break;
      }
      case 8:
      {
        S1 = FullSpin * 1 / 12;
        break;
      }
      case 9:
      {
        S1 = FullSpin * 2 / 12;
        break;
      }
      default:
      {
        break;
      }
      }

      HH = FullSpin * 5 /48;
      MM = FullSpin * 19 /48;

      TIM_OCInitTypeDef octim;

      //Timer_3 1st channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = H10;    //H10 pulses
      TIM_OC1Init(TIM3, &octim);   //1st channel initialization

      //Timer_3 2nd channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = H1;    //H1 pulses
      TIM_OC2Init(TIM3, &octim);    //2nd channel initialization

      //Timer_3 3rd channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //tryb pracy kanalu
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = HH;  //HH pulses
      TIM_OC3Init(TIM3, &octim);   //3rd channel initialization

      ////Timer_3 4th channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = M10;    //M10 pulses
      TIM_OC4Init(TIM3, &octim);   //4th channel initialization

      //Timer_4 1st channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = M1;  //M1 pulses
      TIM_OC1Init(TIM4, &octim);    //1st channel initialization

      //Timer_4 2nd channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = MM;  //MM pulses
      TIM_OC2Init(TIM4, &octim);    //2nd channel initialization


      //Timer_4 3rd channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = S10;    //HH pulses
      TIM_OC3Init(TIM4, &octim);    //3rd channel initialization

      //Timer_4 4th channel config
      octim.TIM_OCMode = TIM_OCMode_Timing;    //mode
      octim.TIM_OutputState = TIM_OutputState_Enable;    //Enable outputs
      octim.TIM_Pulse = S1;  //S10 pulses
      TIM_OC4Init(TIM4, &octim);   //4th channel initialization

      NewSpin = 1;

    }
  }

}

void SysTick_Handler(void)
{
  SysTick1ms++;
  if (SysTick1ms == 100)
  {
    SysTick100ms++;
    SysTick1ms = 0;
    Task1ms = 0;
  }
  if (SysTick100ms == 10)
  {
    s_unit++;
    SysTick100ms = 0;
    Task100ms = 0;
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_5)));
  }
/*




  printf ("Time: %d%d:%d%d:%d%d\r\n", h_decade, h_unit, m_decade, m_unit, s_decade, s_unit);

  s_unit++;
*/
}

uint32_t  SysTick_Config_Mod(uint32_t SysTick_CLKSource, uint32_t Ticks)
{
  //SysTick initialization
  uint32_t Settings;

  assert_param(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource));

  if (Ticks > SysTick_LOAD_RELOAD_Msk)  return (1);    //Kontrola, czy wartosc poczatkowa nie przekracza max

  SysTick->LOAD = (Ticks & SysTick_LOAD_RELOAD_Msk) - 1;  //Set timer start value
  NVIC_SetPriority (SysTick_IRQn, 0);   //set interrupt priority
  SysTick->VAL  = 0;  //Set value
  Settings=SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
  if (SysTick_CLKSource == SysTick_CLKSource_HCLK)
  {
    Settings |= SysTick_CLKSource_HCLK;
  }
  else
  {
    Settings &= SysTick_CLKSource_HCLK_Div8;
  }
  SysTick->CTRL = Settings;    //Save settings to SysTick register and switch on timer
  return (0);
}

void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Trigger);
  }
  if (TIM_GetITStatus(TIM2, TIM_IT_CC2) == SET)    //If interrupt from 2nd channel
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);    //reset interrupt flag

    FullSpin = TIM_GetCapture2(TIM2);    //Read register CCR value for 2nd channel
    TIM_SetCounter(TIM2, 0);    //Timer_2 reset
    TIM_SetCounter(TIM3, 0);    //Timer_3 reset
    TIM_SetCounter(TIM4, 0);    //Timer_4 reset
    GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
    NewSpin = 0;
  }
}

void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)   //if interrupt from Timer_3 event
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);     //reset interrupt flag
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)  {    //value from CC1 is achieved
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
    GPIO_SetBits(GPIOC, GPIO_Pin_0);
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)  {    //value from CC2 is achieved
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
    GPIO_SetBits(GPIOC, GPIO_Pin_1);
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)  {    //value from CC3 is achieved
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
    if (colon_toggle == 1)
    {
      GPIO_SetBits(GPIOC, GPIO_Pin_2);
    }
  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)  {    //value from CC4 is achieved
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
    GPIO_SetBits(GPIOC, GPIO_Pin_3);
  }
}

void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)   //if interrupt from Timer_4 event
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);    //reset interrupt flag
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)  {    //value from CC1 is achieved
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    GPIO_SetBits(GPIOC, GPIO_Pin_4);
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)  {    //value from CC2 is achieved
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);
    if (colon_toggle == 1)
    {
      GPIO_SetBits(GPIOC, GPIO_Pin_5);
    }
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)  {    //value from CC3 is achieved
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC3);
    GPIO_SetBits(GPIOC, GPIO_Pin_6);
  }
  if (TIM_GetITStatus(TIM4, TIM_IT_CC4) != RESET)  {    //value from CC4 is achieved
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC4);
    GPIO_SetBits(GPIOC, GPIO_Pin_7);
  }
}
