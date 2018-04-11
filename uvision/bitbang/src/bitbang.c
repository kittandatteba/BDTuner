//-----------------------------------------------------------------------------
//	 File:		bitbang.c
//	 Contents:	Hooks required to implement USB peripheral function.
//-----------------------------------------------------------------------------
#pragma NOIV              // Do not generate interrupt vectors

#include <Fx2.h>
#include <fx2regs.h>
#include <fx2sdly.h>			// SYNCDELAY macro

extern BOOL GotSUD; 			// Received setup data flag
extern BOOL Sleep;
extern BOOL Rwuen;
extern BOOL Selfpwr;

BYTE Configuration; 			// Current configuration
BYTE AlternateSetting;    // Alternate settings

typedef unsigned  char  u8;
typedef signed    char  s8;
typedef unsigned  short u16;
typedef signed    short s16;
typedef unsigned  long  u32;
typedef signed    long  s32;

#define VR_NAKALL_ON	0xD0
#define VR_NAKALL_OFF	0xD1

//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//	 The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

void TD_Init(void)			// Called once at startup
{
  // set the CPU clock to 12MHz and drive CLKOUT
  CPUCS = ((CPUCS & ~bmCLKSPD) | bmCLKOE);

  EP1OUTCFG = 0xA0;
  EP1INCFG = 0xA0;
  SYNCDELAY;						// see TRM section 15.14
  EP1OUTBC = 0;

  REVCTL = 0x03; SYNCDELAY;		// see TRM 15.5.9

  EP2CFG = 0x00;
  SYNCDELAY;

  EP4CFG = 0x00;
  SYNCDELAY;

  EP6CFG = 0x00;
  SYNCDELAY;

  EP8CFG = 0x00;
  SYNCDELAY;

  // enable dual autopointer feature & inc
  AUTOPTRSETUP = 0x07;

  // Port A configuration
  // (b7,b3,b1,b0) = (TCK,TDO,TMS,TDI)
  IFCONFIG = IFCONFIG & ~bmIFCFG1 & ~bmIFCFG0;        // Ports enable
  PORTACFG = PORTACFG & ~bmFLAGD & ~bmINT1 & ~bmINT0; // PA0,PA1,PA7 enable
  IOA = 0x00;
  OEA = 0x83;
}

void TD_Poll(void)				// Called repeatedly while the device is idle
{
  u8 cmd_cnt, ret_cnt;
  u8 in_buf1, in_buf2;
  u8 out_buf;

  if (!(EP1OUTCS & bmEPBUSY)) {
    cmd_cnt = EP1OUTBC;
    AUTOPTR1H = MSB(&EP1OUTBUF);
    AUTOPTR1L = LSB(&EP1OUTBUF);
    AUTOPTRH2 = MSB(&EP1INBUF);
    AUTOPTRL2 = LSB(&EP1INBUF);
    ret_cnt = 0;

    for (;;) {
      if (cmd_cnt-- <= 0) break;

      // XAUTODAT (b7,b6,b5,b4,b3,b2,b1,b0) = (TCK2,TDO2,TMS2,TDI2,TCK1,TDO1,TMS1,TDI1)

      // 1st in/out
      // IOA (b7,b3,b1,b0) = (TCK1,TDO1,TMS1,TDI1)
      out_buf = XAUTODAT1;
      IOA = ((out_buf & 0x08) << 4) | (out_buf & 0x03);
      out_buf >>= 4;
      in_buf1 = IOA;
      in_buf1 = ((in_buf1 & 0x80) >> 4) | ((in_buf1 & 0x08) >> 1) | (in_buf1 & 0x03);

      // 2nd in/out
      // IOA (b7,b3,b1,b0) = (TCK2,TDO2,TMS2,TDI2)
      IOA = ((out_buf & 0x08) << 4) | (out_buf & 0x03);
      out_buf >>= 4;
      in_buf2 = IOA;
      in_buf2 = ((in_buf2 & 0x80) >> 4) | ((in_buf2 & 0x08) >> 1) | (in_buf2 & 0x03);
      XAUTODAT2 = (in_buf2 << 4) | (in_buf1 & 0x0F);
      ret_cnt++;
    }

    EP1OUTBC = 0;	//arm EP1OUT

    if (ret_cnt) {
      EP1INBC = ret_cnt;
    }
  }
}

BOOL TD_Suspend(void)			// Called before the device goes into suspend mode
{
  return(TRUE);
}

BOOL TD_Resume(void)			// Called after the device resumes
{
  return(TRUE);
}

//-----------------------------------------------------------------------------
// Device Request hooks
//	 The following hooks are called by the end point 0 device request parser.
//-----------------------------------------------------------------------------

BOOL DR_GetDescriptor(void) {
  return(TRUE);
}

BOOL DR_SetConfiguration(void)	// Called when a Set Configuration command is received
{
  Configuration = SETUPDAT[2];
  return(TRUE);         				// Handled by user code
}

BOOL DR_GetConfiguration(void)	// Called when a Get Configuration command is received
{
  EP0BUF[0] = Configuration;
  EP0BCH = 0;
  EP0BCL = 1;
  return(TRUE);         				// Handled by user code
}

