#include "RH_RF95.h"

//initialization of the LoRa module
bool RF95_init(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs)
{
    // Set sleep mode, so we can also set LORA mode:
    spiWrite(hspi2, cs, RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE);
    HAL_Delay(10); // Wait for sleep mode to take over from say, CAD
    // Check we are in sleep mode, with LORA set

    if (spiRead(hspi2, cs, RH_RF95_REG_01_OP_MODE) != (RH_RF95_MODE_SLEEP | RH_RF95_LONG_RANGE_MODE))
    {
	return false; // No device present?
    }

    // Set up FIFO
    // We configure so that we can use the entire 256 byte FIFO for either receive
    // or transmit, but not both at the same time
    spiWrite(hspi2, cs,RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
    spiWrite(hspi2, cs,RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);

    //SetIdleMode
    spiWrite(hspi2, cs, RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_STDBY);

    //ModemConfig
    spiWrite(hspi2, cs, RH_RF95_REG_1D_MODEM_CONFIG1, 0x72);
    spiWrite(hspi2, cs, RH_RF95_REG_1E_MODEM_CONFIG2, 0x74);
    spiWrite(hspi2, cs, RH_RF95_REG_26_MODEM_CONFIG3, 0x00);

    //setPreambleLength
    spiWrite(hspi2, cs, RH_RF95_REG_20_PREAMBLE_MSB, 8 >> 8);
    spiWrite(hspi2, cs, RH_RF95_REG_21_PREAMBLE_LSB, 8 & 0xff);


    //Set Frequency
    uint32_t frf = (915 * 1000000.0) / RH_RF95_FSTEP;
    spiWrite(hspi2, cs, RH_RF95_REG_06_FRF_MSB, (frf >> 16) & 0xff);
    spiWrite(hspi2, cs, RH_RF95_REG_07_FRF_MID, (frf >> 8) & 0xff);
    spiWrite(hspi2, cs, RH_RF95_REG_08_FRF_LSB, frf & 0xff);

    // Lowish power (setTXPower)
    spiWrite(hspi2, cs, RH_RF95_REG_09_PA_CONFIG, RH_RF95_PA_SELECT | 18);

    return true;
}

//Transmit a signal out to another LoRa
bool send(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, const uint8_t* data)
{
	// Position at the beginning of the FIFO
	spiWrite(hspi2, cs, RH_RF95_REG_0D_FIFO_ADDR_PTR, 0);

	// The message data
	spiBurstWrite(hspi2, cs, RH_RF95_REG_00_FIFO, data);
	spiWrite(hspi2, cs, RH_RF95_REG_22_PAYLOAD_LENGTH, strlen(data));

    setModeTx(hspi2, cs); // Start the transmitter

    HAL_Delay(1000);
    // when Tx is done, interruptHandler will fire and radio mode will return to STANDBY
    return true;
}

//set the LoRa module to Rx mode
void setModeRx(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs)
{
	spiWrite(hspi2,GPIOB,0x12,0xff); //clear any flags that is high
	spiWrite(hspi2,GPIOB,0x12,0xff);
	spiWrite(hspi2, cs, RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_RXCONTINUOUS);
}

//set the LoRa module to Tx mode
void setModeTx(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs)
{
	spiWrite(hspi2,GPIOB,0x12,0xff); //clear any flags that is high
	spiWrite(hspi2,GPIOB,0x12,0xff);
	spiWrite(hspi2, cs, RH_RF95_REG_01_OP_MODE, RH_RF95_MODE_TX);
}

//a standard write to the LoRa module register
void spiWrite(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, uint8_t reg, uint8_t val)
{
	uint8_t trans_reg = reg|0x80;
	HAL_GPIO_WritePin(cs,GPIO_PIN_12,GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2,&trans_reg,sizeof(reg),30);
	HAL_SPI_Transmit(&hspi2,& val,sizeof(val),30);
	HAL_GPIO_WritePin(cs,GPIO_PIN_12,GPIO_PIN_SET);
}

//a burst write to the LoRa module register (This still need more work)
void spiBurstWrite(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, uint8_t reg, const uint8_t* msg)
{
	uint8_t trans_reg = reg|0x80;
	HAL_GPIO_WritePin(cs,GPIO_PIN_12,GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2,&trans_reg,sizeof(reg),30);
	HAL_SPI_Transmit(&hspi2,msg,strlen(msg),30);
	HAL_GPIO_WritePin(cs,GPIO_PIN_12,GPIO_PIN_SET);
}

//a standard read from a LoRa module register
uint8_t spiRead(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs,uint8_t reg) //need to test the size
{
	uint8_t output;
	HAL_GPIO_WritePin(cs,GPIO_PIN_12,GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2,&reg,sizeof(reg),30);
	HAL_SPI_Receive(&hspi2,&output,sizeof(output),30);
	HAL_GPIO_WritePin(cs,GPIO_PIN_12,GPIO_PIN_SET);

	return output;
}

void spiReadbuff(SPI_HandleTypeDef hspi2, GPIO_TypeDef* cs, uint8_t* buffer)
{
	uint8_t bytelength;
	uint8_t byte;
	uint8_t rxcurrentaddr;

	//set the pointer accordingly to point to rx_data
	rxcurrentaddr = spiRead(hspi2,GPIOB,0x10);
	spiWrite(hspi2,GPIOB,0x0D,rxcurrentaddr);

	//get the size of packet
	bytelength = spiRead(hspi2,cs,0x13);

	//read the data to the buffer(max size of 256)
	for(int store = 0; store < bytelength; store++)
	{
	  byte = spiRead(hspi2,GPIOB,0x00);
	  memcpy(&(buffer[store]),&byte,1);
	  if (byte == 0x0) {break;}
	}
}
