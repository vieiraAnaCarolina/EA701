/* Maria Letícia Gomes Braga dos Reis - 281318 e Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <util/delay.h>

int main(void) {

unsigned char *ponteiro_ddrb = (unsigned char *) 0x24;
unsigned char *ponteiro_portb = (unsigned char *) 0x25;

*ponteiro_ddrb = *ponteiro_ddrb | 0x20; //Configurar PB5 como saida (usar o ddrb)

while (1) {
    
    *ponteiro_portb = *ponteiro_portb & (~0x20);
    _delay_ms(500);

    *ponteiro_portb = *ponteiro_portb | 0x20;
    _delay_ms(500);
    
    }
}
