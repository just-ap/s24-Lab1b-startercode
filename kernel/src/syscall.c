/**
 * @file 
 *
 * @brief      
 *
 * @date       
 *
 * @author     
 */

#include <unistd.h>
#include <syscall.h>
#include <printk.h>
#include <uart.h>
#include <timer.h>

#define UNUSED __attribute__((unused))
// the following macro is for IO read and write
#define EOF_CHAR 4      // End-of-transmission character (Ctrl-D)
#define BACKSPACE '\b'
#define NEWLINE '\n'
// #define CARRIAGE_RETURN '\r'

extern char __heap_low;
extern char __heap_top;

char *current_break = &__heap_low;

void *sys_sbrk(UNUSED int incr){
  char *old_break = current_break;
  
  if (current_break + incr >= &__heap_top || current_break + incr < &__heap_low){
    return (void *)-1;
  }
  current_break += incr;
  return (void*) old_break;
}

/**
 * @brief sys_write() is used for write syscall. it will first check we are 
 * writing to stdout (stdin = 0, stdout = 1, stderr = 2), if not return -1, and 
 * then output each byte in the buffer
 * 
 * @param file 
 * @param ptr 
 * @param len 
 * @return int 
 */
int sys_write(UNUSED int file, UNUSED char *ptr, UNUSED int len){

  /**
   * @brief Right now, the only file descriptor type we support is for STDOUT.
   * 
   */
  if (file != 1) {
    return -1;
  }

  int count = 0;
  // __asm volatile( "bkpt" );
  while(count < len){
    int put_byte_status = uart_put_byte(ptr[count]);
    while (put_byte_status == -1) {
      put_byte_status = uart_put_byte(ptr[count]);
    }
    count++;
  }

  return count;
}

/**
 * @brief sys_read() is used for read syscall. It will first check we are 
 * reading from stdin (stdin = 0, stdout = 1, stderr = 2) and read the bytes one
 * by one and echo back them
 * 
 * @param file 
 * @param ptr 
 * @param len 
 * @return int 
 */
int sys_read(UNUSED int file, UNUSED char *ptr, UNUSED int len){
  /**
   * @brief first we need to make sure the file is stdin, otherwise
   * return -1; for file argument, stdin = 0, stdout = 1, stderr = 2.
   * 
   */

  if (file != 0) { 
    return -1;
  }

  int count = 0;
  char c = -1;
  // __asm volatile( "bkpt" );
  while(count < len){
    // __asm volatile( "bkpt" );
    int get_byte_status = uart_get_byte(&c);
    while (get_byte_status == -1) {
      get_byte_status = uart_get_byte(&c);
    }
    // uart get byte is pooping the bed and returning -1; so we have to write
    // this to call it until it can read?
    // __asm volatile( "bkpt" );
    if (c == EOF_CHAR) { // if it's end-of-transmission return count
      return count;
    } else if (c == BACKSPACE){ // if it's baskspace echo '\b \b' 
      char ctemp[4] = "\b \b";
      for (int i = 0; i < 3; i++) {
        int put_byte_status = uart_put_byte(ctemp[i]);
        while (put_byte_status == -1) {
          put_byte_status = uart_put_byte(ctemp[i]);
        }
      }
      count--;
    } else if (c == NEWLINE) {
      ptr[count++] = NEWLINE;
      int put_byte_status = uart_put_byte(NEWLINE);
      while (put_byte_status == -1) {
        put_byte_status = uart_put_byte(NEWLINE);
      }
      return count;
    } else {
      ptr[count++] = c;
      int put_byte_status = uart_put_byte(c);
      while (put_byte_status == -1) {
        put_byte_status = uart_put_byte(c);
      }
      // __asm volatile( "bkpt" );
    }
    
  }
  return count;
}

void sys_exit(UNUSED int status){
  printk("the exit code is %d\n", status); //print exit code
  uart_flush(); //flush the uart
  timer_stop(); //disable timer
  return;
}

uint32_t sys_os_get_ticks() {

  /**
   * @brief SysTick should run at 1000 Hz, and increment a counter each time.
   * Grab that variable's value here.
   * 
   */
  return systick_get_millis();

}