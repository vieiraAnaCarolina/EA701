/* Maria Letícia Gomes Braga dos Reis - 281318, Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

unsigned char *ponteiro_ddrd  = (unsigned char *) 0x2A;   //PD4 
unsigned char *ponteiro_portd = (unsigned char *) 0x2B;   //PD4

void config(void){

    cli();

    *ponteiro_ddrd |= 0x10;  //pd4 como saída

    sei();
}


void primeira_etapa(void){

    *ponteiro_portd &= ~0x10; //pd4 nivel baixo

    _delay_ms(20);

    *ponteiro_portd |= 0x10; //pd4 nivel alto


}


int main(void){

    config();

    while(1){

    *ponteiro_ddrd |= 0x10; 
    
    primeira_etapa();

    *ponteiro_ddrd &= ~0x10;  //pd4 como entrada
    
    _delay_ms(2000);
    }

    return 0;
    
    

}
