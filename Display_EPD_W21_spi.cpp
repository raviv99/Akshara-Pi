#include "Display_EPD_W21_spi.h"
#include <SPI.h>

SPISettings epdSettings(2000000, MSBFIRST, SPI_MODE0);

void SPI_Write(unsigned char value)                                    
{                                                           
    SPI.beginTransaction(epdSettings);
    SPI.transfer(value);
    SPI.endTransaction();
}

void EPD_W21_WriteCMD(unsigned char command)
{
	EPD_W21_CS_0;
	EPD_W21_DC_0;  // D/C#   0:command  1:data  
	SPI_Write(command);
	EPD_W21_CS_1;
}

void EPD_W21_WriteDATA(unsigned char datas)
{
	EPD_W21_CS_0;
	EPD_W21_DC_1;  // D/C#   0:command  1:data
	SPI_Write(datas);
	EPD_W21_CS_1;
}

unsigned char EPD_W21_ReadDATA(void)
{
	unsigned char temp;
    SPI.beginTransaction(epdSettings);
    EPD_W21_CS_0;                   
	EPD_W21_DC_1;		
    temp = SPI.transfer(0x00);
	EPD_W21_CS_1;
    SPI.endTransaction();
	return(temp);
}