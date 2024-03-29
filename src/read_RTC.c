/*
 * readRtc.c
 *
 *  Created on: 01.01.2019
 *      Author: DJPifPaf
 */

#include "stm32f10x.h"

extern uint8_t hourDecade;
extern uint8_t hourUnit;
extern uint8_t minuteDecade;
extern uint8_t minuteUnit;
extern uint8_t secondDecade;
extern uint8_t secondUnit;

void initI2cForRtc(void)
{
  GPIO_InitTypeDef gpio;
  I2C_InitTypeDef i2c;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  GPIO_StructInit(&gpio);
  gpio.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; // SCL, SDA
  gpio.GPIO_Mode = GPIO_Mode_AF_OD;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &gpio);

  I2C_SoftwareResetCmd(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, DISABLE);
  I2C_StructInit(&i2c);
  i2c.I2C_Mode = I2C_Mode_I2C;
  i2c.I2C_Ack = I2C_Ack_Enable;
  i2c.I2C_ClockSpeed = 100000;
  I2C_Init(I2C1, &i2c);
  I2C_Cmd(I2C1, ENABLE);

}

void readRtc(void)
{
  I2C_GenerateSTART(I2C1, ENABLE);    //Start
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);

  I2C_Send7bitAddress(I2C1, 0xD0, I2C_Direction_Transmitter);    //Write to address 0xD0
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);

  I2C_SendData(I2C1, 0x00);    //Set address to 0x00
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);

  I2C_GenerateSTART(I2C1, ENABLE);
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);    //Start

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  I2C_Send7bitAddress(I2C1, 0xD0, I2C_Direction_Receiver);    //read first cell
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);

  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data0 = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data1 = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data2 = I2C_ReceiveData(I2C1);

/* DAY OF WEEK */
  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data3 = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data4 = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data5 = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data6 = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, DISABLE);
  I2C_GenerateSTOP(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data7 = I2C_ReceiveData(I2C1);
  //I2C_AcknowledgeConfig(I2C1, ENABLE);

  /* SECONDS */

  uint8_t tenSec = 0x70 & data0;
  tenSec >>= 4;
  uint8_t sec = 0x0F & data0;

  secondDecade = tenSec;
  secondUnit = sec;

  /* MINUTES */

  uint8_t tenMin = 0x70 & data1;
  tenMin >>= 4;
  uint8_t min = 0x0F & data1;

  minuteDecade = tenMin;
  minuteUnit = min;

  /* HOURS */

  printf("DATA2 RAW %x\r\n", data2);
  printf("DATA7 RAW %x\r\n", data7);
  uint8_t typeOfClock = 0X40 & data2;
  typeOfClock >>= 3;

  uint8_t tenHrs;
  hourDecade = tenHrs;

  int8_t amPm;

  /* DAY OF MONTH */

  uint8_t tenDays = 0x50 & data4;
  tenDays >>= 4;
  uint8_t days = 0x0F & data4;
  printf("Day: %d \r\n", data3);

  /* MONTH */
  uint8_t tenMonths = 0x10 & data5;
  tenMonths >>= 4;
  uint8_t months = 0x0F & data5;

  /* YEAR */
  uint8_t tenYears = 0xF0 & data6;
  tenYears >>= 4;
  uint8_t years = 0x0F & data6;

  if (typeOfClock == ENABLE) // 12h clock AM and PM
  {
    tenHrs = 0x10 & data2;
    tenHrs >>= 4;
  }
  else
  {
    tenHrs = 0x30 & data2;
    tenHrs >>= 4;
  }

  uint8_t hrs = 0x0F & data2;

  hourDecade = tenHrs;
  hourUnit = hrs;

  printf("Time: %d%d:%d%d:%d%d", tenHrs, hrs, tenMin, min, tenSec, sec);
  if (typeOfClock == ENABLE) // 12h clock AM and PM
  {
   amPm = 0x20 & data2;
   amPm >>= 4;
    if (amPm == DISABLE) // means AM
    {
      printf(" AM\r\n");
    }
    else
    {
      printf(" PM\r\n");
    }
  }

  printf("\r\n");
  printf("Data: %d%d.%d%d.%d%d\r\n", tenDays, days, tenMonths, months, tenYears, years);
}


