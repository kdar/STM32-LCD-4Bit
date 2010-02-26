/********************************* OUTROOT.COM ********************************
* Author             : Kevin Darlington (http://outroot.com/)
* Date               : 02/21/2010
* Description        : STM32 LCD 4bit library header
*******************************************************************************/

#ifndef __STM32LCD4BIT_H
#define __STM32LCD4BIT_H

#include "stm32f10x.h"
#include "Print.h"

#define DOT_5_7 0x00
#define DOT_5_10 0x04

typedef void (*udelay_t)(__IO u32 us);
typedef void (*delay_t)(__IO u32 nTime);

//-------------------------------------
struct LCD4Bit_Config {
  GPIO_TypeDef *gpio;
  u16 rs_pin;
  u16 enable_pin;
  u16 db4_pin;
  u16 db5_pin;
  u16 db6_pin;
  u16 db7_pin;
  
  u8 cols;
  u8 rows;
  
  u8 dot_size;
  
  LCD4Bit_Config(){};
  
  LCD4Bit_Config(GPIO_TypeDef *gpio, u16 rs_pin, u16 enable_pin,
           u16 db4_pin, u16 db5_pin, u16 db6_pin, u16 db7_pin,
           u8 cols = 20, u8 rows = 2, u8 dot_size = DOT_5_7)
  {
    this->gpio = gpio;
    this->rs_pin = rs_pin;
    this->enable_pin = enable_pin;
    this->db4_pin = db4_pin;
    this->db5_pin = db5_pin;
    this->db6_pin = db6_pin;
    this->db7_pin = db7_pin;
    this->cols = cols;
    this->rows = rows;
    this->dot_size = dot_size;
  }
  
  LCD4Bit_Config(LCD4Bit_Config &data)
  {
    this->gpio = data.gpio; 
    this->rs_pin = data.rs_pin;
    this->enable_pin = data.enable_pin;
    this->db4_pin = data.db4_pin;
    this->db5_pin = data.db5_pin;
    this->db6_pin = data.db6_pin;
    this->db7_pin = data.db7_pin;
    this->cols = data.cols;
    this->rows = data.rows;
    this->dot_size = data.dot_size;
  }
};

//-------------------------------------
class LCD4Bit : public Print
{
  public:
    LCD4Bit(LCD4Bit_Config &config, udelay_t udelay, delay_t delay);
    
    void init_gpio();
    void init();
    void pulseEnablePin();
    void pushNibble(int value);
    void pushByte(int value);
    void commandWriteNibble(int nibble);
    void commandWrite(int value);
    void clear();    
    void cursorTo(int line_num, int x);
    void leftScroll(int num_chars, int delay_time);
    
    void write(u8 value);
    
    udelay_t udelay;
    delay_t delay;
    
    LCD4Bit_Config config;
};

#endif
