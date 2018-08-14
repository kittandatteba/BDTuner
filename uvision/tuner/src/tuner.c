//-----------------------------------------------------------------------------
//   File:      tuner.c
//   Contents:  Hooks required to implement USB peripheral function.
//-----------------------------------------------------------------------------
#pragma NOIV               // Do not generate interrupt vectors

#include <Fx2.h>
#include <fx2regs.h>
#include <fx2sdly.h>            // SYNCDELAY macro

extern BOOL GotSUD;             // Received setup data flag
extern BOOL Sleep;
extern BOOL Rwuen;
extern BOOL Selfpwr;

BYTE Configuration;             // Current configuration
BYTE AlternateSetting;          // Alternate settings

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned long u32;
typedef signed long s32;

#define VR_NAKALL_ON    0xD0
#define VR_NAKALL_OFF   0xD1

//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//   The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

void TD_Init(void)             // Called once at startup
{
	// Initialize EZ-USB I2C controller
	EZUSB_InitI2C();
	I2CTL |= bm400KHZ;

	// set the CPU clock to 48MHz and drive CLKOUT
	CPUCS = ((CPUCS & ~bmCLKSPD) | bmCLKSPD1 | bmCLKOE) ;

	//set FD bus 8bit
	EP2FIFOCFG = 0x04;
	SYNCDELAY;
	EP4FIFOCFG = 0x04;
	SYNCDELAY;
	EP6FIFOCFG = 0x04;
	SYNCDELAY;
	EP8FIFOCFG = 0x04;
	SYNCDELAY;

	EP1OUTCFG = 0xA0;
	EP1INCFG = 0xA0;
	SYNCDELAY;                    // see TRM section 15.14
	EP1OUTBC = 0;

	REVCTL = 0x03; SYNCDELAY;		// see TRM 15.5.9

	EP2CFG = 0xE0;	//(IN, Size = 512, buf = Quad (Buf x4), BULK);
	SYNCDELAY;

	EP4CFG = 0x00;
	SYNCDELAY;

	EP6CFG = 0xE0;	//(IN, Size = 512, buf = Quad (Buf x4), BULK) 
	SYNCDELAY;

	EP8CFG = 0x00;
	SYNCDELAY;

	FIFOPINPOLAR = 0x00; SYNCDELAY;
	EP2AUTOINLENH = 0x02; SYNCDELAY;
	EP2AUTOINLENL = 0x00; SYNCDELAY;
	EP6AUTOINLENH = 0x02; SYNCDELAY;
	EP6AUTOINLENL = 0x00; SYNCDELAY;

	FIFORESET = 0x80; SYNCDELAY;	// activate NAK-ALL to avoid race conditions
	FIFORESET = 0x02; SYNCDELAY;	// reset, FIFO 2
	FIFORESET = 0x04; SYNCDELAY;	// reset, FIFO 4
	FIFORESET = 0x06; SYNCDELAY;	// reset, FIFO 6
	FIFORESET = 0x08; SYNCDELAY;	// reset, FIFO 8
	FIFORESET = 0x00; SYNCDELAY;	// deactivate NAK-ALL

	// enable dual autopointer feature & inc
	AUTOPTRSETUP = 0x07;

	IOD = 0x00;	//MODE_IDLE
	OED = 0xc0;	//MODE is output

	PINFLAGSAB = 0xec;	//FLAGA=EP2FF FLAGB=EP6FF
	SYNCDELAY;                    
}

#define	MODE_IDLE	(0<<6)
#define	MODE_START	(1<<6)

#define CMD_EP6IN_START		0x50	//
#define	CMD_EP6IN_STOP		0x51	//
#define	CMD_EP2IN_START		0x52	//
#define	CMD_EP2IN_STOP		0x53	//
#define	CMD_PORT_CFG		0x54	//addr_mask, out_pins
#define	CMD_PORT_WRITE		0x55	//value
#define	CMD_IFCONFIG		0x56	//value
#define	CMD_MODE_IDLE		0x57
#define	CMD_I2C_READ		0x58	//adrs,len (return length bytes)(max 64bytes)
#define	CMD_I2C_WRITE		0x59	//adrs,len,data... (max 64bytes) 

#define	I2CBF_LEN	64

BYTE xdata i2cbf[I2CBF_LEN];

u8 val_ifconfig;

