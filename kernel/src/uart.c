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
#include <rcc.h>
#include <uart.h>
#include <uart_polling.h>
#include <nvic.h>
#include <gpio.h>

#define UNUSED __attribute__((unused))

/** @brief The UART register map. */
struct uart_reg_map {
    volatile uint32_t SR;   /**< Status Register */
    volatile uint32_t DR;   /**<  Data Register */
    volatile uint32_t BRR;  /**<  Baud Rate Register */
    volatile uint32_t CR1;  /**<  Control Register 1 */
    volatile uint32_t CR2;  /**<  Control Register 2 */
    volatile uint32_t CR3;  /**<  Control Register 3 */
    volatile uint32_t GTPR; /**<  Guard Time and Prescaler Register */
};

struct kernel_buffer {
    char bytes[512]; // make it a queue instead
    int front;
    int back;
    int size;
};

struct kernel_buffer receive;
struct kernel_buffer send;

/** @brief Base address for UART2 */
#define UART2_BASE  (struct uart_reg_map *) 0x40004400

/** @brief Enable Bit for UART Config register */
#define UART_EN (1 << 13)

/** @brief UART Div Value given specific baud rate */
#define UART_DIV 0x8B

/** @brief Enable Bit for RCC */
#define CLOCK_EN (1 << 17)

/** @brief Enable Bit for Transmitter */
#define TRANSMITTER_EN (1 << 3)

/** @brief Enable Bit for Receiver */
#define RECEIVER_EN (1 << 2)

/** @brief Enable interrupt for transmit data register empty */
#define TXEIE_EN (1<<7)

/** @brief Enable interrupt for read data register not empty */
#define RXNEIE_EN (1<<5)

#define MAX_SIZE 512


void uart_init(int baud){
  (void)baud;

    //Initialize uart register map
    struct uart_reg_map *uart = UART2_BASE;

    //Initialize GPIO pins
    gpio_init(GPIO_A, 2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);
    gpio_init(GPIO_A, 3, MODE_ALT, OUTPUT_OPEN_DRAIN , OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);
  
    //Initialize rcc register map
    struct rcc_reg_map *rcc = RCC_BASE;

    //Enable interrupts for UART
    nvic_irq(38,IRQ_ENABLE);
    
    //Enable clock
    rcc->apb1_enr |= CLOCK_EN;

    //Enable transmission
    uart->CR1 |= TRANSMITTER_EN;

    //Enable receiving
    uart->CR1 |= RECEIVER_EN;

    //Enable UART
    uart->CR1 |= UART_EN;

    //Set Baud Rate Register to UART Div value
    uart->BRR = UART_DIV;

    receive.front = 0;
    receive.back = 0;
    receive.size = 0;

    send.front = 0;
    send.back = 0;
    send.size = 0;

    return;

}

//Write to to_send_bytes starting at to_send_size, increment to_send_size, set to_send to true
int uart_put_byte(char c){
  struct uart_reg_map *uart = UART2_BASE;
  
  if (send.size < MAX_SIZE){
    send.bytes[send.back] = c;
    send.back++;
    send.back = send.back % MAX_SIZE;
    send.size++;
    uart->CR1 |= TXEIE_EN;
    return 0;
  }
  uart->CR1 |= TXEIE_EN;
  return -1;
}

//Write to received_bytes starting at received_size, increment received_size, set received to true
int uart_get_byte(char *c){
  struct uart_reg_map *uart = UART2_BASE;
  if (receive.size <= 0) {
    uart->CR1 |= RXNEIE_EN;
    return -1;
  }
  char byte = receive.bytes[receive.front];
  *c = byte;
  receive.front +=1;
  receive.size-=1;
  receive.front = receive.front % MAX_SIZE;
  uart->CR1 |= RXNEIE_EN;
  return 0;
}

//handout has decent description
void uart_irq_handler(){
  struct uart_reg_map *uart = UART2_BASE;


  int transmit_empty = uart->SR & TXEIE_EN;
  //can transmit
  if (transmit_empty){
      int bytes_to_send = send.size;
      if (bytes_to_send >16){
        bytes_to_send = 16;
      }
      if (send.size < bytes_to_send){
        bytes_to_send = send.size;
      }
      for (int i = 0; i<bytes_to_send; i++){
        transmit_empty = uart->SR & TXEIE_EN;
        if (transmit_empty){
          char byte = send.bytes[send.front];
          uart->DR = byte;
          send.front +=1;
          send.size-=1;
          send.front = send.front % MAX_SIZE;
        }
      }
      if (send.size == 0) {
        uart->CR1 &= ~TXEIE_EN;
      }
  } 

  
  //can receive
  for (int i = 0; i < 16; i++){
    int receive_not_empty = uart->SR & RXNEIE_EN;
    if (receive_not_empty){
      if (receive.size < MAX_SIZE){ 
      
        char c = (char)uart->DR;
        receive.bytes[receive.back] = c;
        receive.back++;
        receive.back = receive.back % MAX_SIZE;
        receive.size++;
      }
    
      if (receive.size == MAX_SIZE){
        uart->CR1 &= ~RXNEIE_EN;

      }
    }
  }
  
  nvic_clear_pending(IRQ_ENABLE);


}

void uart_flush(){
  while(send.size > 0){
    char byte = send.bytes[send.front];
    uart_polling_put_byte(byte);
    send.front +=1;
    send.size-=1;
    send.front = send.front % MAX_SIZE;
  }
  receive.front = 0;
  receive.back = 0;
  receive.size = 0;

  send.front = 0;
  send.back = 0;
  send.size = 0;

}