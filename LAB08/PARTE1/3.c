/* Maria Letícia Gomes Braga dos Reis - 281318, Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

//PD4
unsigned char *ponteiro_ddrd  = (unsigned char *) 0x2A;  
unsigned char *ponteiro_portd = (unsigned char *) 0x2B;   
unsigned char *ponteiro_pind  = (unsigned char *) 0x29;   

//Configuração pinchange
unsigned char *ponteiro_PCICR  = (unsigned char *) 0x68;
unsigned char *ponteiro_PCIFR  = (unsigned char *) 0x3B;
unsigned char *ponteiro_PCMSK2 = (unsigned char *) 0x6D;

//Temporizador 0 (0,5 micro)
unsigned char *ponteiro_TCCR0A = (unsigned char *) 0x44;
unsigned char *ponteiro_TCCR0B = (unsigned char *) 0x45;
unsigned char *ponteiro_TCNT0  = (unsigned char *) 0x46;  

volatile int contador = 0;
volatile unsigned char largura_pulso = 0;
volatile unsigned char vetor[40];
volatile unsigned char indice = 0;
volatile unsigned char medida_concluida = 0;

void config(void){

    cli();

    *ponteiro_ddrd |= 0x10;     //PD4 como saída

    *ponteiro_PCICR |= 0x04;    //PCIE2 
    *ponteiro_PCMSK2 |= 0x10;   //PCINT20

    *ponteiro_TCCR0A = 0x00;    //Normal
    *ponteiro_TCCR0B = 0x02;    //prescaler 8

    sei();
}

void primeira_etapa(void){
    
    *ponteiro_ddrd |= 0x10;     //saida
    *ponteiro_portd &= ~0x10;   //nivel baixo

    _delay_ms(20);
    

    *ponteiro_portd |= 0x10;    //nivel alto
}

ISR(PCINT2_vect){
    contador++;

    if (*ponteiro_pind & 0x10) { //borda subida
        *ponteiro_TCNT0 = 0; 
    } 
    else {
        largura_pulso = *ponteiro_TCNT0;  //lê o tempo
        //verificar se é 1 ou 0
        verifica_valor(largura_pulso)
    }
}
    
void verifica_valor(unsigned char largura)
    if (largura >= 45){
        vetor[indice] = 1;

    } else {vetor[indice] = 0;
    }

    indice++;

    if (indice == 40){
        medida_concluida = 1;
        indice = 0;
    }


int main(void){
    config();

    while(1){
    
        primeira_etapa();
        
        *ponteiro_ddrd &= ~0x10; //entrada 
        
        _delay_ms(2000); 
    }

    return 0;
}