void TD_Poll(void)              // Called repeatedly while the device is idle
{
	u8 i;
	u8 cmd_cnt,ret_cnt,len;
	u8 addr_mask,adrs;

	if(!(EP1OUTCS & bmEPBUSY))
	{
		cmd_cnt=EP1OUTBC;
		AUTOPTR1H=MSB( &EP1OUTBUF );
        AUTOPTR1L=LSB( &EP1OUTBUF );
		AUTOPTRH2=MSB( &EP1INBUF );
        AUTOPTRL2=LSB( &EP1INBUF );
		ret_cnt=0;
		for(;;){
			if(cmd_cnt-- == 0) break;
			switch(XAUTODAT1){
			case CMD_I2C_READ:
				adrs=XAUTODAT1;
				len=XAUTODAT1;
				cmd_cnt-=2;
				len=(len>I2CBF_LEN)? I2CBF_LEN: len;
				EZUSB_ReadI2C(adrs, len, i2cbf);
				for(i=0; i<len; i++)
				{
					XAUTODAT2=i2cbf[i];
					ret_cnt++;
				}
				break;
			case CMD_I2C_WRITE:
				adrs=XAUTODAT1;
				len=XAUTODAT1;
				cmd_cnt-=2;
				len=(len>I2CBF_LEN)? I2CBF_LEN: len;
				for(i=0; i<len; i++)
				{
					i2cbf[i]=XAUTODAT1;
					cmd_cnt--;
				}
				EZUSB_WriteI2C(adrs, len, i2cbf);
				break;
			case CMD_PORT_WRITE:
				IOD = (IOD & ~addr_mask) | XAUTODAT1;
				cmd_cnt-=1;
				break;
			case CMD_PORT_CFG:
				addr_mask = (XAUTODAT1 | 0xc0);
				OED = XAUTODAT1 | addr_mask;
				addr_mask = ~addr_mask;
 			    cmd_cnt-=2;
				break;
			case CMD_IFCONFIG:
				val_ifconfig = XAUTODAT1;
				cmd_cnt-=1;
				break;
			case CMD_EP6IN_START:
				IFCONFIG = val_ifconfig;

				FIFORESET = 0x06;	// reset, FIFO 6
				SYNCDELAY;

				EP6FIFOCFG = 0x0C;	//8bit, Auto-IN
				SYNCDELAY;

				IOD = MODE_START | (IOD & 0x3f);
				break;
			case CMD_EP6IN_STOP:
				EP6FIFOCFG = 0x04;	//8bit
				break;
			case CMD_EP2IN_START:
				IFCONFIG = val_ifconfig;

				FIFORESET = 0x02;	// reset, FIFO 2
				SYNCDELAY;

				EP2FIFOCFG = 0x0C;	//8bit, Auto-IN
				SYNCDELAY;

				IOD = MODE_START | (IOD & 0x3f);
				break;
			case CMD_EP2IN_STOP:
				EP2FIFOCFG = 0x04;	//8bit
				break;
			case CMD_MODE_IDLE:
				IOD = MODE_IDLE | (IOD & 0x3f);
				break;
			}
		}
		EP1OUTBC=0;	//arm EP1OUT
		if(ret_cnt)
		{
			EP1INBC=ret_cnt;
		}
	}
}

BOOL TD_Suspend(void)          // Called before the device goes into suspend mode
{
   return(TRUE);
}

BOOL TD_Resume(void)          // Called after the device resumes
{
   return(TRUE);
}

//-----------------------------------------------------------------------------
// Device Request hooks
//   The following hooks are called by the end point 0 device request parser.
//-----------------------------------------------------------------------------

BOOL DR_GetDescriptor(void)
{
   return(TRUE);
}

BOOL DR_SetConfiguration(void)   // Called when a Set Configuration command is received
{
   Configuration = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetConfiguration(void)   // Called when a Get Configuration command is received
{
   EP0BUF[0] = Configuration;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_SetInterface(void)       // Called when a Set Interface command is received
{
   AlternateSetting = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetInterface(void)       // Called when a Set Interface command is received
{
   EP0BUF[0] = AlternateSetting;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_GetStatus(void)
{
   return(TRUE);
}

BOOL DR_ClearFeature(void)
{
   return(TRUE);
}

BOOL DR_SetFeature(void)
{
   return(TRUE);
}

BOOL DR_VendorCmnd(void)
{
  BYTE tmp;
  
  switch (SETUPDAT[1])
  {
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
//   The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

// Setup Data Available Interrupt Handler
void ISR_Sudav(void) interrupt 0
{
   GotSUD = TRUE;            // Set flag
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUDAV;         // Clear SUDAV IRQ
}

// Setup Token Interrupt Handler
void ISR_Sutok(void) interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUTOK;         // Clear SUTOK IRQ
}

void ISR_Sof(void) interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSOF;            // Clear SOF IRQ
}

void ISR_Ures(void) interrupt 0
{
   // whenever we get a USB reset, we should revert to full speed mode
   pConfigDscr = pFullSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
   pOtherConfigDscr = pHighSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;

   EZUSB_IRQ_CLEAR();
   USBIRQ = bmURES;         // Clear URES IRQ
}

void ISR_Susp(void) interrupt 0
{
   Sleep = TRUE;
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUSP;
}

void ISR_Highspeed(void) interrupt 0
{
   if (EZUSB_HIGHSPEED())
   {
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
