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
extern uint8_t buff[8];
typedef struct
{
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t days;
  uint8_t months;
  uint16_t years;
  uint8_t typeOfClock;  //0- ; 1-
} time_t;


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

time_t readDateTime(void)
{
  time_t dt; // Structure created
  uint8_t rawRtc[0x08]; // buffer for data from RTC

  readRtc(0xD0, sizeof(rawRtc), rawRtc); // Read data

  // seconds decade * 10  + seconds unit
  dt.seconds = ((0x70 & rawRtc[0]) >> 4) * 10 + 0x0F & rawRtc[0];
  // minutes decade * 10  + minutes unit
  dt.minutes =  ((0x70 & rawRtc[1]) >> 4) * 10 + 0x0F & rawRtc[1];
  // hours decade * 10 + hours unit

  //Bit 6 means 12/24 hour mode
  //           1 - 12hour mode -> bit 5 means AM/PM 0 - AM; 1 - PM
  //           0 - 24hour mode -> bit 5 means second 10 hour bit (20-23 hours)
  dt.typeOfClock = 0X40 & buff[2] >> 3;
  if (dt.typeOfClock == 0)
  {
    dt.hours = ((0x30 & rawRtc[2] >> 4) * 10 + 0x0F & rawRtc[2]);
  }
  else
  {
    dt.hours = ((0x10 & rawRtc[2] >> 4) * 10 + 0x0F & rawRtc[2]);
  }
  // days decade * 10 + days unit
  dt.days = ((0x30 & rawRtc[4] >> 4) * 10 + 0x0F & rawRtc[4]);
  // months decade *10 + months unit
  dt.months = ((0x10 & rawRtc[5] >> 4) * 10 + 0x0F & rawRtc[5]);
  // years decade * 10 + years unit
  dt.years = ((0xF0 & rawRtc[6] >> 4) * 10 + 0x0F & rawRtc[6]);

  // If structure is filled return it
  return dt;
}


void readRtc(uint8_t address, uint8_t lenght, uint8_t *buff)
{

  I2C_GenerateSTART(I2C1, ENABLE);    //Start
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);

  I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter);    //Write to address 0xD0
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);

  I2C_SendData(I2C1, 0x00);    //Set address to 0x00
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);

  I2C_GenerateSTART(I2C1, ENABLE);
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);    //Start

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  I2C_Send7bitAddress(I2C1, address, I2C_Direction_Receiver);    //read first cell
  while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);

  for (uint8_t  i = 0; i < lenght; i++)
  {
    while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
    buff[i] = I2C_ReceiveData(I2C1);


    I2C_AcknowledgeConfig(I2C1, ENABLE);
  }
  I2C_AcknowledgeConfig(I2C1, DISABLE);
  I2C_GenerateSTOP(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);

  /*while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data1 = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, ENABLE);
  while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
  uint8_t data2 = I2C_ReceiveData(I2C1);*/

/* DAY OF WEEK */
/*  I2C_AcknowledgeConfig(I2C1, ENABLE);
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
*/

}

