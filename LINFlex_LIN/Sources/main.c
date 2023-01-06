/* main.c: LINFlex program for MPC56xxP */
/* Description: Transmit one message from FlexCAN 0 buf. 0 to FlexCAN C buf. 1 */
/* Rev Oct 30 2009 SM - initial version */
/* Rev Mar 14 1020 SM - Modified initModesAndClks, updated header */ 

#include "Pictus_Header_v1_09.h" /* Use proper include file */

void initModesAndClks(void) {
  ME.MER.R = 0x0000001D;          /* Enable DRUN, RUN0, SAFE, RESET modes */
                                  /* Initialize PLL before turning it on: */
/* Use 2 of the next 4 lines depending on crystal frequency: */
/*CGM.CMU_0_CSR.R = 0x000000004; */ /* Monitor FXOSC > FIRC/4 (4MHz); no PLL monitor */
/*CGM.FMPLL[0].CR.R = 0x02400100;*/ /* 8 MHz xtal: Set PLL0 to 64 MHz */   
  CGM.CMU_0_CSR.R = 0x000000000;  /* Monitor FXOSC > FIRC/1 (16MHz); no PLL monitor*/
  CGM.FMPLL[0].CR.R = 0x12400100; /* 40 MHz xtal: Set PLL0 to 64 MHz */   
  ME.RUN[0].R = 0x001F0074;       /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL */
  ME.RUNPC[1].R = 0x00000010; 	  /* Peri. Cfg. 1 settings: only run in RUN0 mode */
  ME.PCTL[48].R = 0x01;           /* MPC56xxB/P/S LINFlex 0: select ME.RUNPC[1] */	

                                  /* Mode Transition to enter RUN0 mode: */
  ME.MCTL.R = 0x40005AF0;         /* Enter RUN0 Mode & Key */
  ME.MCTL.R = 0x4000A50F;         /* Enter RUN0 Mode & Inverted Key */  
  while (ME.GS.B.S_MTRANS) {}     /* Wait for mode transition to complete */    
                                  /* Note: could wait here using timer and/or I_TC IRQ */
  while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode */
}

void initPeriClkGen(void) {
}

void disableWatchdog(void) {
  SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
  SWT.SR.R = 0x0000d928; 
  SWT.CR.R = 0x8000010A;     /* Clear watchdog enable (WEN) */
}        

void initLINFlex_0 (void) {
 
  LINFLEX_0.LINCR1.B.INIT = 1;    /* Put LINFlex hardware in init mode */
  LINFLEX_0.LINCR1.R= 0x00000311; /* Configure module as LIN master & header */
  LINFLEX_0.LINIBRR.B.DIV_M= 383; /* Mantissa baud rate divider component */
  LINFLEX_0.LINFBRR.B.DIV_F = 16; /* Fraction baud rate divider comonent */
  LINFLEX_0.LINCR1.R= 0x00000310; /* Configure module as LIN master & header */
  SIU.PCR[18].R = 0x0400;         /* MPC56xxP: Configure port B2 as LIN0TX */
  SIU.PCR[19].R = 0x0503;         /* MPC56xxP: Configure port B3 as LIN0RX */
  SIU.PSMI[31].R = 0;             /* MPC56xxP: LIN0 Pad select mux port B3 */
}

void transmitLINframe (void) {
  LINFLEX_0.BDRM.R = 0x2020206F;  /* Load buffer data most significant bytes */
  LINFLEX_0.BDRL.R = 0x6C6C6548;  /* Load buffer data least significant bytes */
  LINFLEX_0.BIDR.R = 0x00001E35;  /* Init header:  ID=0x35, 8 B, Tx, enh. cksum*/ 
  LINFLEX_0.LINCR2.B.HTRQ = 1;    /* Request header transmission */
}

void main(void) {
  volatile uint32_t IdleCtr = 0; 
  
  initModesAndClks();      /* Initialize mode entries */
  initPeriClkGen();        /* Initialize peripheral clock generation for LINFlex */
  disableWatchdog();       /* Disable watchdog */
  initLINFlex_0();         /* Initialize FLEXCAN 0 as master */
  transmitLINframe();     /* Transmit one frame from master */
  while (1) {IdleCtr++;}   /* Idle loop: increment counter */
}

