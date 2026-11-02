/* Maria Letícia Gomes Braga dos Reis - 281318 e Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned char estado = 0;

volatile unsigned char *ponteiro_ddrb  = (volatile unsigned char *) 0x24; 
volatile unsigned char *ponteiro_portb = (volatile unsigned char *) 0x25; 

volatile unsigned char *ponteiro_ddrd  = (volatile unsigned char *) 0x2A; 
volatile unsigned char *ponteiro_pind  = (volatile unsigned char *) 0x29; 

//Para lidar com interrupção usando Pin Change Interrupts.
volatile unsigned char *ponteiro_PCICR =  (volatile unsigned char *) 0x68;
volatile unsigned char *ponteiro_PCMSK2 = (volatile unsigned char *) 0x6D; 
volatile unsigned char *ponteiro_PCIFR =  (volatile unsigned char *) 0x3B;

void config(void) {
    cli();                          //DESATIVA INTERUPÇOES GLOBAIS

    *ponteiro_ddrb  |= 0x20;        //LED COMO SAIDA
    *ponteiro_portb &= ~0x20;       //LED COMEÇA DESLIGADO
    *ponteiro_ddrd  &= ~0x04;       //BOTAO COMO SAIDA

    *ponteiro_PCICR |= 0x04;        //ATIVA PCIE2 (00000100)
    *ponteiro_PCMSK2 |= 0x04;       //PCINT18 (?)

    sei();                          //HABILITA INTERRUPÇOES GLOBAIS
}

ISR(PCINT2_vect) {
    estado = 1;                           // borda detectada
    *ponteiro_PCMSK2 &= ~0x04;            // desativa temporariamente a máscara
    *ponteiro_PCIFR  |= 0x04;             // limpa a flag escrevendo 1 no bit PCIF2
}

unsigned char filtragem(void) {
    _delay_ms(20);
    
    if ((*ponteiro_pind & 0x04) == 0) {   // se continuar em nível lógico baixo, a borda é válida
        return 1;
    }
    
    return 0; 
}

void acionamento_led(void) {
    if (estado == 1) {
        if (filtragem() == 1) {
            *ponteiro_portb ^= 0x20; 
        }
        
        estado = 0; 
        *ponteiro_PCIFR  |= 0x04;           // retorna ao estado inicial 
        *ponteiro_PCMSK2 |= 0x04;           // reabilita a interrupção 
        
    }
}

int main(void) {
    config();

    while (1) {
        acionamento_led(); 
    }

    return 0;
}