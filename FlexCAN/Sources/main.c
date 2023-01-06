/* main.c: FlexCAN program */
/* Description: Transmit one message from FlexCAN 0 buf. 0  */
/* Rev 0.1 Jan 16, 2006 S.Mihalik, Copyright Freescale, 2006. All Rights Reserved */
/* Rev 0.2 Jun 6 2006 SM - changed Flexcan A to C & enabled 64 msg buffers */
/* Rev 0.3 Jun 15 2006 SM - 1. Made globals uninitialized */
/*         2. RecieveMsg function:  read CANx_TIMER, removed setting buffer's CODE*/
/*         3. added idle loop code for smoother Nexus trace */
/*         4. modified for newer Freescale header files (r 16) */
/* Rev 0.4 Aug 11 2006 SM - Removed redundant CAN_A.MCR init */
/* Rev 0.5 Jan 31 2007 SM - Removed other redundant CAN_C.MCR init */
/* Rev 0.6 Mar 08 2007 SM - Corrected init of MBs- cleared 64 MBs, instead of 63 */
/* Rev 0.7 Jul 20 2007 SM - Changes for MPC5510 */
/* Rev 0.8 May 15 2008 SM - Changes for new header file symbols */
/* Rev 0.9 May 22 2009 SM - Changes for MPC56xxB/P/S */
/* Rev 1.0 Mar 14 1020 SM - Modified initModesAndClks, updated header */ 
/* NOTE!! structure canbuf_t DATA in header file modified to allow byte addressing*/

#include "Pictus_Header_v1_09.h" /* Use proper include file  */

uint32_t RxCODE;               /* Received message buffer code */
uint32_t RxID;                 /* Received message ID */
uint32_t RxLENGTH;             /* Recieved message number of data bytes */
uint8_t  RxDATA[8];            /* Received message data string*/
uint32_t RxTIMESTAMP;          /* Received message time */                         

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
  ME.PCTL[16].R = 0x01;           /* MPC56xxB/P/S FlexCAN0:  select ME.RUNPC[1] */	
  ME.PCTL[26].R = 0x01;           /* MPC56xxP SafetyPort: select ME.RUNPC[1] */	
                                  /* Mode Transition to enter RUN0 mode: */
  ME.MCTL.R = 0x40005AF0;         /* Enter RUN0 Mode & Key */
  ME.MCTL.R = 0x4000A50F;         /* Enter RUN0 Mode & Inverted Key */  
  while (ME.GS.B.S_MTRANS) {}     /* Wait for mode transition to complete */    
                                  /* Note: could wait here using timer and/or I_TC IRQ */
  while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode */
}

void initPeriClkGen(void) {
  CGM.AC2SC.R = 0x04000000;  /* MPC56xxP Safety Port: Select PLL0 for aux clk 0  */
  CGM.AC2DC.R = 0x80000000;  /* MPC56xxP Safety Port: Enable aux clk 0 div by 1 */
}

void disableWatchdog(void) {
  SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
  SWT.SR.R = 0x0000d928; 
  SWT.CR.R = 0x8000010A;     /* Clear watchdog enable (WEN) */
}        

void initSAFEPORT (void) {
  uint8_t   i;

  SAFEPORT.MCR.R = 0x5000001F;       /* Put in Freeze Mode & enable all 32 msg bufs */
/* Use 1 of the next 2 lines depending on crystal frequency: */  
/*SAFEPORT.CR.R = 0x04DB0006;  */    /* Configure for 8MHz OSC, 100KHz bit time */
  SAFEPORT.CR.R = 0x18DB0006;        /* Configure for 40MHz OSC, 100KHz bit time */
  for (i=0; i<32; i++) {
    SAFEPORT.BUF[i].CS.B.CODE = 0;   /* Inactivate all message buffers */
  } 
  SAFEPORT.BUF[4].CS.B.IDE = 0;      /* MB 4 will look for a standard ID */
  SAFEPORT.BUF[4].ID.B.STD_ID = 555; /* MB 4 will look for ID = 555 */
  SAFEPORT.BUF[4].CS.B.CODE = 4;     /* MB 4 set to RX EMPTY */
  SAFEPORT.RXGMASK.R = 0x1FFFFFFF;   /* Global acceptance mask */
  SIU.PCR[14].R = 0x0624;            /* MPC56xxP: Config port A14 as CAN1TX, open drain */
  SIU.PCR[15].R = 0x0900;            /* MPC56xxP: Configure port A15 as CAN1RX */
  SAFEPORT.MCR.R = 0x0000001F;       /* Negate SAFETY PORT halt state for 32 MB */
}

