/* 
 * File:   SpeedInputCapture.c
 * Author: Anand Desai
 * Created on December 7, 2017, 7:30 PM
 *
 * Calculates wheel speed using input capture at each tick of the wheel.
 */

#include "device.h"
#include <xc.h>
#define PI 3.14159265358979323846
#define WHEEL_TEETH 20   //no. of ticks read per revolution
#define  WHEEL_RADIUS 0.18 // wheel radius in meters
#define units_kmh 3.6 // factor to change speed from m/s to km/h
#define UNITS_MPH 2.25 // factor to change speed from m/s to mph
unsigned int t0_init = 0, t1_init = 0, t0_final, t1_final, t0_period, t1_period;
const double circum = WHEEL_RADIUS * 2 * PI;
double rps_0, rps_1;
// Setups IC1 registers

void IC1_setup() {
    //Input capture setup
    IC1CON1bits.ICTSEL = 0b100; // Timer 1 is clock source
    IC1CON1bits.ICI = 0b000; //interrupt on every capture
    IC1CON1bits.ICM = 0b010; //captures every falling edge
    IPC0bits.IC1IP = 5; // Setup interrupt priority level
    IFS0bits.IC1IF = 0; // clears interrupt flag
    IEC0bits.IC1IE = 1; // enables interrupt
    IC1CON1bits.ICBNE = 0b000; // IC buffer is empty
}
// Setups IC2 registers

void IC2_setup() {
    //Input capture setup
    IC2CON1bits.ICTSEL = 0b100; // Timer 1 is clock source
    IC2CON1bits.ICI = 0b000; //interrupt on every capture
    IC2CON1bits.ICM = 0b010; //captures every falling edge
    IPC1bits.IC2IP = 5; // Setup interrupt priority level
    IFS0bits.IC2IF = 0; // clears interrupt flag
    IEC0bits.IC2IE = 1; // enables interrupt
    IC2CON1bits.ICBNE = 0b000; // IC buffer is empty
}

//  Initializes TMR1 (idk if this is even needed?)

void timer_setup() {

    T1CONbits.TCS = 0; //internal clock
    T1CONbits.TCKPS = 0b11; // Pre-scaler=256
    TMR1 = 0; //timer register cleared
    T1CONbits.TON = 1; // timer on

}

//  capture interrupt for IC1

void __attribute__((interrupt, no_auto_psv)) _IC1Interrupt() {
    t0_final = IC1BUF; // final time for IC1
    if (t0_init < t0_final) {
        t0_period = t0_final - t0_init;
    } else {
        t0_period = 65535 - t0_init + t0_final;
    }
    t0_init = t0_final;
    IFS0bits.IC1IF = 0; // Clear IC1 interrupt flag
}

//  capture interrupt for IC2

void __attribute__((interrupt, no_auto_psv)) _IC2Interrupt() {
    t1_final = IC2BUF; // final time for IC2
    if (t1_init < t1_final) {
        t1_period = t1_final - t1_init;
    } else {
        t1_period = 65535 - t1_init + t1_final;
    }
    t1_init = t1_final;
    IFS0bits.IC2IF = 0; // Clear IC2 interrupt flag
}

//Calculates wheel speed

void wheel_speed_calc() {
    rps_0 = 1 / (t0_period * WHEEL_TEETH); // revolutions per second for wheel_0
    rps_1 = 1 / (t1_period * WHEEL_TEETH); // revolutions per second for wheel_0

    double speed_0 = rps_0 * circum*UNITS_MPH; //calculates speed of wheel_0
    double speed_1 = rps_1 * circum*UNITS_MPH; //calculates speed of wheel_0

}

int main() {

    TRISBbits.TRISB14 = 1;
    TRISBbits.TRISB15 = 1;
    RPINR7bits.IC1R = 0x002E;
    RPINR7bits.IC2R = 0x002F;

    timer_setup();
    IC1_setup();
    IC2_setup();

    while (1) {
        wheel_speed_calc();
    }
    return 0;

}