/* main.c:  performs a single transfer from DSPI_0 to DSPI_1 */
/* Rev 1.0 Sept 14 2004 S.Mihalik  */
/* Rev 2.0 Jan 3 2007 S. Mihalik - Modified to use two SPIs */
/* Rev 2.1 July 20 2007 SM - Modified for MPC551x, changed sysclk (50 MHz) */
/* Rev 2.2 Aug 13 2007 SM - Modified for sysclk of 64 MHz & lenghened CSSCK, ASC*/
/* Rev 2.3 Mar 03 2009 SM - Modified for MPC56xxB/P/S */
/* Rev 2.4 May 22 2009 SM - Simplified code */
/* Rev 2.5 Jun 25 2009 SM - Simplified code */
/* Rev 2.6 Mar 14 2010 SM - modified initModesAndClock, updated header file */
/* Copyright Freescale Semiconductor, Inc. 2007, 2009 All rights reserved. */
#include "Pictus_Header_v1_09.h" /* Use proper include file */

  vuint32_t i = 0;                      /* Dummy idle counter */
  uint16_t RecDataMaster = 0;           /* Data recieved on master SPI */
  uint16_t RecDataSlave  = 0;           /* Data received on slave SPI */

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
  ME.PCTL[4].R = 0x01;            /* MPC56xxB/P/S DSPI0:  select ME.RUNPC[0] */	
  ME.PCTL[5].R = 0x01;            /* MPC56xxB/P/S DSPI1:  select ME.RUNPC[0] */	
/*ME.PCTL[68].R = 0x01; */        /* MPC56xxB/S SIUL:  select ME.RUNPC[0] */	
                                  /* Mode Transition to enter RUN0 mode: */
  ME.MCTL.R = 0x40005AF0;         /* Enter RUN0 Mode & Key */
  ME.MCTL.R = 0x4000A50F;         /* Enter RUN0 Mode & Inverted Key */  
  while (ME.GS.B.S_MTRANS) {}     /* Wait for mode transition to complete */    
                                  /* Note: could wait here using timer and/or I_TC IRQ */
  while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode */
}

void initPeriClkGen(void) {
/* Use the following code as required for MPC56xxB or MPC56xxS:*/
/*CGM.SC_DC[1].R = 0x80; */       /* MPC56xxB/S: Enable peri set 2 sysclk divided by 1 */
}

void disableWatchdog(void) {
  SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
  SWT.SR.R = 0x0000d928; 
  SWT.CR.R = 0x8000010A;     /* Clear watchdog enable (WEN) */
}        

void initDSPI_0(void) {
  DSPI_0.MCR.R = 0x80010001;     /* Configure DSPI_0 as master */
  DSPI_0.CTAR[0].R = 0x780A7727; /* Configure CTAR0  */
  DSPI_0.MCR.B.HALT = 0x0;	     /* Exit HALT mode: go from STOPPED to RUNNING state*/
  SIU.PCR[38].R = 0x0604;        /* MPC56xxP: Config pad as DSPI_0 SOUT output */
  SIU.PCR[39].R = 0x0103;        /* MPC56xxP: Config pad as DSPI_0 SIN input */
  SIU.PCR[37].R = 0x0604;        /* MPC56xxP: Config pad as DSPI_0 SCK output */
  SIU.PCR[36].R = 0x0604;        /* MPC56xxP: Config pad as DSPI_0 PCS0 output */	
}

void initDSPI_1(void) {
  DSPI_1.MCR.R = 0x00010001;     /* Configure DSPI_1 as slave */
  DSPI_1.CTAR[0].R = 0x780A7727; /* Configure CTAR0  */
  DSPI_1.MCR.B.HALT = 0x0;	     /* Exit HALT mode: go from STOPPED to RUNNING state*/
  SIU.PCR[6].R = 0x0503;         /* MPC56xxP: Config pad as DSPI_1 SCK input */
  SIU.PCR[8].R = 0x0103;         /* MPC56xxP: Config pad as DSPI_1 SIN input */
  SIU.PCR[7].R = 0x0604;         /* MPC56xxP: Config pad as DSPI_1 SOUT output*/
  SIU.PCR[5].R = 0x0503;         /* MPC56xxP: Config pad as DSPI_1 PCS0/SS input */
}

void ReadDataDSPI_1(void) {     
  while (DSPI_1.SR.B.RFDF != 1){}  /* Wait for Receive FIFO Drain Flag = 1 */
  RecDataSlave = DSPI_1.POPR.R;    /* Read data received by slave SPI */
  DSPI_1.SR.R = 0x80020000;        /* Clear TCF, RDRF flags by writing 1 to them */
}

void ReadDataDSPI_0(void) {
  while (DSPI_0.SR.B.RFDF != 1){}  /* Wait for Receive FIFO Drain Flag = 1 */
  RecDataMaster = DSPI_0.POPR.R;   /* Read data received by master SPI */
  DSPI_0.SR.R = 0x90020000;        /* Clear TCF, RDRF, EOQ flags by writing 1 */
}

int main(void) {
  initModesAndClks();          /* Initialize mode entries and system clock */
  initPeriClkGen();            /* Initialize peripheral clock generation for DSPIs */
  disableWatchdog();           /* Disable watchdog */
  initDSPI_0();                /* Initialize DSPI_0 as master SPI and init CTAR0 */
  initDSPI_1();                /* Initialize DSPI_1 as Slave SPI and init CTAR0 */
  DSPI_1.PUSHR.R = 0x00001234; /* Initialize slave DSPI_1's response to master */
  DSPI_0.PUSHR.R = 0x08015678; /* Transmit data from master to slave SPI with EOQ */
  ReadDataDSPI_1();            /* Read data on slave DSPI */
  ReadDataDSPI_0();            /* Read data on master DSPI */
  while (1) {i++; }            /* Wait forever */
}