#include "p33FJ256GP710.h"
#include "circBuffer.h"
//#include "apDefinitions.h"
//#include "MultiCircBuffer.h"
#include "uart2.h"
#include "stdio.h"
#include "loggerDefinitions.h"
//#include "overflowBuffer.h"

struct CircBuffer outgoingUart;
CBRef UART_transmitBuffer;
#define BUFFERSIZE 512
struct CircBuffer incomingUart;
CBRef UART_receiveBuffer;

//struct MultiCircBuffer SPIerrorBuffer;
//MCBRef SPI_errorBuffer;

//static DWORD CurrentSector;

//#ifdef DMAON
	
	// struct containing all parts a packet to allow easy writing
	
	

	

		
	unsigned char BufferA[DMABUFFERSIZE] __attribute__((space(dma))) = {0};
	unsigned char BufferB[DMABUFFERSIZE] __attribute__((space(dma))) = {0};
	static char bufferflag;
	static char need_buffer;
	//FSFILE * pointer;
	

//#endif

void UART2Init(void)
{
		// Configure and open the port;
	// U2MODE Register
	// ==============
	U2MODEbits.UARTEN = 0;		// Disable the port		
	U2MODEbits.USIDL = 0;		// Stop on idle
	U2MODEbits.IREN	= 0;		// No IR decoder
	U2MODEbits.RTSMD = 0;		// Ready to send mode (irrelevant)
	U2MODEbits.UEN = 0;		// Only RX and TX
	U2MODEbits.WAKE	= 1;		// Enable at startup
	U2MODEbits.LPBACK = 0;		// Disable loopback
	U2MODEbits.ABAUD = 0;		// Disable autobaud
	U2MODEbits.URXINV = 0;		// Normal operation (high is idle)
	U2MODEbits.PDSEL = 0;		// No parity 8 bit
	U2MODEbits.STSEL = 0;		// 1 stop bit
	U2MODEbits.BRGH  = 1;		
	
	IPC7 = 0x4400;
	// U2STA Register
	// ==============
	U2STAbits.URXISEL	= 0;		// RX interrupt on every character
	U2STAbits.OERR		= 0;		// clear overun error
	
	
	// U2BRG Register
	// ==============
	U2BRG = 86;			// Set the baud rate to 115200 by 86 576000 by 173
	
	// Enable the port;
	U2MODEbits.UARTEN	= 1;		// Enable the port	
	U2STAbits.UTXEN		= 1;		// Enable TX
	UART_transmitBuffer=(struct CircBuffer*)&outgoingUart;  //set up buffer for receive
	newCircBuffer(UART_transmitBuffer);
	//printf("%p\r\n",BufferB);
	#ifdef DMAON
	need_buffer=0;
	DMA0CON=0x0002; 
	DMA0CONbits.DIR=0; //peripheral to ram
	DMA0CONbits.MODE=2; // ping pong
	DMA0CONbits.SIZE=1;  //char supposedly in size, seems to be true
	DMA0CNT=DMABUFFERSIZE-1; 
	DMA0REQ=0b0011110;
	DMA0PAD=(volatile unsigned int) &U2RXREG;
	DMA0STA=__builtin_dmaoffset(BufferA);
	DMA0STB=__builtin_dmaoffset(BufferB);
	IFS0bits.DMA0IF=0;
	IEC0bits.DMA0IE=1;
	DMA0CONbits.CHEN = 1;
	#else
	IEC1bits.U2RXIE		= 1;		// Enable RX interrupts
	UART_receiveBuffer=(struct CircBuffer*)&incomingUart; //set up buffer for transmit
	newCircBuffer(UART_receiveBuffer);

	#endif
}

//puts one char onto circular buffer for writing out to uart
//while enabling transmit interupt if needed

void UART2PutChar( char ch )
{
	if(getLength(UART_transmitBuffer)!=BSIZE)
	{
	writeBack(UART_transmitBuffer,ch);
	IEC1bits.U2TXIE=1;
	}
	if(U2STAbits.TRMT)
	{
		IFS1bits.U2TXIF=1;
	}	
}


//interupt for handling transmitting of bytes
void __attribute__((__interrupt__, no_auto_psv)) _U2TXInterrupt(void)
{
	IFS1bits.U2TXIF=0;
	if(getLength(UART_transmitBuffer)==0)
	{
		IEC1bits.U2TXIE=0;
	}
	else
	{
		U2TXREG=readFront(UART_transmitBuffer);
		//IEC1bits.U2TXIE=1;	
	}
}		

// function to read bytes from uart
//mostly defunct as now bytes are read by DMA
char UART2GetChar()
{
	//return getLength(UART_receiveBuffer);
	if(getLength(UART_receiveBuffer)==0)
	{
		return 0;
	}	
	return readFront(UART_receiveBuffer);
}
	

#ifdef DMAON

//flips between buffers and sets flag to tell which to read
void __attribute__((__interrupt__)) _DMA0Interrupt(void)
{
	//if(need_buffer!=0)
		//printf("have surpassed buffer\r\n");
	static unsigned int BufferCount=0;	
	if(BufferCount==0)
	{
		OFB_push(BufferA);
	}
	else
	{
		OFB_push(BufferB);
	}
	BufferCount ^=1;
	IFS0bits.DMA0IF=0;
}

#else
void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt(void)
{
	while(U2STAbits.URXDA == 1){
		writeBack(UART_receiveBuffer, (unsigned char)U2RXREG);
	}
	
	// If there was an overun error clear it and continue
	if (U2STAbits.OERR == 1){
		U2STAbits.OERR = 0;
	}
	
	// clear the interrupt
	IFS1bits.U2RXIF = 0;
}
#endif

	
char UART2IsEmpty()
{
	if(getLength(UART_receiveBuffer)==0)
		return 1;
	return 0;
}	



//funcionality has been moved to the overflow buffer file
unsigned char oldcalc_checksum(unsigned char inBuffer[], unsigned char outBuffer[])
{
		// Loop through all chars to get a checksum
		unsigned char checkSum; //= inBuffer[0];
		checkSum=inBuffer[0];
		outBuffer[0]=inBuffer[0];
		int i;
		for (i=1;i<DMABUFFERSIZE; i++)
		{
			checkSum^=inBuffer[i];
			outBuffer[i]=inBuffer[i];
		}
		/*for (i = 0; i< DMABUFFERSIZE; i++)
	    {
	   		   // Is this the first value for the checksum?
	        if (i == 0)
	        {
	          // Yes. Set the checksum to the value
	          checkSum = inBuffer[i];
	        }
	        else
	        {
	          // No. XOR the checksum with this character's value
		       checkSum ^= inBuffer[i];
	        }
	        outBuffer[i]=inBuffer[i];
	    }*/
	    // Return the checksum 
	    return checkSum;
}