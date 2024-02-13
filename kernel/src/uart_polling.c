#include <gpio.h>
#include <rcc.h>
#include <unistd.h>
#include <uart_polling.h>

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

/** @brief Mask for read data register not empty */
#define RXNE_MASK  (1<<5)

/** @brief Mask for transmit data register empty */
#define TXE_MASK (1<<7)


/**
 * @brief initializes UART to given baud rate with 8-bit word length, 1 stop bit, 0 parity bits
 *
 * @param baud Baud rate
 */
void uart_polling_init (int baud){
    (void)baud;

    //Initialize uart register map
    struct uart_reg_map *uart = UART2_BASE;

    //Initialize GPIO pins
    gpio_init(GPIO_A, 2, MODE_ALT, OUTPUT_PUSH_PULL, OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);
    gpio_init(GPIO_A, 3, MODE_ALT, OUTPUT_OPEN_DRAIN , OUTPUT_SPEED_LOW, PUPD_NONE, ALT7);
  
    //Initialize rcc register map
    struct rcc_reg_map *rcc = RCC_BASE;

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

    return;
}

/**
 * @brief transmits a byte over UART
 *
 * @param c character to be sent
 */
void uart_polling_put_byte (char c){

    struct uart_reg_map *uart = UART2_BASE;

    // checks the 7th bit, which is the TXE bit to see if DR is empty
    // If not, wait till it is empty
    int TXE = uart->SR & TXE_MASK;
    while (!TXE){
        TXE = uart->SR & TXE_MASK;
    }

    //Write character to DR
    uart->DR = c;
    return;
}

/**
 * @brief receives a byte over UART
 */
char uart_polling_get_byte (){
    
    struct uart_reg_map *uart = UART2_BASE;

    // checks the 5th bit, which is the RXNE bit to see if anything has been transferred to DR
    // If not, wait till something is transferred to DR
    int RXNE = uart->SR & RXNE_MASK;
    while (!RXNE){
        RXNE = uart->SR & RXNE_MASK;
    }

    //Read and return transferred character
    char c = (char)uart->DR;
    return c;
}