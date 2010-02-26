/********************************* OUTROOT.COM ********************************
* Author             : Kevin Darlington (http://outroot.com/)
* Date               : 02/21/2010
* Description        : Main program body
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f10x.h"
#include "../../stm32lcd4bit.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
static __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
void delay(__IO u32 nTime);

/* Private functions ---------------------------------------------------------*/

/* Maple code ----------------------------------------------------------------*/

#define SYSTICK_CNT       0xE000E018  // Current value register
#define MAPLE_RELOAD_VAL     72000
#define CYCLES_PER_MICROSECOND  72
#define FUDGE                   42

static inline u32 systick_get_count(void)
{
  return (u32)*(volatile u32*)SYSTICK_CNT;
}

void udelay(__IO u32 us)
{
  u32 target;
  u32 last, cur, count;
  /* fudge factor hacky hack hack for function overhead  */
  target = us * CYCLES_PER_MICROSECOND - FUDGE;

  /* Get current count */
  last = systick_get_count();
  cur = systick_get_count();
  count = last;
  while ((count-cur) <= target) {
    cur = systick_get_count();

    /* check for overflow  */
    if (cur > last) {
      count += MAPLE_RELOAD_VAL;
    }
    last = cur;
  }
}

int main(void)
{
#ifdef DEBUG
  debug();
#endif

  SystemInit();
  
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemFrequency / 1000))
  { 
    /* Capture error */ 
    while (1);
  }

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  LCD4Bit_Config lcd_config(
    GPIOB,
    GPIO_Pin_11, // RS
    GPIO_Pin_12, // ENABLE
    GPIO_Pin_7,  // DB4
    GPIO_Pin_8,  // DB5
    GPIO_Pin_9,  // DB6
    GPIO_Pin_10,  // DB7
    20, // cols
    4 // rows
  );
  
  LCD4Bit lcd(lcd_config, &delay, &udelay);  
  lcd.init();
  lcd.cursorTo(0, 0);
  lcd.print("Line1");
  lcd.cursorTo(1, 0);
  lcd.print("Line2");
  lcd.cursorTo(2, 0);
  lcd.print("Line3");
  lcd.cursorTo(3, 0);
  lcd.print("Line4");
  
  delay(2000);

  while (1)
  {    
    lcd.commandWrite(0x80);
    
    for (int i=0; i <= 20; i++) {
    
      lcd.commandWrite(0x80+i);
      //lcd.cursorTo(1,i);
      lcd.write(255);
    
      lcd.commandWrite(0xC0+i);
      //lcd.cursorTo(2,i);
      lcd.write(255);
    
      lcd.commandWrite(0x94+i);
      lcd.write(255);
    
      lcd.commandWrite(0xD4+i);
      lcd.write(255);
    
      //delay(1);
    
    }
    
    lcd.commandWrite(0x80);
    
    for (int i=0; i <= 20; i++) {
    
      lcd.commandWrite(0x80+i);
      //lcd.cursorTo(1,i);
      lcd.write(23);
    
      lcd.commandWrite(0xC0+i);
      //lcd.cursorTo(2,i);
      lcd.write(23);
    
      lcd.commandWrite(0x94+i);
      lcd.write(23);
    
      lcd.commandWrite(0xD4+i);
      lcd.write(23);
    
      //delay(1);
    }
  }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay(__IO u32 nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
