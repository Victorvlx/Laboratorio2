// LABORATORY 2 - Traffic Light Controller
// Description: Traffic light control using ATtiny85 microcontroller.
// Loux Roland Victor
// References for this lab:
// 1: https://www.gadgetronicx.com/attiny85-timer-tutorial-generating-time-delay-interrupts/
// 2: https://www.gadgetronicx.com/attiny85-external-pin-change-interrupt/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Function Declarations
void DelayTimes();

// Traffic Light States
#define VEHICLE_GREEN 0
#define VEHICLE_RED 2
#define PEDESTRIAN_GREEN 1
#define PEDESTRIAN_RED 3
#define FLASHING_LIGHT 4

// Time States
#define MIN_VEHICLE_PASS_TIME 5
#define FLASHING_TIME 6

// Global Variables
int ButtonPress = 0;
int CycleCount = 0;
int CurrentState = 0;
int TimeState = 5;
int Seconds = 0;

// BUTTON INTERRUPT
ISR(INT0_vect)
{
    ButtonPress = 1;
}

// TIME INTERRUPT FOR DELAY
ISR(TIMER0_OVF_vect)
{
    DelayTimes();
}

// Function to Count Time in Seconds
void DelayTimes()
{
    if (CycleCount == 63) 
    {
        CycleCount = 0;
        ++Seconds;
    }
    else
        CycleCount++;
}

// Configure Timers and Interrupts
void TimerConfiguration()
{
    TCCR0A = 0x00; // Normal mode
    TCCR0B = 0x00;
    TCCR0B |= (1 << CS00) | (1 << CS02); // Prescaling at 1024
    sei(); // Enable global interrupt
    TCNT0 = 0;
    TIMSK |= (1 << TOIE0); // Enable timer0 interrupt
}

// External Switch (Button) Configuration
void ExternalSwitchConfiguration()
{
    DDRB |= (1 << PB3) | (1 << PB2) | (1 << PB1) | (1 << PB0); // Output values
    GIMSK |= (1 << INT0); // Enable INT0 (external interrupt)
    MCUCR |= (1 << ISC01) | (1 << ISC10) | (1 << ISC11); // Configure as edge-triggered
}

// Setup All Interrupts
void SetupInterrupts()
{
    ExternalSwitchConfiguration();
    TimerConfiguration();
}

// State Machine for Traffic Lights
void TrafficLightStateMachine()
{
    switch (CurrentState)
    {
    case VEHICLE_GREEN:
        TimeState = MIN_VEHICLE_PASS_TIME;
        PORTB = (0 << PB3) | (1 << PB2) | (1 << PB1) | (0 << PB0);
        if (ButtonPress == 0)
        {
            CurrentState = VEHICLE_GREEN;
        }
        else if (ButtonPress == 1)
        {
            if (Seconds >= 10)
            {
                CycleCount = 0;
                Seconds = 0;
                CurrentState = FLASHING_LIGHT;
            }
        }
        break;

    case FLASHING_LIGHT:
        if (Seconds < 3)
        {
            if (TimeState == MIN_VEHICLE_PASS_TIME)
            {
                if (CycleCount == 30 || CycleCount == 90 || CycleCount == 150)
                    PORTB = (0 << PB3) | (0 << PB2) | (1 << PB1) | (0 << PB0);
                else if (CycleCount == 60 || CycleCount == 120)
                    PORTB = (0 << PB3) | (1 << PB2) | (1 << PB1) | (0 << PB0);
            }
            if (TimeState == FLASHING_TIME)
            {
                if (CycleCount == 30 || CycleCount == 90 || CycleCount == 150)
                    PORTB = (1 << PB3) | (0 << PB2) | (0 << PB1) | (0 << PB0);
                else if (CycleCount == 60 || CycleCount == 120)
                    PORTB = (1 << PB3) | (0 << PB2) | (0 << PB1) | (1 << PB0);
            }
        }
        else
        {
            if (TimeState == MIN_VEHICLE_PASS_TIME)
            {
                Seconds = 0;
                CycleCount = 0;
                CurrentState = VEHICLE_RED;
            }
            if (TimeState == FLASHING_TIME)
            {
                Seconds = 0;
                CycleCount = 0;
                CurrentState = PEDESTRIAN_RED;
            }
        }
        break;

    case VEHICLE_RED:
        PORTB = (1 << PB3) | (0 << PB2) | (1 << PB1) | (0 << PB0);
        if (Seconds >= 1)
        {
            CurrentState = PEDESTRIAN_GREEN;
            CycleCount = 0;
            Seconds = 0;
        }
        else
            CurrentState = VEHICLE_RED;
        break;

    case PEDESTRIAN_GREEN:
        PORTB = (1 << PB3) | (0 << PB2) | (0 << PB1) | (1 << PB0);
        TimeState = FLASHING_TIME;
        if (Seconds >= 10)
        {
            CurrentState = FLASHING_LIGHT;
            CycleCount = 0;
            Seconds = 0;
        }
        else
            CurrentState = PEDESTRIAN_GREEN;
        break;

    case PEDESTRIAN_RED:
        PORTB = (1 << PB1) | (1 << PB3);
        if (Seconds >= 1)
        {
            CurrentState = VEHICLE_GREEN;
            CycleCount = 0;
            Seconds = 0;
            ButtonPress = 0;
        }
        else
            CurrentState = PEDESTRIAN_RED;
        break;
    }
}

int main(void)
{
    // Initialize Output Pins
    PORTB &= (0 << PB0) | (0 << PB1) | (0 << PB2) | (0 << PB3);
    
    CurrentState = VEHICLE_GREEN;
    ButtonPress = 0;
    
    SetupInterrupts();
    
    while (1) 
    {
        TrafficLightStateMachine();
    }
    
    return 0;
}
