/* 
 * File:   Main_lab7.c
 * Author: santr
 *
 * Created on 3 de abril de 2022, 03:05 PM
 */
// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

/*-----------------------Constantes----------------------------------*/
#define SUBIR PORTBbits.RB0
#define BAJAR PORTBbits.RB1

#define _XTAL_FREQ 4000000

/*
 --------------------------Variables-----------------------------
 */

int8_t var_tmr0;
int8_t unidades;
int8_t decenas;
int8_t centenas;
int8_t numero;
int8_t bandera;
uint8_t segU;
uint8_t segD;
uint8_t segC;


/*
 ----------------------Protopipo de funciones-------------------------
 */
void setup(void);
void multi(void);
void convertir(void);
uint8_t Tabla(uint8_t numero7);

/*-----------------------Interrupciones---------------------------------------*/
void __interrupt() isr (void){
    if(INTCONbits.RBIF){            // Fue interrupción del PORTB
        if(RB0 == 1){                 // Verificamos si fue RB0 quien generó la interrupción
            PORTA--;                // Incremento del PORTC (INCF PORTC) 
        }
        if(RB1 == 1){
            PORTA++;
        }
        // NOTA: En clase limpiamos la bandera dentro del if de BOTON presionado,
        //       para evitar problemas es mejor limpiarla en el if de la interrupción
          INTCONbits.RBIF = 0; // Limpiamos bandera de interrupción
    }
    if(INTCONbits.T0IF){
        multi();
        TMR0 = var_tmr0;
        INTCONbits.T0IF = 0;
    }
    
    return;
}


/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();                        // Llamamos a la función de configuraciones
    while(1){
        convertir();
        PORTC = 113;
        // Lo que está adentro del loop es lo que siempre se mantiene ejecutandose
        // cuando el uC está en funcionamiento
    }
    return;
}



/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    var_tmr0 = 248;
    
    ANSEL = 0;
    ANSELH = 0b00000000;        // Usaremos solo I/O digitales
    
    TRISC = 0x00;               
    TRISA = 0x00;
    TRISE = 0x00;
    TRISD = 0x00;               //Puertos A, C, D y E comos salidas
    
    PORTC = 0;                 
    PORTA = 0;
    PORTD = 0;                  // Limpiamos puertos A, C, D y E
    PORTE = 0;
    
    
    
    //TRISB = 1;                // RB0 como entrada (configurada en decimal)
    TRISB = 0b00000011;       // RB0 como entrada (configurada con binario)
    
    
    OPTION_REGbits.nRBPU = 0;   // Habilitamos resistencias de pull-up del PORTB
    WPUBbits.WPUB0 = 1;         // Habilitamos resistencia de pull-up de RB0
    WPUBbits.WPUB1 = 1;
    
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    INTCONbits.RBIE = 1;        // Habilitamos interrupciones del PORTB
    INTCONbits.T0IE = 1;
    IOCBbits.IOCB0 = 1;         // Habilitamos interrupción por cambio de estado para RB0
    IOCBbits.IOCB1 = 1;
    
    INTCONbits.RBIF = 0;        // Limpiamos bandera de interrupción
    INTCONbits.T0IF = 0;
    
    //CONFIGURACION TMR0
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS2 = 1;
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    TMR0 = var_tmr0;
}

/*
 ------------------------Funciones-------------------------------------------
 */
void multi(void){
    PORTE = 0;
    bandera++;
    switch(bandera){
        case 1:
            PORTD = 0;
            PORTE = 0b001;
            segU = Tabla(centenas);  //busacar el valor del 7seg
            PORTD = segU;   //Mostrar el valor en el 7seg
            break;
        case 2:
            PORTD = 0;
            PORTE = 0b010;
            segD = Tabla(decenas);
            PORTD = segD;
            break;
        default:
            PORTD = 0;
            PORTE = 0b100;
            segC = Tabla(unidades);
            PORTD= segC;
            bandera = 0;
            
    }
}
    
void convertir(void){ 
    numero = PORTC;
    centenas = numero / 100;
    numero = numero - (centenas*100);
    decenas = numero / 10;
    unidades = numero - (decenas*10);
    }

uint8_t Tabla(uint8_t numero7)
{
    //recibe un valor de 8 bits que se desea ver en el 7 segmentos
    //devulve el valor apropiado para usar en un 7 segmentos
    
    uint8_t valor, seg;
    seg = numero7;
    
    switch(seg)
    {
        case 0:
            valor= 0b00111111;
            break;
        case 1:
            valor= 0b00000110;
            break;
        case 2:
            valor= 0b01011011;
            break;
        case 3:
            valor= 0b01001111;
            break;
        case 4:
            valor= 0b01100110;
            break;
        case 5:
            valor= 0b01101101;
            break;
        case 6:
            valor= 0b01111101;
            break;
        case 7:
            valor= 0b00000111;
            break;
        case 8:
            valor= 0b01111111;
            break;
        case 9:
            valor= 0b01101111;
            break;
        case 10:
            valor= 0b01110111;
            break;
        case 11:
            valor= 0b01111100;
            break;
        case 12:
            valor= 0b00111001;
            break;
        case 13:
            valor= 0b01011110;
            break;
        case 14:
            valor= 0b01111001;
            break;
        case 15:
            valor= 0b01110001;
            break;     
    }
    return valor;
}

