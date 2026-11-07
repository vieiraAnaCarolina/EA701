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
volatile int contador_atv = 0;
volatile int ativacao = 0;

int ETAPA1 = 0; //estado 1 da etapa 1

void config(void){

    cli();

    *ponteiro_ddrd |= 0x10;     //PD4 como saída

    *ponteiro_PCICR |= 0x04;    //PCIE2 
    *ponteiro_PCMSK2 |= 0x10;   //PCINT20

    *ponteiro_TCCR0A = 0x00;    //Normal
    *ponteiro_TCCR0B = 0x02;    //prescaler 8

    sei();
}

ISR(TIMER0_COMPA_vect){
    contador ++;
    contador_atv ++;
}

int main(void){
    config();

    while(1){
     
        switch (ETAPA1) {
            case (0):
                if(ativacao == 0){
                    if(contador_atv >= 40000){
                        ativacao = 1;
                        contador_atv = 0;
                
                    }
                    else{
                        ETAPA1 = 0;
                    }
                }

                else{
                    *ponteiro_ddrd |= 0x10;  //saida
                    *ponteiro_portd &= ~0x10; //nivel baixo
                    contador = 0;
                    ETAPA1 = 1;
                }
                break;

            case (1):

                if (contador >= 40){
                    contador = 0;
                    *ponteiro_ddrd &= ~0x10;  //entrada
                    ativacao = 0;
                    contador_atv = 0;
                    ETAPA1 = 0;

                }

                else{
                    *ponteiro_portd &= ~0x10;
                    ETAPA1 = 1;
                }
                break;
         
        }

        return 0;
    }

}