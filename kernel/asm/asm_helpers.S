/* Contains a handful of assembly functions used by the kernel. */

.cpu cortex-m4
.syntax unified
.thumb

.section .text

.thumb_func
.global enter_user_mode
enter_user_mode:
  /* This is where we set the PSP register's value to the psp stack top
  according to the linker template file that we're given. This basically just
  defines what address the PSP will START AT at boot.
  */
  LDR R0, =__psp_stack_top
  MSR PSP, R0
  /* In _ctr0, we update the CONTROL register so that the SP points to the PSP
  and so that the CPU goes into unpriveleged mode --> effectively "Thread Mode"
  Within that function, we then call ctr0, which is what calls the "main()"
  function of the user program.
  */
  B _crt0
  