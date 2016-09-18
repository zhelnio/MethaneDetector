/*
 * MethaneDetector.c
 *
 * Created: 17.09.2016 11:12:02
 * Author : stas
 */ 

#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

/*
	PB1 (AIN1)	- detector signal
	PB0 (AIN0)	- reference
	PD6			- detector on/off
	PD5			- alarm on/off
*/

inline void init(void)
{
	PORTD = 0;
	DDRD = (1 << DDD5) | (1 << DDD6);

	DIDR = (1 << AIN1D) | (1 << AIN0D);
}

inline void turnAlarmOn(void)
{
	PORTD |= (1 << PD5);
}

inline void turnAlarmOff(void)
{
	PORTD &= ~(1 << PD5);
}

inline void turnDetectorOn(void)
{
	PORTD |= (1 << PD6);
}

inline void turnDetectorOff(void)
{
	PORTD &= ~(1 << PD6);
}

inline bool gasDetected(void)
{
	return !(ACSR & (1 << ACO));
}

inline static void initStartSignal(void)
{
	turnAlarmOn();
	_delay_ms(1000);
	turnAlarmOff();
}

inline static void initEndSignal(void)
{
	turnAlarmOn();
	_delay_ms(500);
	turnAlarmOff();
	_delay_ms(500);
	turnAlarmOn();
	_delay_ms(1000);
	turnAlarmOff();
}

inline static void alarmSignal(void)
{
	turnAlarmOn();
	_delay_ms(500);
	turnAlarmOff();
	_delay_ms(500);

	turnAlarmOn();
	_delay_ms(1000);
	turnAlarmOff();
	_delay_ms(1000);
}

int main(void)
{
	init();

	initStartSignal();

	turnDetectorOn();
	_delay_ms(1000*30);

	initEndSignal();

    while (1) 
    {
		if(gasDetected())
			alarmSignal();
		else
			_delay_ms(1000);
    }
}

