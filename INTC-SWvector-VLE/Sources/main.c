/* main.c - Software vector mode program using C isr */
/* Jan 15, 2009 S.Mihalik- Initial version based on previous AN2865 example */
/* Apr 13 2009 S. Mihalik- Simplifed by removing unneeded sysclk code */
/* May 08 2009 S. Mihalik- disabled watchdog */
/* Jul 03 2009 S Mihalik - Simplified code */
/* Mar 14 1020 SM - Modified initModesAndClks, updated header */ 
/* Copyright Freescale Semiconductor, Inc. 2009. All rights reserved. */

#include "Pictus_Header_v1_09.h" /* Use proper include file such as mpc5554.h */

extern IVOR4Handler();
extern uint32_t __IVPR_VALUE; /* Interrupt Vector Prefix vaue from link file*/
extern const vuint32_t IntcIsrVectorTable[];

    uint32_t Pit1Ctr = 0;   /* Counter for PIT 1 interrupts */
    uint32_t SWirq4Ctr = 0;	/* Counter for software interrupt 4 */

void initModesAndClock(void) {
  ME.MER.R = 0x0000001D;          /* Enable DRUN, RUN0, SAFE, RESET modes */
                                  /* Initialize PLL before turning it on: */
/* Use 2 of the next 4 lines depending on crystal frequency: */
/*CGM.CMU_0_CSR.R = 0x000000004; */ /* Monitor FXOSC > FIRC/4 (4MHz); no PLL monitor */
/*CGM.FMPLL[0].CR.R = 0x02400100;*/ /* 8 MHz xtal: Set PLL0 to 64 MHz */   
  CGM.CMU_0_CSR.R = 0x000000000;  /* Monitor FXOSC > FIRC/1 (16MHz); no PLL monitor*/
  CGM.FMPLL[0].CR.R = 0x12400100; /* 40 MHz xtal: Set PLL0 to 64 MHz */   
  ME.RUN[0].R = 0x001F0074;       /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL */
  ME.RUNPC[1].R = 0x00000010; 	  /* Peri. Cfg. 1 settings: only run in RUN0 mode */
  ME.PCTL[92].R = 0x01;           /* PIT, RTI: select ME_RUN_PC[1] */
                                  /* Mode Transition to enter RUN0 mode: */
  ME.MCTL.R = 0x40005AF0;         /* Enter RUN0 Mode & Key */
  ME.MCTL.R = 0x4000A50F;         /* Enter RUN0 Mode & Inverted Key */  
  while (ME.GS.B.S_MTRANS) {}     /* Wait for mode transition to complete */    
                                  /* Note: could wait here using timer and/or I_TC IRQ */
  while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode */
}

void disableWatchdog(void) {
  SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
  SWT.SR.R = 0x0000d928; 
  SWT.CR.R = 0x8000010A;     /* Clear watchdog enable (WEN) */
}        

asm void initIrqVectors(void) {
  lis	   r3, __IVPR_VALUE@h   /* IVPR value is passed from link file */
  ori      r3, r3, __IVPR_VALUE@l 
  mtivpr   r3									 
}

void initINTC(void) {	
  INTC.MCR.B.HVEN = 0;       /* Initialize for SW vector mode */
  INTC.MCR.B.VTES = 0;       /* Use default vector table 4B offsets */
  INTC.IACKR.R = (uint32_t) &IntcIsrVectorTable[0];    /* INTC ISR table base */
}

void initPIT(void) {
  PIT.PITMCR.R = 0x00000001;       /* Enable PIT and configure stop in debug mode */
  PIT.CH[1].LDVAL.R = 64000;       /* Timeout= 64K sysclks x 1sec/64M sysclks= 1 ms */
  PIT.CH[1].TCTRL.R = 0x000000003; /* Enable PIT1 interrupt & start PIT counting */ 
  INTC.PSR[60].R = 0x01;           /* PIT 1 interrupt vector with priority 1 */
}

void initSwIrq4(void) {
  INTC.PSR[4].R = 2;		/* Software interrupt 4 IRQ priority = 2 */
}

void enableIrq(void) {
  INTC.CPR.B.PRI = 0;          /* Lower INTC's current priority */
  asm(" wrteei 1");	    	   /* Enable external interrupts */
}

void main (void) {	
  vuint32_t i = 0;			    /* Dummy idle counter */

  initModesAndClock();  /* MPC56xxP/B/S: Initialize mode entries, set sysclk = 64 MHz*/
  disableWatchdog();    /* Disable watchdog */
  initIrqVectors();		/* Initialize exceptions: only need to load IVPR */
  initINTC();			/* Initialize INTC for software vector mode */
  initPIT();		  	/* Initialize PIT1 for 1KHz IRQ, priority 2 */
  initSwIrq4();			/* Initialize software interrupt 4 */
  enableIrq();		   	/* Ensure INTC current prority=0 & enable IRQ */
 
  while (1) { 
  	i++;
  }	
}

void Pit1ISR(void) {
  Pit1Ctr++;                  /* Increment interrupt counter */
  if ((Pit1Ctr & 1)==0) {     /* If PIT1Ctr is even*/
    INTC.SSCIR[4].R = 2;      /*  then nvoke software interrupt 4 */
  }
  PIT.CH[1].TFLG.B.TIF = 1;   /* CLear PIT 1 flag by writing 1 */
}

void SwIrq4ISR(void) {
  SWirq4Ctr++;		 		  /* Increment interrupt counter */
  INTC.SSCIR[4].R = 1;		  /* Clear channel's flag */  
}

