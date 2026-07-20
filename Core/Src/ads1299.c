/**
 ****************************************************************************************************
 * @file        ADS1299.c
 * @author      润泰实验室
 * @version     V1.0
 * @date        2025-08-27
 * @brief       ADS1299驱动代码
 * @license     Copyright (c) 2020-2032, 深圳市润谊泰谊科技有限责任公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:RT-ADS1299模块+STM32F103核心板
 * 在线视频:
 * 技术论坛:
 * 公司网址:
 * 购买地址:
 *
 ****************************************************************************************************
 */

#include "ads1299.h"
#include "spi.h"
#include "usart.h"
#include "delay.h"
#include "string.h"

#define SPI1_Handler hspi1


static uint8_t spi_tx_buf[30];
static uint8_t spi_rx_buf[30];


/* ADS1299寄存器 */
uint8_t ADS1299_REG[24]; 

// 模拟前端放大倍数
uint8_t pga = 12;

/**
 * @brief  SPI读写
 * @param  tx_data: 发送数据缓冲区指针 
 * @param  rx_data: 接收数据缓冲区指针
 * @param  len: 发送和接收的数所长度
 * @retval 无
 */
void ADS1299_SPI_RW(uint8_t *tx_data, uint8_t *rx_data,uint8_t len)
{
	HAL_SPI_TransmitReceive(&SPI1_Handler, tx_data, rx_data, len, 10);
}


/**
 * @brief  设置SPI速率
 * @retval 无
 */
void ADS1299_SPI_Rate_Set(uint32_t baudRatePrescaler) 
{
	SPI1_Handler.Init.BaudRatePrescaler = baudRatePrescaler;;
	if (HAL_SPI_Init(&SPI1_Handler) != HAL_OK)
  {
    Error_Handler();
  }
}
	
/**
 * @brief  发送命令
 * @param  cmd: 命令码 
 * @retval 无
 */
void ADS1299_Send_Cmd(uint8_t cmd)
{
	ADS1299_CS_L;
	spi_tx_buf[0] = cmd;
	ADS1299_SPI_RW(spi_tx_buf,spi_rx_buf,1);
	ADS1299_CS_H;
}

/**
 * @brief  连接写入寄存器
 * @param  addr: 寄存器起始地址 
 * @param  regs: 寄存器数组指针
 * @param  len:  写入寄存器数量
 * @retval 无
 */
void ADS1299_Write_Regs(uint8_t addr,uint8_t *regs,uint8_t len)
{
   spi_tx_buf[0] = 0x40+addr;
   spi_tx_buf[1] = 0x00+len-1;
   ADS1299_CS_L;
   ADS1299_SPI_RW(spi_tx_buf,spi_rx_buf,2);
   ADS1299_SPI_RW(regs,spi_rx_buf,len);
   ADS1299_CS_H;
}

/**
 * @brief  连接读取寄存器
 * @param  addr: 寄存器起始地址 
 * @param  regs: 寄存器数组指针
 * @param  len:  读取寄存器数量
 * @retval 无
 */
void ADS1299_Read_Regs(uint8_t addr,uint8_t *regs,uint8_t len)
{
   spi_tx_buf[0] = 0x20+addr;
   spi_tx_buf[1] = 0x00+len-1;
   ADS1299_CS_L;
	 ADS1299_SPI_RW(spi_tx_buf,spi_rx_buf,2);
	 memset(spi_tx_buf,0x00,sizeof(spi_tx_buf));
	 ADS1299_SPI_RW(spi_tx_buf,regs,len);
   ADS1299_CS_H;
}

/**
 * @brief  ADS1299上电复位
 * @retval 无
 */
void ADS1299_Rest(void)
{
    ADS1299_CS_H;
    ADS1299_START_L;
	  ADS1299_REST_H;
	  ADS1299_PWDN_H; 
	  delay_ms(200); //将PWDN引脚拉高后，等待稳定   
	
	 // 复位设备
	  ADS1299_REST_L;
	  delay_ms(10);
	  ADS1299_REST_H;
	  delay_ms(50); 
	  
	  //发送停止连续读取指令
	  ADS1299_Send_Cmd(ADS1299_SDATAC); 
	
	  //发送停止采集指令
	  ADS1299_Send_Cmd(ADS1299_STOP);   
}

/**
 * @brief  ADS1299 初始化
 * @retval 无
 */
