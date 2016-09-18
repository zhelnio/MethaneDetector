/*
MIT License

Copyright (c) 2016 Stanislav Zhelnio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */ 

#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/sleep.h>

/*
	PB1 (AIN1)	- detector signal
	PB0 (AIN0)	- reference
	PD6			- detector on/off
	PD5			- alarm on/off
*/

#pragma region HAL

inline void init(void)
{
	//set ports to out
	PORTD = 0;
	DDRD = (1 << DDD5) | (1 << DDD6);

	//disable digital input on comparator pins
	DIDR = (1 << AIN1D) | (1 << AIN0D);

	//timer init
	TCCR1A = 0;
	TCCR1B = (1 << WGM12)| (1 << CS12) | (1 << CS10); // clk/1024, CTC mode
	TCCR1C = 0;
	OCR1A  = 488;			// T = 0.5 sec
	OCR1B  = 0;
	TIMSK  = (1 << OCIE1A);	// interrupt on overflow

	set_sleep_mode(SLEEP_MODE_IDLE);

	//enable interrupts
	sei();
}

// I'm using active buzzer so there is no PWM or something else for Alarm signal
inline void turnAlarmOn(void)		{ PORTD |= (1 << PD5);  } 
inline void turnAlarmOff(void)		{ PORTD &= ~(1 << PD5); }
inline void turnDetectorOn(void)	{ PORTD |= (1 << PD6);	}
inline void turnDetectorOff(void)	{ PORTD &= ~(1 << PD6);	}
inline bool getSensorValue(void)	{ return !(ACSR & (1 << ACO));	}


#pragma endregion HAL

#pragma region input filter

// if we have some noise, then gas detected
#define FILTER_WINDOW_SIZE 16

bool gasDetected(void)
{
	for(uint8_t i = 0; i < FILTER_WINDOW_SIZE; i++)
		if(getSensorValue())
			return true;
	return false;
}

#pragma endregion input filter

#pragma region Sound

// 8 notes are playing for each sound: one per bit
typedef enum {SoundInitDone = 0b00000101, SoundAlarm = 0b00111001, SoundNone = 0 } Sound;
Sound	nextSound = 0;

inline void setDiscreteSound(Sound sound)
{
	nextSound = sound;
}

inline Sound getDiscreteSound(void)
{
	Sound s = nextSound;
	nextSound = SoundNone;	
	return s;
}

void playDiscreteSound()
{
	static uint8_t currentSound = 0;
	static uint8_t soundNoteNum = 8;

	if(soundNoteNum == 8)
	{
		currentSound = getDiscreteSound();
		soundNoteNum = 0;
	}
	else
	{
		if(currentSound & (1 << soundNoteNum++))
			turnAlarmOn();
		else
			turnAlarmOff();
	}
}
#pragma endregion Sound

#pragma region FSM

//periods of sensor preheat before detection and waiting after
#define HEAT_PERIOD 30
#define WAIT_PERIOD 90

#define TIMER_TICKS_IN_SECOND 2

/*
FSM states:
	INIT	- first step
	HEAT	- enable heater in sensor and wait
	DETECT	- check sensor value
	ALARM	- gas detected, enable alarm sound
	WAIT	- sensor heater disabled

    INIT -> HEAT -> DETECT -.
             ^        |     |
             |        |     |
            WAIT <----'     |
             ^              |
             |              |
            ALARM <---------'
             ^  |
             ''''
*/

typedef enum {Init, Heat, Detect, Alarm, Wait } FsmState;

inline FsmState getNextState(FsmState state, uint16_t currentStateDelay)
{	
	switch (state)
	{
		default:
		case Init:		return Heat;
		case Heat:		return (currentStateDelay) ? Heat : Detect;
		case Detect:	return gasDetected() ? Alarm : Wait;
		case Wait:		return (currentStateDelay) ? Wait : Heat;
		case Alarm:		return gasDetected() ? Alarm : Wait;
	}
}

uint16_t getStateDelay(FsmState state)
{
	switch (state)
	{
		default:
		case Init:		return 0;
		case Heat:		return HEAT_PERIOD * TIMER_TICKS_IN_SECOND;
		case Detect:	return 0;
		case Wait:		return WAIT_PERIOD * TIMER_TICKS_IN_SECOND;
		case Alarm:		return 0;
	}
}

inline void setStateOutput(FsmState state)
{
	switch (state)
	{
		case Init:		break;
		case Heat:		turnDetectorOn(); break;
		case Detect:	break;
		case Wait:		turnDetectorOff(); break;
		case Alarm:		setDiscreteSound(SoundAlarm); break;
	}
}

void fsm(void)
{
	static FsmState State = Init;
	static uint16_t currentStateDelay = 0;

	FsmState Next = getNextState(State, currentStateDelay);
	currentStateDelay = (currentStateDelay > 0) ? (currentStateDelay - 1) : getStateDelay(Next);

	State = Next;
	setStateOutput(State);
}
#pragma endregion FSM

int main(void)
{
	init();

	turnDetectorOn();

	setDiscreteSound(SoundInitDone);

	while (1)
		sleep_mode();
}

ISR(TIMER1_COMPA_vect)
{
	playDiscreteSound();

	fsm();
}
