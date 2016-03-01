/*************************************************************************************
			       DEPARTMENT OF ELECTRICAL AND ELECTRONIC ENGINEERING
					   		     IMPERIAL COLLEGE LONDON 

 				      EE 3.19: Real Time Digital Signal Processing
					       Dr Paul Mitcheson and Daniel Harvey

				        		  LAB 3: Interrupt I/O

 				            ********* I N T I O. C **********

  Demonstrates inputing and outputing data from the DSK's audio port using interrupts. 

 *************************************************************************************
 				Updated for use on 6713 DSK by Danny Harvey: May-Aug 2006
				Updated for CCS V4 Sept 10
 ************************************************************************************/
/*
 *	You should modify the code so that interrupts are used to service the 
 *  audio port.
 */
/**************************** Pre-processor statements ******************************/

#include <stdlib.h> 
//  Included so program can make use of DSP/BIOS configuration tool.  
#include "dsp_bios_cfg.h"

/* The file dsk6713.h must be included in every program that uses the BSL.  This 
   example also includes dsk6713_aic23.h because it uses the 
   AIC23 codec module (audio interface). */
#include "dsk6713.h"
#include "dsk6713_aic23.h"

// Some functions to help with writing/reading the audio ports when using interrupts.
#include <helper_functions_ISR.h>

// Filter Coefficients stored in array 'double b[] = {......}'
#include "coef.txt"


#define ORDER sizeof(a)/sizeof(a[0]) - 1

#define BUFSIZE ORDER + 1

/******************************* Global declarations ********************************/

/* Audio port configuration settings: these values set registers in the AIC23 audio 
   interface to configure it. See TI doc SLWS106D 3-3 to 3-10 for more info. */
DSK6713_AIC23_Config Config = { \
			 /**********************************************************************/
			 /*   REGISTER	            FUNCTION			      SETTINGS         */ 
			 /**********************************************************************/\
    0x0017,  /* 0 LEFTINVOL  Left line input channel volume  0dB                   */\
    0x0017,  /* 1 RIGHTINVOL Right line input channel volume 0dB                   */\
    0x01f9,  /* 2 LEFTHPVOL  Left channel headphone volume   0dB                   */\
    0x01f9,  /* 3 RIGHTHPVOL Right channel headphone volume  0dB                   */\
    0x0011,  /* 4 ANAPATH    Analog audio path control       DAC on, Mic boost 20dB*/\
    0x0000,  /* 5 DIGPATH    Digital audio path control      All Filters off       */\
    0x0000,  /* 6 DPOWERDOWN Power down control              All Hardware on       */\
    0x0043,  /* 7 DIGIF      Digital audio interface format  16 bit                */\
    0x008d,  /* 8 SAMPLERATE Sample rate control             8 KHZ                 */\
    0x0001   /* 9 DIGACT     Digital interface activation    On                    */\
			 /**********************************************************************/
};


// Codec handle:- a variable used to identify audio interface  
DSK6713_AIC23_CodecHandle H_Codec;


// Delay buffer
short xt[2] = {0, 0};

// Output of filter
double yt[2] = {0, 0}; 

double x[BUFSIZE];


// Input read from signal generator
short sample = 0; 

short output = 0;

double C = 0.000001;
double R = 1000;
double T = 1.0/8000;


/******************************* Function prototypes ********************************/
void init_hardware(void);     
void init_HWI(void);          
void ISR_AIC(void);        
void tustin(void);
void direct_form_2(void);
void direct_form_2_transpose(void);
void direct_form_2_transposeshitone(void);
/********************************** Main routine ************************************/
void main(){    
int i=0;
// initialize board and the audio port
 	init_hardware();
	
  /* initialize hardware interrupts */
	init_HWI();  
	
	// Zeroise Buffer	 		
	
	for (; i < BUFSIZE; i++)
		x[i] = 0;
		
		
	
  /* loop indefinitely, waiting for interrupts */  					
	while(1){}
  
}  
        
/********************************** init_hardware() **********************************/  
void init_hardware()
{
    // Initialize the board support library, must be called first 
    DSK6713_init();
    
    // Start the AIC23 codec using the settings defined above in config 
    H_Codec = DSK6713_AIC23_openCodec(0, &Config);

	/* Function below sets the number of bits in word used by MSBSP (serial port) for 
	receives from AIC23 (audio port). We are using a 32 bit packet containing two 
	16 bit numbers hence 32BIT is set for  receive */
	MCBSP_FSETS(RCR1, RWDLEN1, 32BIT);	

	/* Configures interrupt to activate on each consecutive available 32 bits 
	from Audio port hence an interrupt is generated for each L & R sample pair */	
	MCBSP_FSETS(SPCR1, RINTM, FRM);

	/* These commands do the same thing as above but applied to data transfers to  
	the audio port */
	MCBSP_FSETS(XCR1, XWDLEN1, 32BIT);	
	MCBSP_FSETS(SPCR1, XINTM, FRM);	
}

/********************************** init_HWI() **************************************/  
void init_HWI(void)
{
	IRQ_globalDisable();			// Globally disables interrupts
	IRQ_nmiEnable();				// Enables the NMI interrupt (used by the debugger)
	IRQ_map(IRQ_EVT_RINT1,4);		// Maps an event to a physical interrupt
	IRQ_enable(IRQ_EVT_RINT1);		// Enables the event
	IRQ_globalEnable();				// Globally enables interrupts
} 

/******************** WRITE YOUR INTERRUPT SERVICE ROUTINE HERE***********************/  

/*
 * Convolves input with filter and outputs
 */
 
void ISR_AIC(void)
{
	sample = mono_read_16Bit(); 
		
	direct_form_2_transpose();
	
	mono_write_16Bit(output); 
}

void tustin(void)
{
	xt[1] = xt[0];
	xt[0] = sample;
	
	yt[0] = (xt[0])*(1/17) + xt[1] * 1/17 + yt[1] * 15/17;
	yt[1] = yt[0];
	output = (short)yt[0];
	
}

void direct_form_2(void)
{
	int i = BUFSIZE - 1;
	double left = 0;
	double right = 0;
	for (; i > 1; i--)
	{
		x[i] = x[i-1];
		left -= a[i]*x[i];
		right += b[i]*x[i];
	}
	
	left += sample;
	output = right + b[0]*left;
	x[0] = left;
}

void direct_form_2_transpose(void)
{
	int i = BUFSIZE - 1;
	double y = output;
	
	for (; i > 1; i--)
		x[i] = x[i-1];
	
	x[0] = sample;
	
	output = x[0]*b[0];
	for(; i < BUFSIZE; i++)
		output += x[i]*b[i] + y*a[i];
		
	
}
/*
void direct_form_2_transposeshitone(void)
{
	int i = 0;

	output = b[0]*sample;
	
	for(; <BUFSIZE; i++)
	{
		output += b[i]*x[i] - 

	}
}*/
