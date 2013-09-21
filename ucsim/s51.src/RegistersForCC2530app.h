/* Registers used for the CC2530 simulation */
/* Added on 2013-04-25 */


#ifdef CC2530

//#define P0	0X80 // already defined in regs51b.h
//#define SP 	0X81
//#define DPH 	0X82
//#define DPL 	0X83
//#define DPL1	0X84
//#define DPH1	0X85
#define U0CSR	0X86 // redefines DPS
//#define PCON	0X87
//#define TCON	0X88
#define P0IFG	0X89
#define P1IFG	0X8A
#define P2IFG	0X8B
#define PICTL	0X8C
#define P1IEN	0X8D
#define ZERO	0X8E
#define P0INP	0X8F
//#define P1	0X90
#define RFIRQF1	0X91
#define DPS	0X92
#define MPAGE	0X93
#define T2CTRL	0X94
#define ST0	0X95
#define ST1	0X96
#define ST2	0X97
#define SCON	0X98
#define SBUF	0X99
#define IEN2	0X9A
#define S1CON	0X9B
#define T2EVTCFG	0X9C
#define SLEEPSTA	0X9D
#define CLKCONSTA	0X9E
#define FMAP	0X9F
#define P2	0XA0
#define T2IRQF	0XA1
#define T2M0	0XA2
#define T2M1	0XA3
#define T2MOVF0	0XA4
#define T2MOVF1	0XA5
#define T2MOVF2	0XA6
#define T2IRQM	0XA7
#define IEN0	0XA8
#define EXIF	0XA9
#define IEN1	0XAA
#define P0IEN	0XAB
#define P2IEN	0XAC
#define STLOAD	0XAD
#define PMUX	0XAE
#define T1STAT	0XAF
#define P3	0XB0
#define ENCDI	0XB1
#define ENCDO	0XB2
#define ENCCS	0XB3
#define ADCCON1	0XB4
#define ADCCON2	0XB5
#define ADCCON3	0XB6
#define IPH0	0XB7
#define IPL0	0XB8
#define IP1	0XB9
#define ADCL	0XBA
#define ADCH	0XBB
#define RNDL	0XBC
#define RNDH	0XBD
#define SLEEPCMD	0XBE
#define RFERRF	0XBF
#define IRCON	0XC0
#define U0DBUF	0XC1
#define U0BAUD	0XC2
#define T2MSEL	0XC3
#define U0UCR	0XC4
#define U0GCR	0XC5
#define CLKCONCMD	0XC6
#define MCON	0XC7
#define T2CON	0XC8
#define WDCTL	0XC9
#define T3CNT	0XCA
#define T3CTL	0XCB
#define T3CCTL0	0XCC
#define T3CC0	0XCD
#define T3CCTL1	0XCE
#define T3CC1	0XCF
#define PSW	0XD0
#define DMAIRQ	0XD1
#define DMA1CFGL	0XD2
#define DMA1CFGH	0XD3
#define DMA0CFGL	0XD4
#define DMA0CFGH	0XD5
#define DMAARM	0XD6
#define DMAREQ	0XD7
#define TIMIF	0XD8
#define RFD	0XD9
#define T1CC0L	0XDA
#define T1CC0H	0XDB
#define T1CC1L	0XDC
#define T1CC1H	0XDD
#define T1CC2L	0XDE
#define T1CC2H	0XDF
#define ACC	0XE0
#define RFST	0XE1
#define T1CNTL	0XE2
#define T1CNTH	0XE3

#define T1CTL	0XE4
//#define T1CTL.MODE 	0XE4

#define T1CCTL0	0XE5
#define T1CCTL1	0XE6
#define T1CCTL2	0XE7
#define IRCON2	0XE8
#define RFIRQF0	0XE9
#define T4CNT	0XEA
#define T4CTL	0XEB
#define T4CCTL0	0XEC
#define T4CC0	0XED
#define T4CCTL1	0XEE
#define T4CC1	0XEF
#define B	0XF0
#define PERCFG	0XF1
#define APCFG	0XF2
#define P0SEL	0XF3
#define P1SEL	0XF4
#define P2SEL	0XF5
#define P1INP	0XF6
#define P2INP	0XF7
#define U1CSR	0XF8
#define U1DBUF	0XF9
#define U1BAUD	0XFA
#define U1UCR	0XFB
#define U1GCR	0XFC
#define P0DIR	0XFD
#define P1DIR	0XFE
#define P2DIR	0XFF

/* Bit masks in T1CCTL0 */
/*
#define bmECOM0	0x40
#define bmCAPP0	0x20
#define bmCAPN0	0x10
#define bmMAT0	0x08
#define bmTOG0	0x04
#define bmPWM0	0x02
#define bmECCF0	0x01*/

/* Bit masks in T1CTL */
#define bmM0	0x01
#define bmM1	0x02
#define bmDIV	0x0C

#define bmTimerMode 0x04
#define bmCaptMode 0x03
#define bmCmpMode 0x04
#define bmWTDonCMP 0x38
#define bmTickSpd 0x38

/* Bit masks in IEN1 */

#define bmP0IE	0x20
#define bmT4IE	0x10
#define bmT3IE	0x08
#define bmT2IE	0x04
#define bmT1IE	0x02
#define bmDMAIE	0x01

/* Bit masks in T1STAT */

#define bmOVFIF	0x20
#define bmCH4IF	0x10
#define bmCH3IF	0x08
#define bmCH2IF	0x04
#define bmCH1IF	0x02
#define bmCH0IF	0x01

/* Bit masks in TIMIF */

#define bmT1OVFIM	0x40
#define bmT4CH1IF	0x20
#define bmT4CH0IF	0x10
#define bmT4OVFIF	0x08
#define bmT3CH1IF	0x04
#define bmT3CH0IF	0x02
#define bmT3OVFIF	0x01

/* Bit masks in IRCON */

#define bmT1IF  0x02
#define bmT2IF  0x04
#define bmT3IF  0x08
#define bmT4IF  0x10

/*USART*/

#define bmURX0IF 0x08
#define bmUTX0IF 0x02
#define bmURX0IE 0x04
#define bmUTX0IE 0x04
#define bmURX1IF 0x80
#define bmUTX1IF 0x04
#define bmURX1IE 0x08
#define bmUTX1IE 0x08

/*UxGCR*/
#define bmCpol    0x80
#define bmCpha    0x40
#define bmOrder   0x20
#define bmBaud_e  0x1F

/*UxCSR*/
#define bmMode    0x80
#define bmRe      0x40
#define bmSlave   0x20
#define bmFe      0x10
#define bmErr     0x08
#define bmRx_byte 0x04
#define bmTx_byte 0x02
#define bmActive  0x01

/*UxUCR*/
#define bmFlush    0x80
#define bmFlow     0x40
#define bmD9       0x20
#define bmBit9     0x10
#define bmParity   0x08
#define bmSPB      0x04
#define bmStop     0x02
#define bmStart    0x01

/* Bit masks of flag bits in PSW (0xd0)*/

#define bmCY	0x80 /* carry */
#define bmAC	0x40 /* acarry */
#define bmF0	0x20 /* flag 0 */
#define bmRS1	0x10 /* register select 1 */
#define bmRS0	0x08 /* register select 0 */
#define bmOV	0x04 /* arithmetic overflow */
#define bmP	0x01 /* parity, set by hardware */

#endif
