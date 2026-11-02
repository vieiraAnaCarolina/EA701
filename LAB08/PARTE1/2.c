/* Maria Letícia Gomes Braga dos Reis - 281318, Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

//PD4
unsigned char *ponteiro_ddrd  = (unsigned char *) 0x2A;    
unsigned char *ponteiro_portd = (unsigned char *) 0x2B;   

//Configuração pinchange
unsigned char *ponteiro_PCICR = (unsigned char *) 0x68;
unsigned char *ponteiro_PCIFR = (unsigned char *) 0x3B;
unsigned char *ponteiro_PCMSK2 = (unsigned char *) 0x6D;

volatile int contador = 0;

void config(void){

    cli();

    *ponteiro_ddrd |= 0x10;  //PD4 como saída
    
    *ponteiro_PCICR |= 0x04;  //Ativa PCIE2
    *ponteiro_PCMSK2 |= 0x10; //Ativa PCINT20

    sei();
}

void primeira_etapa(void){

    *ponteiro_portd &= ~0x10; //pd4 nivel baixo

    _delay_ms(20);

    *ponteiro_portd |= 0x10; //pd4 nivel alto


}

ISR(PCINT2_vect){
    contador++;

}

int main(void){

    config();

    while(1){

    *ponteiro_ddrd |= 0x10; 
    
    primeira_etapa();

    *ponteiro_ddrd &= ~0x10;  //entrada
    
    _delay_ms(2000);
    }

    return 0;
    
}
