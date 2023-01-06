/* main.c - Example of flash configuration plus using branch target buffers */
/* Copyright Freescale Semiconductor, Inc 2009 All rights reserved. */
/* Rev 0.1 May 21 2009 S. Mihalik - Initial version based on AN2865 example */
/* Rev 0.2 Sep 11 2009 S. Mihalik - corrected BUCSR value to 0x0201 */
/* Rev 0.3 Mar 11 2010 SM - corrected FLASH_CONFIG_DATA to 0x1084126F */

#include "Pictus_Header_v1_09.h" /* Include appropriate header file  */

#define FLASH_CONFIG_DATA 0x1084126F    /* MPC56xxP flash config value for 64 MHz */
#define FLASH_CONFIG_REG CFLASH.PFCR0.R /* Flash config. register address */
#define FLASH_ACCESS_PROT_DATA 0x00FE000D    /* MPC56xxB flash access prot. value */
#define FLASH_ACCESS_PROT_REG CFLASH.FAPR.R  /* Flash Access Prot. Reg. address */

asm void enable_accel_BTB(void) {
  li      r0, 0      /* Enable branch acceleration (HID[PBRED]=0) */
  mtHID0  r0	
  li      r0, 0x0201 /* Invalidate Branch Target Buffers and enable them */
  mtBUCSR r0
}

int main(void) {

  uint32_t i=0; /* Dummy idle counter */
  
/* NOTE: Structures are default aligned on a boundary which is a multiple of */
/*       the largest sized element, 4 bytes in this case.  The first two */
/*       instructions are 4 bytes, so the last instruction is duplicated to  */
/*       avoid the compiler adding padding of 2 bytes before the instruction. */
  uint32_t mem_write_code_vle [] = {
    0x54640000, /* e_stw r3,(0)r4 machine code: writes r3 contents to addr in r4 */
    0x7C0006AC, /* mbar machine code: ensure prior store completed */
    0x00040004  /* 2 se_blr's machine code: branches to return address in link reg. */
    };

  typedef void (*mem_write_code_ptr_t)(uint32_t, uint32_t);
          /* create a new type def: a func pointer called mem_write_code_ptr_t */
          /* which does not return a value (void) */
          /* and will pass two 32 bit unsigned integer values */
          /* (per EABI, the first parameter will be in r3, the second r4) */

  (*((mem_write_code_ptr_t)mem_write_code_vle))  /* cast mem_write_code as func ptr*/
                                 /* * de-references func ptr, i.e. converts to func*/
     (FLASH_CONFIG_DATA,            /* which passes integer (in r3) */
      (uint32_t)&FLASH_CONFIG_REG); /* and address to write integer (in r4) */

  (*((mem_write_code_ptr_t)mem_write_code_vle))  /* cast mem_write_code as func ptr*/
                                 /* * de-references func ptr, i.e. converts to func*/
     (FLASH_ACCESS_PROT_DATA,            /* which passes integer (in r3) */
      (uint32_t)&FLASH_ACCESS_PROT_REG); /* and address to write integer (in r4) */

  enable_accel_BTB();            /* Enable branch accel., branch target buffers */

  while(1) {i++;}                /* Wait forever */
}



