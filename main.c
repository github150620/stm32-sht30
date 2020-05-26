/*
 * Hardware Connection:
 *
 *   STM32F104C8T6               SHT30
 *  +-------------+            +-------+
 *  |             |            |       |
 *  |          PB10 <--------> SDA     |
 *  |             |            |       |
 *  |          PB11 <--------> SCK     |
 *  |             |            |       |
 *  +-------------+            +-------+
 */

#include "stm32f10x.h"

void delay(int i) {
	while (i--);
}

void SHT30_Write(u8 msb, u8 lsb) {
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	
	I2C_GenerateSTART(I2C2, ENABLE);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C2, 0x88, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
	I2C_SendData(I2C2, msb);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_SendData(I2C2, lsb);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP(I2C2, ENABLE);	
}

int SHT30_ReadTempAndHumi(u8 *data) {
	int i;
	
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	
	I2C_GenerateSTART(I2C2, ENABLE);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C2, 0x88, I2C_Direction_Transmitter);
	i = 0;
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
		if (++i > 200) {
			return -1;
		}
	}
	
	I2C_SendData(I2C2, 0xE0);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C2, 0x00);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTART(I2C2, ENABLE);
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C2, 0x89, I2C_Direction_Receiver);
	i = 0;
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
		if (++i > 200) {
			return -1;
		}
	}
	
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
	data[0] = I2C_ReceiveData(I2C2);
	
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
	data[1] = I2C_ReceiveData(I2C2);

	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
	data[2] = I2C_ReceiveData(I2C2);

	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
	data[3] = I2C_ReceiveData(I2C2);
	
	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
	data[4] = I2C_ReceiveData(I2C2);

	I2C_AcknowledgeConfig(I2C2, DISABLE);

	while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED));
	data[5] = I2C_ReceiveData(I2C2);

	I2C_GenerateSTOP(I2C2, ENABLE);	
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	
	return 0;
}

int main(void) {
	u8 data[6];
	int temp;
	int humi;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_initStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
	
	I2C_initStructure.I2C_ClockSpeed = 400000; // 400KHz
	I2C_initStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_initStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_initStructure.I2C_OwnAddress1 = 0x77;
	I2C_initStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_initStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C2, &I2C_initStructure);
	
	I2C_Cmd(I2C2, ENABLE);

	delay(100000);
	
	// Set Repeatability High, mps 0.5
	SHT30_Write(0x21, 0x30);

	while (1) {
		delay(10000000);
		if (SHT30_ReadTempAndHumi(data) == 0) {
			temp = -45 + 175 * (data[0] * 256 + data[1]) / 0xffff;
			humi = 100 * (data[3] * 256 + data[4]) / 0xffff;
		}
	}
}
