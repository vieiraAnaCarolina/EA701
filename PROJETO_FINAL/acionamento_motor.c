/* Maria Letícia Gomes Braga dos Reis - 281318 e Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <avr/io.h>        
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned char *ponteiro_UBRR0H = (volatile unsigned char *) 0xC5;
volatile unsigned char *ponteiro_UBRR0L = (volatile unsigned char *) 0xC4;
volatile unsigned char *ponteiro_UCSR0A = (volatile unsigned char *) 0xC0;
volatile unsigned char *ponteiro_UCSR0B = (volatile unsigned char *) 0xC1;
volatile unsigned char *ponteiro_UCSR0C = (volatile unsigned char *) 0xC2;
volatile unsigned char *ponteiro_UDR0   = (volatile unsigned char *) 0xC6;

volatile unsigned char *ponteiro_TCCR0A = (volatile unsigned char *) 0x44;
volatile unsigned char *ponteiro_TCCR0B = (volatile unsigned char *) 0x45;
volatile unsigned char *ponteiro_TIMSK0 = (volatile unsigned char *) 0x6E;
volatile unsigned char *ponteiro_OCR0A  = (volatile unsigned char *) 0x47;

volatile unsigned char *ponteiro_ddrb   = (volatile unsigned char *) 0x24; 
volatile unsigned char *ponteiro_portb = (volatile unsigned char *) 0x25; 

volatile unsigned char *ponteiro_ddrc   = (volatile unsigned char *) 0x27; 
volatile unsigned char *ponteiro_portc = (volatile unsigned char *) 0x28;
volatile unsigned char *ponteiro_pinc  = (volatile unsigned char *) 0x26; 


volatile unsigned char comando_ativo = 'q'; 
volatile unsigned char novo_dado = 0;
volatile unsigned char indice = 0;
volatile const char *msg_TXC0 = 0;
volatile unsigned char rx_char = 'q'; 
int tempo_msg =0;


const char msg_frente[]      = "FRENTE\n";
const char msg_tras[]        = "TRAS\n";
const char msg_antihorario[] = "ANTI-HORARIO\n";
const char msg_horario[]     = "HORARIO\n";
const char msg_parado[]      = "PARADO\n";
const char msg_DDDcm[]       = "DDDcm\n";
const char msg_vel70[]       = "Velocidade 70%\n";
const char msg_vel80[]       = "Velocidade 80%\n";
const char msg_vel100[]      = "Velocidade 100%\n";

void config(void);
void enviar_mensagem(const char *msg);
void escolher_mensagem(void);
void processar_entrada(void);
void atualizar_leds(void);

void config(void){
    cli();

    *ponteiro_UBRR0H = 0x00;
    *ponteiro_UBRR0L = 0x33;                 
    *ponteiro_UCSR0A = 0x00;                 
    *ponteiro_UCSR0B = 0xD8; 
    *ponteiro_UCSR0C = 0x06; 

    *ponteiro_OCR0A  = 0xF9;         //250
    *ponteiro_TIMSK0 = 0x02;
    *ponteiro_TCCR0B = 0x03;         //Prescaler = 64
    *ponteiro_TCCR0A = 0x02; 

    *ponteiro_pinc &= ~0x1E;  // zera os valores de PINC1 a PINC4 --> estado parado
    sei();
}

void enviar_mensagem(const char *msg){
    msg_TXC0 = msg;
    indice = 0;
    *ponteiro_UDR0 = msg_TXC0[indice]; 
}

ISR(USART_RX_vect){
    unsigned char dado_recebido = *ponteiro_UDR0;
    
    if (dado_recebido != '\n' && dado_recebido != '\r') {
        rx_char = dado_recebido;
        novo_dado = 1;
    }
}

ISR(USART_TX_vect){
    indice++;
    if(msg_TXC0[indice] != '\0') { 
        *ponteiro_UDR0 = msg_TXC0[indice]; 
    }
}

void processar_entrada(void){
    switch (rx_char){
        case 'w':
            comando_ativo = 'w';
            escolher_mensagem();
            *ponteiro_pinc |= 0x18;
            break;
        case 's':
            comando_ativo = 's';
            escolher_mensagem();
            *ponteiro_pinc |= 0x06;
            break;
        case 'a':
            comando_ativo = 'a';
            escolher_mensagem();
            *ponteiro_pinc |= 0x14;
            break;
        case 'd':
            comando_ativo = 'd';
            escolher_mensagem();
            *ponteiro_pinc |= 0x0A;
            break;
        case 'q':
            comando_ativo = 'q';
            escolher_mensagem();
            *ponteiro_pinc |= 0x00;
            break;
        case 'e':
            comando_ativo = 'e';
            escolher_mensagem();
            break;
        case '7':
            comando_ativo = '7';
            escolher_mensagem();
            break;
        case '8':
            comando_ativo = '8';
            escolher_mensagem();
            break;
        case '0':
            comando_ativo = '0';
            escolher_mensagem();
            break;
        default:
            escolher_mensagem();
            break;
    }
}

//Rotina de interrupção relacionada a interrupção temporal
ISR(TIMER0_COMPA_vect){
    tempo_msg++;
    if(tempo_msg >= 1000){ 
        escolher_mensagem();
        tempo_msg = 0;
    }  
    
}

void escolher_mensagem(void){
    switch (comando_ativo){
        case 'w': 
            enviar_mensagem(msg_frente);
            break;

        case 's': 
            enviar_mensagem(msg_tras);
            break;
                    
        case 'a': 
            enviar_mensagem(msg_antihorario);
            break;

        case 'd': 
            enviar_mensagem(msg_horario);
            break;

        case 'q': 
            enviar_mensagem(msg_parado);
            break;
        case 'e': 
            enviar_mensagem(msg_DDDcm);
            break;

        case '7': 
            enviar_mensagem(msg_vel70);
            break;

        case '8': 
            enviar_mensagem(msg_vel80);
            break;
        case '0': 
            enviar_mensagem(msg_vel100);
            break;

        default: 
            break;
    }
}

int main (void){
    config();

    while(1){

        if (novo_dado == 1){
            novo_dado = 0;
            processar_entrada();
        }

    }

    return 0;
}