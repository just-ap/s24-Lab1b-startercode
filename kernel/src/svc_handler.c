/**
 * @file svc_handler.c
 * @author Nathan Litzinger (nlitzing@andrew.cmu.edu)
 * @brief Contains the C-portion of the SVC handler.
 * @version 0.1
 * @date 2024-02-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdint.h>
#include <debug.h>
#include <svc_num.h>
#include <syscall.h>

#define UNUSED __attribute__((unused))

/**
 * @brief Calls the system call that corresponds with the SVC number found
 * within the SVC instruction that was run (found at an offset from the address
 * of the top of the process stack == psp_top_address.)
 * 
 * @param psp_top_address The address of the top of the process stack == the
 * address of the item that as most recently pushed to the process stack.
 * 
 * @note Lecture 4, Slide 75 gives some pretty comprehensive starter code here
 * to reference.
 * 
 */
void svc_c_handler(UNUSED uint32_t *psp_top_address) {

  /**
   * @brief Define a struct "stack frame" that specifies/defines what fields we
   * will find pushed to the stack according to the APPCS calling convention
   * for Armv7-m. r0 will be at the beginning of the stack frame (first address)
   * and the xPSR will be at the end. If the syscall made required more than
   * four arguments (placed in r0-->r3), then they will be found in the stack
   * immediately after the last element of the pushed stack frame (so after the
   * xPSR register(s)).
   * 
   * @note the xPSR is really just a SINGLE 32-bit register, but where different
   * portions of the 32-bit field correspond to different "sub-fields" (I.e.,
   * the APSR, IPSR, and EPSR).
   * 
   * @note The exact structure of this stack frame can be found in section 2.3.7
   * of the generic m4 manual.
   * 
   */
  typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
  } stack_frame_t;

  /**
   * @brief Create a wrapper around the frame in the stack that came from the
   * context that made the system call in the first place. This is where we will
   * find the arguments provided for the syscall function.
   * 
   */
  stack_frame_t *caller_frame = (stack_frame_t *)psp_top_address;

  /**
   * @brief 1.) Load the SVC instruction word from flash memory. How? We know
   * that for Armv7-m, when the SVC instruction is being decoded, the PC will
   * hold the address of the instruction that follows the SVC instruction. This
   * PC value is what will get pushed to the stack along with the values of all
   * the other registers from the caller context.
   * 
   * @note Pointer arithmetic here: move back the address by one half-word (so
   * subtract 2 byte addresses).
   * 
   * @note The SVC instruction is only a 16-bit instruction!
   * 
   */
  uint16_t svc_instruction = *((uint16_t *)(caller_frame->pc) - 1);
  
  /**
   * @brief Then, extract the SVC number from the bottom eight bits of the SVC
   * instruction.
   * 
   */
  uint8_t svc_number = (uint8_t)(svc_instruction & 0x00FF);

  // __asm volatile( "bkpt" );

  /**
   * @brief Now, call the sycall C function that corresponds with the extracted
   * SVC numbber.
   * 
   * @todo Need to determine what we need to do with the syscall return code.
   * Check the handout.
   * 
   */

  switch ( svc_number ) {

    case (uint8_t)SVC_SBRK: {

      /**
       * @brief The argument for sys_sbrk will be in the first register of the
       * caller frame.
       * 
       */
      typedef struct {
        int increment;
      } sys_sbrk_args_t;

      sys_sbrk_args_t *sys_sbrk_args = (sys_sbrk_args_t *)caller_frame;
      
      void *return_value = sys_sbrk(sys_sbrk_args->increment);

      /**
       * @brief Set the return value r0 by setting its value in the process
       * stack--that way, when it gets restored/pushed back into the registers,
       * the return value will ultimately end up in r0 where the caller function
       * is expecting it.
       * 
       */
      caller_frame->r0 = (uint32_t)return_value;

      break;

    }

    case (uint8_t)SVC_WRITE: {

      /**
       * @brief The arguments for sys_write will be in the first three registers
       * of the caller frame, with the first argument being placed in r0. We
       * will refer to the items on the stack via this struct type. This just
       * gives us a safe, structured, and clear way of accessing our syscall's
       * arguments in the stack frame.
       * 
       */
      typedef struct {
        int file;
        char *buffer;
        int num_bytes;
      } sys_write_args_t;

      sys_write_args_t *sys_write_args = (sys_write_args_t *)caller_frame;

      int return_value = sys_write(sys_write_args->file, sys_write_args->buffer, sys_write_args->num_bytes);

      /**
       * @brief Set the return value r0 by setting its value in the process
       * stack--that way, when it gets restored/pushed back into the registers,
       * the return value will ultimately end up in r0 where the caller function
       * is expecting it.
       * 
       */
      caller_frame->r0 = (uint32_t)return_value;
      
      break;

    }

    case (uint8_t)SVC_READ: {
      
      /**
       * @brief The arguments for sys_read will be in the first three registers
       * of the caller frame, with the first argument being placed in r0. We
       * will refer to the items on the stack via this struct type.
       * 
       */
      typedef struct {
        int file;
        char *buffer;
        int num_bytes;
      } sys_read_args_t;

      sys_read_args_t *sys_read_args = (sys_read_args_t *)caller_frame;

      int return_value = sys_read(sys_read_args->file, sys_read_args->buffer, sys_read_args->num_bytes);

      /**
       * @brief Set the return value r0 by setting its value in the process
       * stack--that way, when it gets restored/pushed back into the registers,
       * the return value will ultimately end up in r0 where the caller function
       * is expecting it.
       * 
       */
      caller_frame->r0 = (uint32_t)return_value;

      break;

    }

    case (uint8_t)SVC_EXIT: {

      /**
       * @brief The argument for sys_exit will be in the first register of the
       * caller frame.
       * 
       */
      typedef struct {
        int status;
      } sys_exit_args_t;

      sys_exit_args_t *sys_exit_args = (sys_exit_args_t *)caller_frame;
      
      sys_exit(sys_exit_args->status);

      /**
       * @brief sys_exit does not have a return value, so nothing to do here.
       * 
       */
      break;

    }

    case (uint8_t)SVC_OS_GET_TICKS: {

      /**
       * @brief os_get_ticks is not called with any arguments, so just call it
       * here.
       * 
       */
      uint32_t return_value = sys_os_get_ticks();

      /**
       * @brief Set the return value r0 by setting its value in the process
       * stack--that way, when it gets restored/pushed back into the registers,
       * the return value will ultimately end up in r0 where the caller function
       * is expecting it.
       * 
       */
      caller_frame->r0 = (uint32_t)return_value;

      break;

    }

    default: {
      DEBUG_PRINT( "Not implemented, svc num %d\n", svc_number);
      // ASSERT( 0 );
    }
  }

  /**
   * @note The return value of this function gets set in whichever case is
   * executed.
   */

}