void ADS1299_Init(void)
{	
	//ADS1299_SPI_Rate_Set(SPI_BAUDRATEPRESCALER_128);
	
	/* 上电复位 */
  ADS1299_Rest();

	ADS1299_REG[0x00] = 0x1E;   //ID
	ADS1299_REG[0x01] = 0x95;   //CONFIG1 0x96(0.25kSPS) 0x95(0.5kSPS)  0x94(1kSPS)  0x93(2kSPS) 
  ADS1299_REG[0x02] = 0xD4;   //CONFIG2 在内部生成测试信号     测测试试信信 : 2×–(VREFP – VREFN)/2400V = 2*4.5/24000V = 3.7mV
  ADS1299_REG[0x03] = 0xEC;   //CONFIG3 
  ADS1299_REG[0x04] = 0x00;   //LOFF

//	// 采集测试信号
//	ADS1299_REG[0x05] = 0x55;   //CH1SET
//  ADS1299_REG[0x06] = 0x55;   //CH2SET
//  ADS1299_REG[0x07] = 0x55;   //CH3SET
//  ADS1299_REG[0x08] = 0x55;   //CH4SET
//  ADS1299_REG[0x09] = 0x55;   //CH5SET
//  ADS1299_REG[0x0A] = 0x55;   //CH6SET
//  ADS1299_REG[0x0B] = 0x55;   //CH7SET
//  ADS1299_REG[0x0C] = 0x55;   //CH8SET
//	
//  // 采集短路噪声
//	ADS1299_REG[0x05] = 0x51;   //CH1SET
//  ADS1299_REG[0x06] = 0x51;   //CH2SET
//  ADS1299_REG[0x07] = 0x51;   //CH3SET
//  ADS1299_REG[0x08] = 0x51;   //CH4SET
//  ADS1299_REG[0x09] = 0x51;   //CH5SET
//  ADS1299_REG[0x0A] = 0x51;   //CH6SET
//  ADS1299_REG[0x0B] = 0x51;   //CH7SET
//  ADS1299_REG[0x0C] = 0x51;   //CH8SET
	
	// 采集输入信号	
	ADS1299_REG[0x05] = 0x50;   //CH1SET
  ADS1299_REG[0x06] = 0x50;   //CH2SET
  ADS1299_REG[0x07] = 0x50;   //CH3SET
  ADS1299_REG[0x08] = 0x50;   //CH4SET
  ADS1299_REG[0x09] = 0x50;   //CH5SET
  ADS1299_REG[0x0A] = 0x50;   //CH6SET
  ADS1299_REG[0x0B] = 0x50;   //CH7SET
  ADS1299_REG[0x0C] = 0x50;   //CH8SET
	
  ADS1299_REG[0x0D] = 0x00;   //RLD_SENSP
  ADS1299_REG[0x0E] = 0x00;   //RLD_SENSN
  ADS1299_REG[0x0F] = 0x00;   //LOFF_SENSP 关闭导联脱落检测
  ADS1299_REG[0x10] = 0x00;   //LOFF_SENSN 关闭导联脱落检测
  ADS1299_REG[0x11] = 0x00;   //LOFF_FLIP
  ADS1299_REG[0x12] = 0x00;   //LOFF_STATP
  ADS1299_REG[0x13] = 0x00;   //LOFF_STATN
  ADS1299_REG[0x14] = 0x00;   //GPIO 
	//ADS1299_REG[0x14] = 0x0F;   //GPIO 
  //ADS1299_REG[0x15] = 0x00;   //MISC1 //差分模式
	ADS1299_REG[0x15] = 0x20;   //MISC1  //单端模式
  ADS1299_REG[0x16] = 0x00;   //MISC2
  ADS1299_REG[0x17] = 0x00;   //CONFIG4 连续转化模式
	
	// 写入寄存器 
	ADS1299_Write_Regs(0x01,ADS1299_REG+1,23);
	
	// 加入延时，确保能读取寄存器成功
	delay_ms(10);
	
	// 读取寄存器
	ADS1299_Read_Regs(0x00,ADS1299_REG,24);
	
	// ID寄存器，去掉版本号表示位后 ADS1299 ID = 0x1E
	ADS1299_REG[0] &= 0x1F;
  
	// 打印从器件读取的寄存器值 
	for(uint8_t n=0;n<24;n++)
	{
		printf("ADS1299_reg[0x%x] = 0x%x \r\n",n,ADS1299_REG[n]);
	}
	
	// 提高SPI速率传输速率
	ADS1299_SPI_Rate_Set(SPI_BAUDRATEPRESCALER_4);
}

