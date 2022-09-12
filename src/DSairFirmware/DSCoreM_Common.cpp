
#include <arduino.h>
#include "DSCoreM_Type.h"
#include <util/delay.h>
#include <avr/io.h>
#include "DSCoreM_Common.h"

UC gMode = MODE_POWEROFF;
unsigned int gPrevTime = 0;

void DS_GateOff(void)
{
	gMode = MODE_POWEROFF;
	DS_OutputOff();
	
	//Shut off
	//PORTD &= ~_BV(PD7);
	
	/* LED_RUN OFF */
	DS_LEDRUN(0);

}

void DS_OutputOff(void)
{
	
#if defined(__AVR_ATmega4809__)
	VPORTB.OUT &= ~PIN1_bm;//Set low
	VPORTB.OUT &= ~PIN0_bm;
#else
	PORTB &= ~_BV(PB1); 
	PORTB &= ~_BV(PB2); 
#endif
}

void DS_OutputBrake(void)
{
#if defined(__AVR_ATmega4809__)
	VPORTB.OUT |= PIN1_bm;//Set high
	VPORTB.OUT |= PIN0_bm;
#else
	PORTB |= _BV(PB1);
	PORTB |= _BV(PB2);
#endif
}

void DS_LEDRUN(UC inOnOff)
{
	if( inOnOff == 1)
	{
		//PORTC &= ~_BV(PC0); 		
	}
	else
	{
		//PORTC |= _BV(PC0); 		
	}
	
}


void DS_LEDERR(UC inOnOff)
{
	if( inOnOff == 1)
	{
		//PORTC &= ~_BV(PC1);
	}
	else
	{
		//PORTC |= _BV(PC1);
	}
	
}


void DS_Power(UC inMode)
{
	if( inMode == 1)
	{
		/* パワーオン */
		gMode = MODE_POWERON;
		
		//電流を流す
		DS_WriteBit(0);	
		
		/* LED_RUN ON */
		DS_LEDRUN(1);	
		
		/* エラーランプ解除 */
		DS_LEDERR(0);		
	}
	else
	{
		DS_GateOff();	
		/* エラーランプ解除 */
		//DS_LEDERR(0);		
	}

}

UC DS_IsPowerOn(void)
{
	return gMode == MODE_POWERON ? 1 : 0;
}


void DS_WriteBit(UC inBit)
{
	
	if(gMode == MODE_POWEROFF)
	{
		return;
	}

	if( inBit == 0)
	{
		/* PORT B 1,2 */
	#if defined(__AVR_ATmega4809__)
		VPORTB.OUT |= PIN1_bm;//Set high
		VPORTB.OUT &= ~PIN0_bm;//Set low
	#else
		PORTB |= _BV(PB1); // 0 -> 1
		PORTB &= ~_BV(PB2); //1 -> 0
	#endif
	}
	else if (inBit == 1)
	{
		/* PORT B 1,2 */
	#if defined(__AVR_ATmega4809__)
		VPORTB.OUT |= PIN0_bm;//Set high
		VPORTB.OUT &= ~PIN1_bm;//Set low
	#else
		PORTB |= _BV(PB2); // 0 -> 1
		PORTB &= ~_BV(PB1); // 1 -> 0
	#endif
	}
	else
	{
		DS_GateOff();	
	}
}

void DS_WaitSec(UC inSec)
{
	int i;
	
	for( i = 0; i < 5 * inSec; i++)
	{
		_delay_ms(200);
	}
}


UC DS_CalculateCRC(UC *inPackets, UC inLen)
{
	int i = 1;
	UC aCRC = inPackets[0];

	for( i = 1; i < inLen; i++)
	{
		aCRC = aCRC ^ inPackets[i];
	}
	
	return aCRC;

}

UC DS_CalcSpeedStep(short inSpeed, UC inDiv)
{
	
	if( inSpeed == 0)
	{
		return 0;
	}
	else
	{
		
		short aSpeed = ((inSpeed >> 2) * inDiv) >> 8;
		
		return (UC)aSpeed + 1;
	}
	
}


UC DS_GetLocIDProtocol(UC address)
{
	if( address < 0x04)
	{
		return PROTOCOL_MM2;
	}
	else if( (address >= 0x30) && (address <= 0x33))
	{
		return PROTOCOL_MM2_ACC;
	}
	else if( (address >= 0x38) && (address <= 0x3F))
	{
		return PROTOCOL_DCC_ACC;
	}
	else if( (address >= 0x40) && (address <= 0x70))
	{
		return PROTOCOL_MFX;
	}
	else if( (address >= 0xC0) && (address <= 0xFF))
	{
		return PROTOCOL_DCC;
	}
	else
	{
		return PROTCOL_UNKNOWN;
	}
}


UC DS_ABSdiff(UC inUpper, UC inLower)
{
	if( inUpper > inLower)
	{
		return inUpper - inLower;
	}
	else
	{
		return inLower - inUpper;
	}
}



void DS_WaitClk(UC inTime4us)
{
	
	unsigned long aCurrent = micros();
	
	while(1)
	{
		if( micros() >= aCurrent)
		{
			if((micros() - aCurrent) >= inTime4us)
			{
				break;
			}
		}
		else
		{
			if((micros() + (0xFFFFFFFF - aCurrent)) >= inTime4us)
			{
				break;
			}
		}
		
	}
	
	
}


void DS_InitWaitClkX(void)
{
#if defined(__AVR_ATmega4809__)
	TCB2.CNT = 0;
	TCB2.CTRLA = 0;
	TCB2.CTRLB = 0;

#else
	// set up timer with prescaler = 8, 0.5usおき(2MHz), 128usで最大。
	TCCR2A = 0;
	TCCR2B = 0x02; // 8分周 (2MHz)
	
	// initialize counter
	TCNT2 = 0;
#endif
}

void DS_WaitClkX(UC inWaitTime)
{
	
#if defined(__AVR_ATmega4809__)
	
	switch(inWaitTime)
	{
		
	case 110:
		delayMicroseconds(58);
		break;
	case 210:
	default:
		delayMicroseconds(105);
		break;
	}
	
#else
	TCNT2 = 0;
	
	while(TCNT2 < inWaitTime)
	{
		//__asm__("nop\n\t");
	}
#endif
	
}

