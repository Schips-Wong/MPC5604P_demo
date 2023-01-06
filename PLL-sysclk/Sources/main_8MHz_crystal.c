/* main.c - PLL example for MPC56xxP */
/* Description:  Set sysclk to FMPLL0 running at 64 MHz & enable CLKOUT */
/* Jan 14 2009 S. Mihalik - Initial version */  
/* May 11 2009 S. Mihalik - Simplified code */  
/* Jun 24 2009 S. Mihalik - Simplified code */
/* Mar 10 2010 S Mihalik - Modified initModesAndClock & updated header file */
/* Copyright Freescale Semiconductor, Inc 2010 All rights reserved. */

#include "Pictus_Header_v1_09.h" /* Use proper include file */

void initModesAndClock(void) {
  ME.MER.R = 0x0000001D;          /* Enable DRUN, RUN0, SAFE, RESET modes */
                                  /* Initialize PLL before turning it on: */
/* Use 2 of the next 4 lines depending on crystal frequency: */
  CGM.CMU_0_CSR.R = 0x000000004;    /* Monitor FXOSC > FIRC/4 (4MHz); no PLL monitor */
  CGM.FMPLL[0].CR.R = 0x02400100;   /* 8 MHz xtal: Set PLL0 to 64 MHz */   
/*CGM.CMU_0_CSR.R = 0x000000000;*//* Monitor FXOSC > FIRC/1 (16MHz); no PLL monitor*/
/*CGM.FMPLL[0].CR.R = 0x12400100;*//* 40 MHz xtal: Set PLL0 to 64 MHz */   
  ME.RUN[0].R = 0x001F0074;       /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL0 */
                                  /* Mode Transition to enter RUN0 mode: */
  ME.MCTL.R = 0x40005AF0;         /* Enter RUN0 Mode & Key */
  ME.MCTL.R = 0x4000A50F;         /* Enter RUN0 Mode & Inverted Key */  
  while (ME.GS.B.S_MTRANS == 1) {}    /* Wait for mode transition to complete */    
                                  /* Notes: */
                                  /* 1. I_TC IRQ could be used here instead of polling */
                                  /*    to allow software to complete other init. */
                                  /* 2. A timer could be used to prevent waiting forever.*/  
  while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode */
                                  /* Note: This verification ensures a SAFE mode */
                                  /*       tranistion did not occur. SW could instead */
                                  /*       enable the safe mode tranision interupt */
}

void initOutputClock(void) {
  CGM.OCEN.B.EN = 1; 	        /* Output Clock enabled (to go to pin) */
  CGM.OCDSSC.B.SELDIV = 2;      /* Output Clock’s selected division is 2**2 = 4 */
  CGM.OCDSSC.B.SELCTL = 0;      /* MPC56xxP/S: Output clock select 16 MHz int RC osc */
  SIU.PCR[22].R = 0x0400;       /* MPC56xxP: assign port PB[6] pad to Alt Func 1 */
                            	/* CLKOUT = 16 MHz IRC/4 = 4MHz */   
 
  CGM.OCDSSC.B.SELCTL = 1;      /* MPC56xxP/S: Assign output clock to XTAL */
                                /* CLKOUT = Fxtal/4 = 2 or 10 MHz for 8 or 40 MHx XTAL */   

  CGM.OCDSSC.B.SELCTL = 2;      /* Assign output clock to FMPLL[0] */
                                /* CLKOUT = 64 MHz/4 = 4MHz */   
}

void disableWatchdog(void) {
  SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
  SWT.SR.R = 0x0000d928; 
  SWT.CR.R = 0x8000010A;     /* Clear watchdog enable (WEN) */
}        

void main (void) {	
  vuint32_t i = 0;	   /* Dummy idle counter */
  
  initModesAndClock(); /* Initialize mode entries and system clock */
  initOutputClock();   /* Initialize Output Clock to 16 M, XOSC, then PLL */  
  disableWatchdog();   /* Disable watchdog */
  while (1) { 
  	i++;
  }	
}