BOOL DR_SetInterface(void)      // Called when a Set Interface command is received
{
  AlternateSetting = SETUPDAT[2];
  return(TRUE);                 // Handled by user code
}

BOOL DR_GetInterface(void)      // Called when a Set Interface command is received
{
  EP0BUF[0] = AlternateSetting;
  EP0BCH = 0;
  EP0BCL = 1;
  return(TRUE);                 // Handled by user code
}

BOOL DR_GetStatus(void) {
  return(TRUE);
}

BOOL DR_ClearFeature(void) {
  return(TRUE);
}

BOOL DR_SetFeature(void) {
  return(TRUE);
}

BOOL DR_VendorCmnd(void) {
  BYTE tmp;

  switch (SETUPDAT[1]) {
  case VR_NAKALL_ON:
    tmp = FIFORESET;
    tmp |= bmNAKALL;
    SYNCDELAY;
    FIFORESET = tmp;
    break;
  case VR_NAKALL_OFF:
    tmp = FIFORESET;
    tmp &= ~bmNAKALL;
    SYNCDELAY;
    FIFORESET = tmp;
    break;
  default:
    return(TRUE);
  }

  return(FALSE);
}

//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//	 The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

// Setup Data Available Interrupt Handler
void ISR_Sudav(void) interrupt 0
{
  GotSUD = TRUE;			 // Set flag
  EZUSB_IRQ_CLEAR();
  USBIRQ = bmSUDAV;		 // Clear SUDAV IRQ
}

// Setup Token Interrupt Handler
void ISR_Sutok(void) interrupt 0
{
  EZUSB_IRQ_CLEAR();
  USBIRQ = bmSUTOK;		 // Clear SUTOK IRQ
}

void ISR_Sof(void) interrupt 0
{
  EZUSB_IRQ_CLEAR();
  USBIRQ = bmSOF;			  // Clear SOF IRQ
}

void ISR_Ures(void) interrupt 0
{
  // whenever we get a USB reset, we should revert to full speed mode
  pConfigDscr = pFullSpeedConfigDscr;
  ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
  pOtherConfigDscr = pHighSpeedConfigDscr;
  ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;

  EZUSB_IRQ_CLEAR();
  USBIRQ = bmURES; 		// Clear URES IRQ
}

void ISR_Susp(void) interrupt 0
{
  Sleep = TRUE;
  EZUSB_IRQ_CLEAR();
  USBIRQ = bmSUSP;
}

void ISR_Highspeed(void) interrupt 0
{
  if (EZUSB_HIGHSPEED()) {
    pConfigDscr = pHighSpeedConfigDscr;
    ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
    pOtherConfigDscr = pFullSpeedConfigDscr;
    ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
  }

  EZUSB_IRQ_CLEAR();
  USBIRQ = bmHSGRANT;
}
void ISR_Ep0ack(void) interrupt 0
{
}
void ISR_Stub(void) interrupt 0
{
}
void ISR_Ep0in(void) interrupt 0
{
}
void ISR_Ep0out(void) interrupt 0
{
}
void ISR_Ep1in(void) interrupt 0
{
}
void ISR_Ep1out(void) interrupt 0
{
}
void ISR_Ep2inout(void) interrupt 0
{
}
void ISR_Ep4inout(void) interrupt 0
{
}
void ISR_Ep6inout(void) interrupt 0
{
}
void ISR_Ep8inout(void) interrupt 0
{
}
void ISR_Ibn(void) interrupt 0
{
}
void ISR_Ep0pingnak(void) interrupt 0
{
}
void ISR_Ep1pingnak(void) interrupt 0
{
}
void ISR_Ep2pingnak(void) interrupt 0
{
}
void ISR_Ep4pingnak(void) interrupt 0
{
}
void ISR_Ep6pingnak(void) interrupt 0
{
}
void ISR_Ep8pingnak(void) interrupt 0
{
}
void ISR_Errorlimit(void) interrupt 0
{
}
void ISR_Ep2piderror(void) interrupt 0
{
}
void ISR_Ep4piderror(void) interrupt 0
{
}
void ISR_Ep6piderror(void) interrupt 0
{
}
void ISR_Ep8piderror(void) interrupt 0
{
}
void ISR_Ep2pflag(void) interrupt 0
{
}
void ISR_Ep4pflag(void) interrupt 0
{
}
void ISR_Ep6pflag(void) interrupt 0
{
}
void ISR_Ep8pflag(void) interrupt 0
{
}
void ISR_Ep2eflag(void) interrupt 0
{
}
void ISR_Ep4eflag(void) interrupt 0
{
}
void ISR_Ep6eflag(void) interrupt 0
{
}
void ISR_Ep8eflag(void) interrupt 0
{
}
void ISR_Ep2fflag(void) interrupt 0
{
}
void ISR_Ep4fflag(void) interrupt 0
{
}
void ISR_Ep6fflag(void) interrupt 0
{
}
void ISR_Ep8fflag(void) interrupt 0
{
}
void ISR_GpifComplete(void) interrupt 0
{
}
void ISR_GpifWaveform(void) interrupt 0
{
}
