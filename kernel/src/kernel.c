/**
 * @file     kernel.c
 *
 * @brief    Kernel entry point
 *
 * @date     20 Oct 2019
 *
 * @author   Benjamin Huang <zemingbh@andrew.cmu.edu>
 */

#include "arm.h"
#include "kernel.h"
#include "printk.h"
#include "uart.h"
#include "timer.h"

#define UART_BAUD_RATE 115200

int kernel_main( void ) {

  /**
   * @brief Define what frequency the SysTick timer should fire interrupts at.
   * It is VERY IMPORTANT that this frequency is 1000--as otherwise, the number
   * of "milliseconds" returned by os_get_ticks won't make sense.
   * 
   * @todo I don't even think this should be configurable as an argument. If
   * anything, the frequency should just refer to the frequency the kernel runs
   * at (<= the rate the SysTick timer is running at). Maybe that's the idea--in
   * which case, the argument should be renamed "kernel_frequency."
   * 
   */
  static const int SYSTICK_FREQUENCY_HZ = 1000;

  init_349(); // DO NOT REMOVE THIS LINE
  uart_init( UART_BAUD_RATE );
  timer_start(SYSTICK_FREQUENCY_HZ);
  printk("Kernel Initialized, entering user mode.\n"); //sudo minicom -D /dev/serial/by-id/[tab] -b 115200
  enter_user_mode();
  return 0;

}