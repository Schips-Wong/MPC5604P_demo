/* main.c - ADC SW Scan example for MPC56xxP */
/* Description:  Converts inputs ANS0:2 using scan mode (continuous) */
/* Rev 1 Oct 26 2009 S Mihalik - initial version */  
/* Rev 1.1 Mar 14 2010 SM - modified initModesAndClock, updated header file */
/* Copyright Freescale Semiconductor, Inc 2009 All rights reserved. */

#include "Pictus_Header_v1_09.h" /* Use proper header file */
  uint16_t Result[3];             /* ADC conversion results */
  uint16_t ResultInMv[3];         /* ADC conversion results in mv */
  
void initModesAndClock(void) {
  ME.MER.R = 0x0000001D;          /* Enable DRUN, RUN0, SAFE, RESET modes */
                                  /* Initialize PLL before turning it on: */
/* Use 2 of the next 4 lines depending on crystal frequency: */
/*CGM.CMU_0_CSR.R = 0x000000004; */ /* Monitor FXOSC > FIRC/4 (4MHz); no PLL monitor */
/*CGM.FMPLL[0].CR.R = 0x02400100;*/ /* 8 MHz xtal: Set PLL0 to 64 MHz */   
  CGM.CMU_0_CSR.R = 0x000000000;  /* Monitor FXOSC > FIRC/1 (16MHz); no PLL monitor*/
  CGM.FMPLL[0].CR.R = 0x12400100; /* 40 MHz xtal: Set PLL0 to 64 MHz */   
  ME.RUN[0].R = 0x001F0074;       /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL0 */
  ME.RUNPC[1].R = 0x00000010; 	  /* Peri. Cfg. 1 settings: only run in RUN0 mode */
  ME.PCTL[32].R = 0x01; 	      /* MPC56xxB/P/S ADC 0: select ME.RUNPC[1] */	
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

void initPeriClkGen(void) {
}

void main (void) {	
  vuint32_t i = 0;	      /* Dummy idle counter */

  initModesAndClock();    /* Initialize mode entries and system clock */
  disableWatchdog();      /* Disable watchdog */
  initPeriClkGen();       /* Initialize peripheral clock generation for DSPIs */

  SIU.PCR[23].R = 0x2400;         /* MPC56xxP: Initialize PB[7] as AN0 */
  SIU.PCR[24].R = 0x2400;         /* MPC56xxP: Initialize PB[8] as AN1 */
  SIU.PCR[33].R = 0x2400;         /* MPC56xxP: Initialize PC[1] as AN2 */

  ADC_0.MCR.R = 0x20000000;       /* Initialize ADC0 for scan mode */
  ADC_0.NCMR[0].R = 0x00000007;   /* Select ANS0:2 inputs for conversion */
  ADC_0.CTR[0].R = 0x00008606;    /* Conversion times for 32MHz ADClock */
  ADC_0.MCR.B.NSTART=1;           /* Trigger normal conversions for ADC0 */
   
  while (1) { 
    while (ADC_0.CDR[2].B.VALID != 1) {};               /* Wait for last scan to complete */
    Result[0]= ADC_0.CDR[0].B.CDATA;                    /* Read ANS0 conversion result data */
    Result[1]= ADC_0.CDR[1].B.CDATA;                    /* Read ANS1 conversion result data */
    Result[2]= ADC_0.CDR[2].B.CDATA;                    /* Read ANS2 conversion result data */
    ResultInMv[0] = (uint16_t) (5000*Result[0]/0x3FF);  /* Converted result in mv */
    ResultInMv[1] = (uint16_t) (5000*Result[1]/0x3FF);  /* Converted result in mv */
    ResultInMv[2] = (uint16_t) (5000*Result[2]/0x3FF);  /* Converted result in mv */
   	i++;
  }	
}
