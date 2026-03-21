#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

//IO settings
#define isEPD_W21_BUSY digitalRead(4)  //BUSY
#define EPD_W21_RST_0 digitalWrite(5,LOW)  //RES
#define EPD_W21_RST_1 digitalWrite(5,HIGH)
#define EPD_W21_DC_0  digitalWrite(6,LOW) //DC
#define EPD_W21_DC_1  digitalWrite(6,HIGH)
#define EPD_W21_CS_0 digitalWrite(7,LOW) //CS
#define EPD_W21_CS_1 digitalWrite(7,HIGH)

#define EPD_W21_MOSI_0  digitalWrite(11,LOW) //SDIN (SDA) - FSPI MOSI
#define EPD_W21_MOSI_1  digitalWrite(11,HIGH) 
#define EPD_W21_CLK_0 digitalWrite(12,LOW) //SCK (SCL) - FSPI SCK
#define EPD_W21_CLK_1 digitalWrite(12,HIGH)
#define EPD_W21_READ digitalRead(11)  //SDA


void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);
unsigned char EPD_W21_ReadDATA(void);

#endif 
