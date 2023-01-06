/* main.c - DMA Block Move  */
/* OCt 23 2008 S.Mihalik -inital version */             
/* Copyright Freescale Semiconductor, Inc 2008 All rights reserved. */

#include "Pictus_Header_v1_09.h" /* Use proper include file such as mpc5510.h or mpc5554.h */

const  uint8_t  SourceData[] = {"Hello World\r"};	/* Source data string */
	   uint8_t  Destination = 0;	                /* Destination byte */

void initTCD0(void) {

  EDMA.TCD[0].SADDR = (vuint32_t) &SourceData;  /* Load address of source data */
  EDMA.TCD[0].SSIZE = 0;                        /* Read 2**0 = 1 byte per transfer */
  EDMA.TCD[0].SOFF = 1;                         /* After transfer, add 1 to src addr*/
  EDMA.TCD[0].SLAST = -12;                      /* After major loop, reset src addr*/ 
  EDMA.TCD[0].SMOD = 0;                         /* Source modulo feature not used */

  EDMA.TCD[0].DADDR = (vuint32_t) &Destination; /* Load address of destination */
  EDMA.TCD[0].DSIZE = 0;                        /* Write 2**0 = 1 byte per transfer */
  EDMA.TCD[0].DOFF = 0;                         /* Do not increment destination addr */
  EDMA.TCD[0].DLAST_SGA = 0;                    /* After major loop, no dest addr change*/ 
  EDMA.TCD[0].DMOD = 0;                         /* Destination modulo feature not used */
  
  EDMA.TCD[0].NBYTES = 1;                       /* Transfer 1 byte per minor loop */
  EDMA.TCD[0].BITER = 12;                       /* 12 minor loop iterations */
  EDMA.TCD[0].CITER = 12;                       /* Initialize current iteraction count */
  EDMA.TCD[0].D_REQ = 1;                        /* Disable channel when major loop is done*/
  EDMA.TCD[0].INT_HALF = 0;                     /* Interrupts are not used */
  EDMA.TCD[0].INT_MAJ = 0;
  EDMA.TCD[0].CITERE_LINK = 0;                  /* Linking is not used */									  
  EDMA.TCD[0].BITERE_LINK = 0;
  EDMA.TCD[0].MAJORE_LINK = 0;                  /* Dynamic program is not used */
  EDMA.TCD[0].E_SG = 0; 
  EDMA.TCD[0].BWC = 0;                          /* Default bandwidth control- no stalls */
  EDMA.TCD[0].START = 0;                        /* Initialize status flags */
  EDMA.TCD[0].DONE = 0;
  EDMA.TCD[0].ACTIVE = 0;
}

void main (void) {	
  volatile uint32_t i = 0;              /* Dummy idle counter */
  
  initTCD0();            /* Initialize DMA Transfer Control Descriptor 0 */

  EDMA.CR.R = 0x0000E400; /* Use fixed priority arbitration for DMA groups and channels */ 
  EDMA.CPR[0].R = 0x0;  /* Channel 0 priorites: group priority = 0, channel priority = 0 */
  
  EDMA.SERQR.R = 0;      /* Enable EDMA channel 0 */
  
  EDMA.SSBR.R = 0;       /* Set channel 0 START bit to initiate first minor loop transfer */
 
                          /* Initate DMA service using software */   
  while (EDMA.TCD[0].CITER != 1) { /* while not on last minor loop */
                                    /* wait for START=0 and ACTIVE=0 */
    while ((EDMA.TCD[0].START == 1) | (EDMA.TCD[0].ACTIVE == 1)) {}
    EDMA.SSBR.R = 0;     /* Set channel 0 START bit again for next minor loop transfer */  
  }
  while (1) { 
    LoopForever:
    i++; 
    }      /* Loop forever */
}

