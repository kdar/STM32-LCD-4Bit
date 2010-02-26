/********************************* OUTROOT.COM ********************************
* Author             : Kevin Darlington (http://outroot.com/)
* Date               : 02/21/2010
* Description        : STM32 LCD 4bit library
*******************************************************************************/

#include "stm32lcd4bit.h"
#include "string.h"

// Command bytes for LCD
#define CMD_CLR 0x01
#define CMD_RIGHT 0x1C
#define CMD_LEFT 0x18
#define CMD_HOME 0x02

GPIO_InitTypeDef GPIO_LCD4Bit_ConfigStructure;

//=====================================
LCD4Bit::LCD4Bit(LCD4Bit_Config &config, udelay_t udelay, delay_t delay)
{
  this->config.gpio = config.gpio;
  this->config.rs_pin = config.rs_pin;
  this->config.enable_pin = config.enable_pin;
  this->config.db4_pin = config.db4_pin;
  this->config.db5_pin = config.db5_pin;
  this->config.db6_pin = config.db6_pin;
  this->config.db7_pin = config.db7_pin;
  this->config.cols = config.cols;
  this->config.rows = config.rows;
  
  this->udelay = udelay;
  this->delay = delay;
}

//=====================================
void LCD4Bit::init_gpio()
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                          RCC_APB2Periph_GPIOB |
                          RCC_APB2Periph_GPIOC |
                          RCC_APB2Periph_AFIO,
                          ENABLE);
  
  GPIO_LCD4Bit_ConfigStructure.GPIO_Pin = this->config.rs_pin | this->config.enable_pin |
                                    this->config.db4_pin | this->config.db5_pin |
                                    this->config.db6_pin | this->config.db7_pin;
  GPIO_LCD4Bit_ConfigStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_LCD4Bit_ConfigStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(this->config.gpio, &GPIO_LCD4Bit_ConfigStructure);
}

//=====================================
void LCD4Bit::init()
{
  if (this->config.rows < 1) {
    this->config.rows = 1;
  } else if (this->config.rows > 4) {
    this->config.rows = 4;
  }
  
  this->init_gpio();
  
  // The first 4 nibbles and timings are in the HD44780 standard.
  this->commandWriteNibble(0x03);
  this->delay(5);
  this->commandWriteNibble(0x03);
  this->udelay(100);
  this->commandWriteNibble(0x03);
  this->delay(5);

  // Sets up 4-bit mode.
  this->commandWriteNibble(0x02);
  this->commandWriteNibble(0x02);

  int num_lines = (this->config.rows - 1) << 3;

  this->commandWriteNibble(num_lines | this->config.dot_size);
  this->udelay(60);

  // The rest of the init is not specific to 4-bit mode.
  // NOTE: we're writing full bytes now, not nibbles.

  // display control:
  // Turn display on, cursor off, no blinking.
  this->commandWrite(0x0C);
  this->udelay(60);

  // Clear display
  this->clear();

  // Entry mode set: 06
  // Increment automatically, display shift, entire shift off
  this->commandWrite(0x06);
}

//=====================================
// Pulse the enable pin high.
// This clocks whatever command or data is in DB4~7 into the LCD controller.
void LCD4Bit::pulseEnablePin()
{
  GPIO_ResetBits(this->config.gpio, this->config.enable_pin);
  this->udelay(1);
  // send a pulse to enable
  GPIO_SetBits(this->config.gpio, this->config.enable_pin);
  this->udelay(1);
  GPIO_ResetBits(this->config.gpio, this->config.enable_pin);
  this->udelay(1);
}

//=====================================
// Push a nibble of data to the LCD's DB4~7 pins, clocking with the enable pin.
void LCD4Bit::pushNibble(int value)
{
  GPIO_WriteBit(this->config.gpio, this->config.db4_pin, value & 01 ? Bit_SET : Bit_RESET);
  value >>= 1;
  GPIO_WriteBit(this->config.gpio, this->config.db5_pin, value & 01 ? Bit_SET : Bit_RESET);
  value >>= 1;
  GPIO_WriteBit(this->config.gpio, this->config.db6_pin, value & 01 ? Bit_SET : Bit_RESET);
  value >>= 1;
  GPIO_WriteBit(this->config.gpio, this->config.db7_pin, value & 01 ? Bit_SET : Bit_RESET);
  
  pulseEnablePin();
}

//=====================================
// Push a byte of data to the LCD's DB4~7 pins, in two steps, clocking each with the enable pin.
void LCD4Bit::pushByte(int value)
{
  int val_lower = value & 0x0F;
  int val_upper = value >> 4;
  this->pushNibble(val_upper);
  this->pushNibble(val_lower);
}

//=====================================
void LCD4Bit::commandWriteNibble(int nibble)
{
  GPIO_ResetBits(this->config.gpio, this->config.rs_pin);
  this->pushNibble(nibble);
}

//=====================================
void LCD4Bit::commandWrite(int value)
{
  GPIO_ResetBits(this->config.gpio, this->config.rs_pin);
  this->pushByte(value);
  // TODO: perhaps better to add a delay after EVERY command, here.  many need a delay, apparently.
}

//=====================================
// Print the given character at the current cursor position.
void LCD4Bit::write(u8 value)
{
  //set the RS and RW pins to show we're writing data
  GPIO_SetBits(this->config.gpio, this->config.rs_pin); //digitalWrite(RS, HIGH);

  //let pushByte worry about the intricacies of Enable, nibble order.
  this->pushByte(value);
}

//=====================================
// Clear the LCD screen.
void LCD4Bit::clear()
{
  this->commandWrite(CMD_CLR);
  this->delay(1);
}

//=====================================
// Move the cursor to the given absolute position.  Line numbers start at 0.
void LCD4Bit::cursorTo(int line_num, int x)
{
  // First, put cursor home.
  this->commandWrite(CMD_HOME);

  if (x > this->config.cols-1) {
    x = this->config.cols-1;
  }
  
  switch (line_num)
  {
    case 0:
      x+=0x00;
    break;
    case 1:
      x+= 0x40;
    break;
    case 2:
      x+= 0x14; // Line 3 is an extension of line 1
    break; 
    case 3:
    default:
      x+= 0x54; // Line 4 is an extension of line 2
    break; 
  }
  commandWrite(0x80+x);
}

//=====================================
// Scroll whole display to left
void LCD4Bit::leftScroll(int num_chars, int delay_time)
{
  for (int i=0; i<num_chars; i++) {
    this->commandWrite(CMD_LEFT);
    this->delay(delay_time);
  }
}