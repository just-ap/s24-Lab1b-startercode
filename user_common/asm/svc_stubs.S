/**
 * @file   swi_stubs.S
 *
 * @brief  Stub functions for newlib and 349libc syscalls
 *
 * @date
 * @author
 */

.cpu cortex-m4
.syntax unified
.section .swi_stub
.thumb

#include "../../kernel/include/svc_num.h"

.global _start
_start:
  SVC SVC_SCHD_START
  bx lr

.global _sbrk
_sbrk:
  SVC SVC_SBRK
  bx lr

.global _write
_write:
  /* Call the SVC instruction with the number that we define to correspond with
  a "write" syscall (in svc_num.h).
  */
  SVC SVC_WRITE
  /* Not sure why we're using bx over the other branch instructions, but will
  clarify this later.
  */
  bx lr

.global _close
_close:
  SVC SVC_CLOSE
  bx lr

.global _fstat
_fstat:
  SVC SVC_FSTAT
  bx lr

.global _isatty
_isatty:
  SVC SVC_ISATTY
  bx lr

.global _lseek
_lseek:
  SVC SVC_LSEEK
  bx lr

.global _read
_read:
  SVC SVC_READ
  bx lr

.global _gettimeofday
_gettimeofday:
  SVC SVC_TIME
  bx lr

.global _times
_times:
  SVC SVC_THR_TIME
  bx lr

.global _getpid
_getpid:
  SVC SVC_GET_PID
  bx lr

.global _kill
_kill:
  SVC SVC_KILL
  bx lr

.global _exit
_exit:
  SVC SVC_EXIT
  bx lr

/* Haven't defined SVC numbers for servo syscall functions in svc_num.h yet. */

.global servo_enable
servo_enable:
  bkpt

.global servo_set
servo_set:
  bkpt

.global _os_get_ticks
_os_get_ticks:
  SVC SVC_OS_GET_TICKS
  bx lr