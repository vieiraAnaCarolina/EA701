//treinando interrupções

#define F_CPU 16000000UL
#include <avr/io.h>        
#include <avr/interrupt.h>
#include <util/delay.h>

//Configuração das interrupções (Recepção Completa e Transmissão Completa)
volatile unsigned char *ponteiro_UBRR0H = (volatile unsigned char *) 0xC5;
volatile unsigned char *ponteiro_UBRR0L = (volatile unsigned char *) 0xC4;
volatile unsigned char *ponteiro_UCSR0A = (volatile unsigned char *) 0xC0;
volatile unsigned char *ponteiro_UCSR0B = (volatile unsigned char *) 0xC1;
volatile unsigned char *ponteiro_UCSR0C = (volatile unsigned char *) 0xC2;
volatile unsigned char *ponteiro_UDR0   = (volatile unsigned char *) 0xC6;

//Configuração dos leds
volatile unsigned char *ponteiro_ddrb = (volatile unsigned char *) 0x24;
volatile unsigned char *ponteiro_portb   = (volatile unsigned char *) 0x25;

const char msg_desligado[] = "Desligado\n"
const char msg_vermelho[] = "Pisca vermelho\n"
const char msg_verde[] = "Pisca verde\n"
const char msg_azul[] = "Pisca azul\n"
const char msg_todos[] = "Pisca todos\n"
const char msg_erro[] = "Comando incorreto\n"


void config(void){

    ()cli

    *ponteiro_UBRR0H = 0x00;
    *ponteiro_UBRR0L = 0x33;                 
    *ponteiro_UCSR0A = 0x00;                 
    *ponteiro_UCSR0B = 0xD8; 
    *ponteiro_UCSR0C = 0x06;

    *ponteiro_ddrb |= 0x08; //LED PB3 COMO SAIDA
    *ponteiro_ddrb |= 0x10; //LED PB4 COMO SAIDA
    *ponteiro_ddrb |= 0x20; //LED PB5 COMO SAIDA

    *ponteiro_portb &= ~0x38; //TODOS OS LEDS COMEÇAM DESLIGADOS

    ()sei
}

ISR(USART_RX_vect){


}

ISR(USART_TX_vect){


    
}