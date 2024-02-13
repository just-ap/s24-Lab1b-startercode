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

#define UART_BAUD_RATE 138

int kernel_main( void ) {

  init_349(); // DO NOT REMOVE THIS LINE
  uart_init( UART_BAUD_RATE );
  enter_user_mode();

  return 0;

}
