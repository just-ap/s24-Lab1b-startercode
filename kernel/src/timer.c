/**
 * @file timer.c
 * @author Nathan Litzinger (nlitzing@andrew.cmu.edu)
 * @brief 
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <timer.h>
#include <unistd.h>
#include <stdint.h>
#include <printk.h>

#define UNUSED __attribute__((unused))

int timer_start(UNUSED int frequency){

  /**
   * @brief Define any constants needed for this function.
   * 
   */
  static const uint32_t CORE_FREQUENCY_HZ = 84000000;
  
  /**
   * @brief According to 4.5.5 of the programming manual, the correct
   * initialization sequence for the timer is to set the reload value, clear the
   * current value, and then configure the timer via its control register.
   * 
   */

  /**
   * @brief Set the reload value as a function of the specified frequency. More
   * details in 4.5.2 of the programming manual.
   * 
   */
  uint32_t *STK_RELOAD_ADDR = (uint32_t *)0xE000E014;
  // Clear any existing value in the RELOAD value field (bottom 24 bits).
  *STK_RELOAD_ADDR &= 0xFF000000;
  // Compute reload value from frequency.
  uint32_t STK_RELOAD_VALUE = ((uint32_t)CORE_FREQUENCY_HZ/frequency) - 1;
  // Mask out only the bottom 24 bit values of the computed reload value so that
  // we don't overwrite any bits in the reserved space. Should really return an
  // error if there's some kind of overflow.
  STK_RELOAD_VALUE &= 0x00FFFFFF;
  // Update the reload address field with the new reload value.
  *STK_RELOAD_ADDR |= STK_RELOAD_VALUE;

  /**
   * @brief Set the SysTick timer's current value register to 0. More details in
   * 4.5.3 of the programming manual.
   * 
   */
  uint32_t *STK_VALUE_ADDR = (uint32_t *)0xE000E018;
  // Zero out the lower 24 bits == the current value portion of the register.
  *STK_VALUE_ADDR &= 0xFF000000;
  
  /**
   * @brief Configure the SysTick control and status register (STK_CTRL).
   * Basically, just need to set the last three bits to "1" to enabling
   * counting, enable interrupts, and select the processor clock as the source.
   * More details in 4.5.1 of the ST programming manual.
   * 
   */
  uint32_t *STK_CTRL_ADDR = (uint32_t *)0xE000E010;

  uint32_t STK_CTRL_VALUE = 0x7; // set last 3 bits to be 111

  *STK_CTRL_ADDR |= STK_CTRL_VALUE;

  return 0;
}

void timer_stop(){

  /**
   * @brief Read in the STK_CTRL register's current value. The STK_CTRL register
   * is mapped to the address 0xE000E010.
   * 
   */
  uint32_t *STK_CTRL_ADDR = (uint32_t *)0xE000E010;
  uint32_t STK_CTRL_VALUE = *STK_CTRL_ADDR;

  /**
   * @brief Stop the SysTick timer from counting by setting the ENABLE bit in
   * the STK_CTRL register to 0. Do this by first masking out the value and then
   * by bitwise-oring in the value we want (0). Actually don't have to bitwise
   * in a 0 as it is already 0!
   * 
   */
  STK_CTRL_VALUE = STK_CTRL_VALUE & (uint32_t)0xFFFFFFFE; // 111..1110 (1110 == 0xE)
  
  /**
   * @brief Write back the updated STK_CTRL_VALUE to the STK_CTRL register at
   * the address above.
   * 
   */
  *STK_CTRL_ADDR = STK_CTRL_VALUE;
  
  return;

}

/**
 * @brief Static variable that will be incremented for every millisecond that
 * passes.
 * 
 */
static uint32_t millis = 0;

uint32_t systick_get_millis() {
  return millis;
}

void systick_c_handler(){

  /**
   * @brief Increment millis.
   * 
   */
  millis++;

  // printk("From SysTick Handler!\n");

}