void initCAN_0 (void) {
  uint8_t   i;
  CAN_0.MCR.R = 0x5000001F;       /* Put in Freeze Mode & enable all 32 msg bufs */
/* Use 1 of the next 2 lines depending on crystal frequency: */  
/*CAN_0.CR.R = 0x04DB0006; */     /* Configure for 8MHz OSC, 100KHz bit time */
  CAN_0.CR.R = 0x18DB0006;        /* Configure for 40MHz OSC, 100KHz bit time */
  for (i=0; i<32; i++) {
    CAN_0.BUF[i].CS.B.CODE = 0;   /* Inactivate all message buffers */
  } 
  CAN_0.BUF[0].CS.B.CODE = 8;     /* Message Buffer 0 set to TX INACTIVE */
  SIU.PCR[16].R = 0x0624;         /* MPC56xxP: Config port B0 as CAN0TX, open drain */
  SIU.PCR[17].R = 0x0500;         /* MPC56xxP: Configure port B1 as CAN0RX */
  CAN_0.MCR.R = 0x0000001F;       /* Negate FlexCAN 0 halt state for 21 MB */
}

void TransmitMsg (void) {
  uint8_t	i;
                                   /* Assumption:  Message buffer CODE is INACTIVE */
  const uint8_t TxData[] = {"Hello"};  /* Transmit string*/
  CAN_0.BUF[0].CS.B.IDE = 0;           /* Use standard ID length */
  CAN_0.BUF[0].ID.B.STD_ID = 555;      /* Transmit ID is 555 */
  CAN_0.BUF[0].CS.B.RTR = 0;           /* Data frame, not remote Tx request frame */
  CAN_0.BUF[0].CS.B.LENGTH = sizeof(TxData) -1 ; /* # bytes to transmit w/o null */
  for (i=0; i<sizeof(TxData); i++) {
    CAN_0.BUF[0].DATA.B[i] = TxData[i]; /* Data to be transmitted */
  }
  CAN_0.BUF[0].CS.B.SRR = 1;           /* Tx frame (not req'd for standard frame)*/
  CAN_0.BUF[0].CS.B.CODE =0xC;         /* Activate msg. buf. to transmit data frame */ 
}

void RecieveMsg (void) {
  uint8_t j;
  uint32_t dummy;

  while (SAFEPORT.IFRL.B.BUF04I == 0) {};  /* Wait for SAFETY PORT MB 4 flag */
  RxCODE   = SAFEPORT.BUF[4].CS.B.CODE;    /* Read CODE, ID, LENGTH, DATA, TIMESTAMP */
  RxID     = SAFEPORT.BUF[4].ID.B.STD_ID;
  RxLENGTH = SAFEPORT.BUF[4].CS.B.LENGTH;
  for (j=0; j<RxLENGTH; j++) { 
    RxDATA[j] = SAFEPORT.BUF[4].DATA.B[j];
  }
  RxTIMESTAMP = SAFEPORT.BUF[4].CS.B.TIMESTAMP; 
  dummy = SAFEPORT.TIMER.R;                /* Read TIMER to unlock message buffers */    
  SAFEPORT.IFRL.R = 0x00000010;            /* Clear SAFETY PORT MB 4 flag */
}

void main(void) {
  volatile uint32_t IdleCtr = 0; 
  
  initModesAndClks();      /* Initialize mode entries */
  initPeriClkGen();        /* Initialize peripheral clock generation for DSPIs */
  disableWatchdog();       /* Disable watchdog */
  initSAFEPORT();          /* Initialize SafetyPort & one of its buffers for receive*/
  initCAN_0();             /* Initialize FlexCAN 0 & one of its buffers for transmit*/
  TransmitMsg();           /* Transmit one message from a FlexCAN 0 buffer */
  RecieveMsg();            /* Wait for the message to be recieved at FlexCAN 1 */
  while (1) {              /* Idle loop: increment counter */
    IdleCtr++;               
  }
}

