/* Maria Letícia Gomes Braga dos Reis - 281318, Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <avr/interrupt.h>

//PD4
unsigned char *ponteiro_ddrd  = (unsigned char *) 0x2A;  
unsigned char *ponteiro_portd = (unsigned char *) 0x2B;   
unsigned char *ponteiro_pind  = (unsigned char *) 0x29;   

//Configuração pinchange (usada a partir da Etapa 2)
unsigned char *ponteiro_PCICR  = (unsigned char *) 0x68;
unsigned char *ponteiro_PCIFR  = (unsigned char *) 0x3B;
unsigned char *ponteiro_PCMSK2 = (unsigned char *) 0x6D;

//Temporizador 0 (tick de 0,5 ms)
unsigned char *ponteiro_TCCR0A = (unsigned char *) 0x44;
unsigned char *ponteiro_TCCR0B = (unsigned char *) 0x45;
unsigned char *ponteiro_TCNT0  = (unsigned char *) 0x46;
unsigned char *ponteiro_OCR0A  = (unsigned char *) 0x47;
unsigned char *ponteiro_TIMSK0 = (unsigned char *) 0x6E;

volatile unsigned int contador = 0;      
volatile unsigned int contador_atv = 0;  
volatile int ativacao = 0;

typedef enum { REPOUSO, ETAPA1_START } estado_t;

estado_t estado = REPOUSO;

void config(void){

    cli();

    *ponteiro_ddrd  &= ~0x10;   //PD4 como entrada 
    *ponteiro_portd |=  0x10;   //pull-up: linha em nível alto

    *ponteiro_TCCR0A = 0x02;    //CTC
    *ponteiro_TCCR0B = 0x03;    //prescaler 64 
    *ponteiro_OCR0A  = 124;     //125 contagens x 4 us = 0,5 ms
    *ponteiro_TIMSK0 = 0x02;    //OCIE0A: habilita interrupção do comparador A

    sei();
}

ISR(TIMER0_COMPA_vect){
    contador ++;
    contador_atv ++;
}

int main(void){
    config();

    while(1){
     
        switch (estado) {
            case REPOUSO:
                if(ativacao == 0){
                    if(contador_atv >= 4000){      //2 s em repouso
                        ativacao = 1;
                        contador_atv = 0;
                    }
                    else{
                        estado = REPOUSO;
                    }
                }

                else{
                    *ponteiro_portd &= ~0x10; //nivel baixo
                    *ponteiro_ddrd |= 0x10;   //saida
                    contador = 0;
                    estado = ETAPA1_START;
                }
                break;

            case ETAPA1_START:

                if (contador >= 40){          //40 x 0,5 ms = 20 ms
                    contador = 0;
                    *ponteiro_ddrd &= ~0x10;  //entrada 
                    *ponteiro_portd |= 0x10;  //pull-up
                    ativacao = 0;
                    contador_atv = 0;
                    estado = REPOUSO;
                }

                else{
                    *ponteiro_portd &= ~0x10;
                    estado = ETAPA1_START;
                }
                break;
         
        }
    }

    return 0;
}