// 设置采样率、PGA放大倍数、通道输入
// 采样率:
// 0x00: 250sps
// 0x01: 500sps
// 0x02: 1000sps
// 0x03: 2000sps
// 0x04: 4000sps
// 量程:
// 0x00: ±4.5V（PGA=1）      
// 0x01: ±2.25V（PGA=2）
// 0x02: ±1.125V（PGA=4）
// 0x03: ±750mV（PGA=6）
// 0x04: ±562.5mV（PGA=8）
// 0x05: ±375mV（PGA=12）
// 0x06: ±187.5mV（PGA=24）
// 通道选择:
// 0x00: 电极输入
// 0x01: 正负极内部短路
// 0x02: 测试信号
void ADS1299_Set_Sample_Parameter(uint8_t rate_index, uint8_t pga_index, uint8_t ch_input_index)
{ 
	 //采样率
	 if(rate_index <= 0x04)
	 {
		  uint8_t rate_regs[5] = {0x06,0x05,0x04,0x03,0x02};	 
		  ADS1299_REG[0x01] &= 0xF8;
		  ADS1299_REG[0x01] |= rate_regs[rate_index];	 
	 }
	 
   //PGA
	 if(pga_index <= 0x06)
	 {
		 	 uint8_t pga_regs[7] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06};	 
			 for(uint8_t ch=0;ch<8;ch++)
			 {
					 uint8_t reg = ADS1299_REG[0x05+ch];
					 reg &= 0x8F;
					 reg |= pga_regs[pga_index]<<4;
					 ADS1299_REG[0x05+ch] = reg;
			 } 
			 
			 // 更新放大倍数
			 uint8_t pga_vals[7] = {1,2,4,6,8,12,24};	 
		   pga = pga_vals[pga_index]; 
	 }

		// 通道输入
	  if(ch_input_index <= 0x02)
		{ 
		    uint8_t ch_regs[7] = {0x00,0x01,0x05};	 
				for(uint8_t ch=0;ch<8;ch++)
				{					
						uint8_t reg = ADS1299_REG[0x05+ch];
						reg &= 0xF8;
		        reg |= ch_regs[ch_input_index];	 
						ADS1299_REG[0x05+ch] = reg;
				}
		}
	
		// 降低SPI速率，配置寄存器
		ADS1299_SPI_Rate_Set(SPI_BAUDRATEPRESCALER_64);
		delay_ms(1);
		

		ADS1299_Send_Cmd(ADS1299_SDATAC); //停止连续读取
	  ADS1299_Send_Cmd(ADS1299_STOP);   //停止采集
	  delay_ms(1);
		
		// 写入寄存器 
		ADS1299_Write_Regs(0x01,ADS1299_REG+1,25);
		
		// 加入延时，确保能读取寄存器成功
		delay_ms(1);
		
		// 读取寄存器
		ADS1299_Read_Regs(0x00,ADS1299_REG,26);
		
		// ID寄存器，去掉版本号表示位后 ADS1299 ID = 0x1E
	  ADS1299_REG[0] &= 0x1F;
		
		// 打印从器件读取的寄存器值 
		for(uint8_t n=0;n<25;n++)
		{
			printf("ads1299_reg[0x%x] = 0x%x \r\n",n,ADS1299_REG[n]);
		}
		
		// 提高SPI速率，读取数据
    ADS1299_SPI_Rate_Set(SPI_BAUDRATEPRESCALER_4);
}

void ADS1299_Get_Sample_Parameter(uint8_t *rate_index, uint8_t *pga_index, uint8_t *ch_input_index)
{
	uint8_t rate_regs[5] = {0x06,0x05,0x04,0x03,0x02};
  uint8_t rate_reg = ADS1299_REG[0x01]&0x07;

	for(uint8_t i =0;i<5;i++) 
	{
		if(rate_reg == rate_regs[i])
		{
			*rate_index =  i;
			break;
		}
	}
	
	uint8_t pga_regs[7] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06};	
	uint8_t pga_reg = ((ADS1299_REG[0x05] & 0x70) >> 4) & 0x07;
	for(uint8_t i =0;i<4;i++) 
	{
		if(pga_reg == pga_regs[i])
		{
			*pga_index =  i;
			break;
		}
	}
	
	uint8_t ch_regs[7] = {0x00,0x01,0x05};	 
	uint8_t ch_reg =ADS1299_REG[0x05] & 0x07;
	for(uint8_t i =0;i<4;i++) 
	{
		if(ch_reg == ch_regs[i])
		{
			*ch_input_index =  i;
			break;
		}
	}
}


/**
 * @brief  停止采集
 * @retval 无
 */
void ADS1299_Stop()
{  
	  ADS1299_Send_Cmd(ADS1299_SDATAC); //停止连续读取
    ADS1299_START_L;
	  ADS1299_CS_H;
}

/**
 * @brief  开始采集
 * @retval 无
 */
void ADS1299_Start()
{
	  memset(spi_tx_buf,0x00,sizeof(spi_tx_buf));
	  ADS1299_Send_Cmd(ADS1299_RDATAC); //开启连续读取
    ADS1299_START_H;
	  ADS1299_CS_L;
}

/**
 * @brief  开始采集
 * @retval 无
 */
void ADS1299_ReadData(float *chx_val)
{	
	//ADS1299_CS_L;
	HAL_SPI_TransmitReceive(&SPI1_Handler, spi_tx_buf, spi_rx_buf, 27, 100);
	//ADS1299_CS_H;
	
	for(uint8_t ch =0;ch<8;ch++)
	{
		 uint8_t index = 3 + 3*ch;
	   chx_val[ch] = ((int32_t)(spi_rx_buf[index]<<24 | spi_rx_buf[index+1]<<16 | spi_rx_buf[index+2]<<8))/256.0f;
		
		 /*((2*4.5)/2^24)*10^6  =  0.5364 */
		 chx_val[ch] = (chx_val[ch] * 0.5364f)/pga;  //单位uV  
	}
}



