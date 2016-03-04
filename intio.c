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

// Filter Coefficients stored in arrays 'double a[] = {......}; double b[] = {......};'
#include "coef.txt"

// Get buffer length based on coefficient array
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

// Delay buffer (Tustin)
short xt[2] = {0, 0};

// Output of filter (Tustin)
double yt[2] = {0, 0}; 

// Buffer to hold inputs 
double x[BUFSIZE];

// Input read from signal generator
double sample = 0; 

// Value stored for output 
double output = 0;

/******************************* Function prototypes ********************************/
void init_hardware(void);      
void init_HWI(void);          
void ISR_AIC(void);        
void tustin(void);
void direct_form_2(void);
void direct_form_2_transpose(void);

/********************************** Main routine ************************************/
void main()
{    
	// Zeroise Buffer
	int i = 0;
	for (; i < BUFSIZE; i++)
		x[i] = 0;
	
	// initialize board and the audio port
 	init_hardware();
	
	 /* initialize hardware interrupts */
	init_HWI();  
	
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

// Interrupt Service routine applies IIR filter to input
void ISR_AIC(void)
{
	// Read in sample 
	sample = mono_read_16Bit(); 
		
	// Perform IIR
	direct_form_2_transpose(); 
	
	// Write out sample
	mono_write_16Bit(output); 
}

//Implementation of the first order IIR filter (RC Filter)
void tustin(void)
{
	// Shift samples along buffer
	xt[1] = xt[0];
	
	// Read in new sample
	xt[0] = sample;
	
	// Apply filter
	yt[0] = (xt[0])*(1/17) + xt[1] * 1/17 + yt[1] * 15/17; 
	
	// Shift outputs along buffer
	yt[1] = yt[0];
	
	// Set output value
	output = yt[0];
}

// Direct form 2 Filter
void direct_form_2(void)
{
	// Initialise iterator to the end of buffer 
	int i = BUFSIZE - 1;
	
	// Variables to store values of left side adder (x[i]*-a[i]) 
	// and right side adder (x[i]*b[i]) 
	double left = 0;
	double right = 0;
	
	
	for (; i > 0; i--)
	{
		// Shift values back in buffer
		x[i] = x[i-1];
		
		// Apply filter coefficients 
		left -= a[i]*x[i];
		right += b[i]*x[i];
	}
	
	// Adding new sample to left adder
	left += sample;
	
	// Applying the final b[0] coefficient to the sum
	output = right + b[0]*left;
	
	// Start of array set to output of left adder
	x[0] = left;
}

// Direct Form 2 Transpose filter
void direct_form_2_transpose(void)
{
	int i = 0;
	
	// x[0] is the sum of the previous inputs that have had the filter coefficents 1-n applied to them 
	output = x[0] + b[0]*sample;
	
	// As seen in the diagram, each x[i] is the sum of the ouput of the i+1th adder, and the the i+1th a and b 
	//coeffients applied to the new sample
	
	// Break at i < BUFSIZE-2 because last element (index BUFSIZE-1) is calclated slightly differently below 
	for(; i < BUFSIZE - 2; i++)
		x[i] = x[i+1] + b[i+1]*sample - a[i+1]*output;
	
	// Sligtly different calculation as there is no adder behind the last element, so it is just the sum of
	// the (BUFSIZE-1)th coefficients applied to the new sample
	x[BUFSIZE-2] = b[BUFSIZE-1]*sample - a[BUFSIZE-1]*output;
}
