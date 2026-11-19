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

volatile unsigned char *ponteiro_TCCR2A = (volatile unsigned char *) 0xB0;
volatile unsigned char *ponteiro_TCCR2B = (volatile unsigned char *) 0xB1;
volatile unsigned char *ponteiro_TIMSK2 = (volatile unsigned char *) 0x70;
volatile unsigned char *ponteiro_OCR2A  = (volatile unsigned char *) 0xB3;
volatile unsigned char *ponteiro_OCR2B  = (volatile unsigned char *) 0xB4;

volatile unsigned char *ponteiro_TCCR0A = (volatile unsigned char *) 0x44;
volatile unsigned char *ponteiro_TCCR0B = (volatile unsigned char *) 0x45;
volatile unsigned char *ponteiro_TIMSK0 = (volatile unsigned char *) 0x6E;
volatile unsigned char *ponteiro_OCR0A  = (volatile unsigned char *) 0x47;

volatile unsigned char *ponteiro_ddrb   = (volatile unsigned char *) 0x24; 
volatile unsigned char *ponteiro_portb  = (volatile unsigned char *) 0x25; 

volatile unsigned char *ponteiro_ddrc   = (volatile unsigned char *) 0x27; 
volatile unsigned char *ponteiro_portc  = (volatile unsigned char *) 0x28;
volatile unsigned char *ponteiro_pinc   = (volatile unsigned char *) 0x26; 

volatile unsigned char *ponteiro_ddrd   = (volatile unsigned char *) 0x2A;

typedef enum { ST_PARADO, ST_FRENTE, ST_TRAS, ST_GIRANDO } estado_t;
 
volatile estado_t estado = ST_PARADO;
volatile unsigned int tempo_giro = 0;      
volatile unsigned char fim_giro = 0;


volatile unsigned char comando_ativo = 'q'; 
volatile unsigned char novo_dado = 0;
volatile unsigned char indice = 0;
volatile const char *msg_TXC0 = 0;
volatile unsigned char rx_char = 'q'; 
volatile int contador_sensor = 0;
volatile unsigned int tempo_msg = 0;
volatile unsigned char girando = 0;

const char msg_frente[]      = "FRENTE\n";
const char msg_tras[]        = "TRAS\n";
const char msg_antihorario[] = "ANTI-HORARIO\n";
const char msg_horario[]     = "HORARIO\n";
const char msg_parado[]      = "PARADO\n";
const char msg_DDDcm[]       = "DDDcm\n";
const char msg_vel70[]       = "Velocidade 70%\n";
const char msg_vel80[]       = "Velocidade 80%\n";
const char msg_vel100[]      = "Velocidade 100%\n";
const char dist_25[]         = "OBSTACULO!\n";

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

   
    *ponteiro_OCR0A  = 0xFA;         //250
    *ponteiro_TIMSK0 = 0x02;
    *ponteiro_TCCR0B = 0x03;         //Prescaler = 64
    *ponteiro_TCCR0A = 0x02;

    // PWM
    *ponteiro_OCR2A  = 0xB3;         // inicia em 70%
    *ponteiro_OCR2B  = 0xB3;         // inicia em 70%
    *ponteiro_TIMSK2 = 0x00;
    *ponteiro_TCCR2B = 0x03;         // prescaler 32
    *ponteiro_TCCR2A = 0xA3;         // Fast PWM, TOP = 0xFF
     



    *ponteiro_ddrb |= 0x08;   // PB3 (pino 11) saída
    *ponteiro_ddrd |= 0x08;   // PD3 (pino 3) saída
    *ponteiro_ddrc  |= 0x3F;   // PC1..PC5 (A1..A5) saídas
    *ponteiro_portc &= ~0x1E; // parado

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
            if (calculo_distancia() > 25 ){
                comando_ativo = 'w';
                escolher_mensagem();
                *ponteiro_portc &= ~0x1E;
                *ponteiro_portc |= 0x18;
                estado = ST_FRENTE;
            }

            else{ // se a distancia for menor q 25 cm envia o alerta OBSTACULO!
                comando_ativo = 'o';
                escolher_mensagem();
                *ponteiro_portc &= ~0x1E;
                estado = ST_PARADO;
            }
            
            break;
        case 's':
            comando_ativo = 's';
            escolher_mensagem();
            *ponteiro_portc &= ~0x1E;
            *ponteiro_portc |= 0x06;
            estado = ST_TRAS;
            break;
        case 'a':
            comando_ativo = 'a';
            escolher_mensagem();
            *ponteiro_portc &= ~0x1E;
            *ponteiro_portc |= 0x14;
            tempo_giro = 0; // inicia a contagem quando inicia o giro
            fim_giro = 0;
            estado = ST_GIRANDO;
            break;
        case 'd':
            comando_ativo = 'd';
            escolher_mensagem();
            *ponteiro_portc &= ~0x1E;
            *ponteiro_portc |= 0x0A;
            tempo_giro = 0; // inicia a contagem quando inicia o giro
            fim_giro = 0;
            estado = ST_GIRANDO;
            break;
        case 'q':
            comando_ativo = 'q';
            escolher_mensagem();
            *ponteiro_portc &= ~0x1E;
            *ponteiro_portc |= 0x00;
            estado = ST_PARADO;
            break;
        case 'e':
            comando_ativo = 'e';
            escolher_mensagem();
            break;
        case '7':
            comando_ativo = '7'; // 255 x 70% = 179
            *ponteiro_OCR2A = 0xB3;  // pino 11
            *ponteiro_OCR2B = 0xB3; // pino 3
            escolher_mensagem();
            break;
        case '8':
            comando_ativo = '8';  // 255 x 80% = 204
            *ponteiro_OCR2A = 0xCC; 
            *ponteiro_OCR2B = 0xCC;
            escolher_mensagem();
            break;
        case '0':
            comando_ativo = '0';  // 255 x 100% = 255
            *ponteiro_OCR2A = 0xFF; 
            *ponteiro_OCR2B = 0xFF;
            escolher_mensagem();
            break;
        
        default:
            escolher_mensagem();
            break;
    }
}

void maquina_estados(void){
    switch (estado){
        case ST_PARADO:
        case ST_TRAS:
            break;                                   // só espera um novo comando
 
        case ST_FRENTE:
            if (ler_distancia() <= 25){              // obstáculo
                *ponteiro_portc &= ~0x1E;
                estado = ST_PARADO;
                comando_ativo = 'o';
                escolher_mensagem();
            }
            break;
 
        case ST_GIRANDO:
            if (fim_giro){                           // passaram 50 ms
                fim_giro = 0;
                *ponteiro_portc &= ~0x1E;
                estado = ST_PARADO;
                comando_ativo = 'q';
                escolher_mensagem();
            }
            break;
    }
}

//Rotina de interrupção relacionada a interrupção temporal (0,5 µs)
ISR(TIMER0_COMPA_vect){
    tempo_msg++;

    if (estado == ST_GIRANDO){
        tempo_giro++;
        if (tempo_giro >= 50) fim_giro = 1;         //  50 ms
    }

    if(tempo_msg >= 1000){ // 1s
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

        case 'o':
            enviar_mensagem(dist_25);
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
