/* Maria Letícia Gomes Braga dos Reis - 281318 e Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// Registradores da UART0
volatile unsigned char *ponteiro_UBRR0H = (volatile unsigned char *) 0xC5;
volatile unsigned char *ponteiro_UBRR0L = (volatile unsigned char *) 0xC4;
volatile unsigned char *ponteiro_UCSR0A = (volatile unsigned char *) 0xC0;
volatile unsigned char *ponteiro_UCSR0B = (volatile unsigned char *) 0xC1;
volatile unsigned char *ponteiro_UCSR0C = (volatile unsigned char *) 0xC2;
volatile unsigned char *ponteiro_UDR0   = (volatile unsigned char *) 0xC6;

// Registradores do Timer 2 (PWM dos motores: pino 11 / OC2A e pino 3 / OC2B)
volatile unsigned char *ponteiro_TCCR2A = (volatile unsigned char *) 0xB0;
volatile unsigned char *ponteiro_TCCR2B = (volatile unsigned char *) 0xB1;
volatile unsigned char *ponteiro_TIMSK2 = (volatile unsigned char *) 0x70;
volatile unsigned char *ponteiro_OCR2A  = (volatile unsigned char *) 0xB3;
volatile unsigned char *ponteiro_OCR2B  = (volatile unsigned char *) 0xB4;

// Registradores do Timer 0 (base de 1 ms em COMPA e pulso TRIG em COMPB)
volatile unsigned char *ponteiro_TIFR0  = (volatile unsigned char *) 0x35;
volatile unsigned char *ponteiro_TCCR0A = (volatile unsigned char *) 0x44;
volatile unsigned char *ponteiro_TCCR0B = (volatile unsigned char *) 0x45;
volatile unsigned char *ponteiro_TCNT0  = (volatile unsigned char *) 0x46;
volatile unsigned char *ponteiro_OCR0A  = (volatile unsigned char *) 0x47;
volatile unsigned char *ponteiro_OCR0B  = (volatile unsigned char *) 0x48;
volatile unsigned char *ponteiro_TIMSK0 = (volatile unsigned char *) 0x6E;

// Registradores do Timer 1 (frequência de cintilação do LED em A5 / PC5)
volatile unsigned char *ponteiro_TIMSK1 = (volatile unsigned char *) 0x6F;
volatile unsigned char *ponteiro_TCCR1A = (volatile unsigned char *) 0x80;
volatile unsigned char *ponteiro_TCCR1B = (volatile unsigned char *) 0x81;
volatile unsigned int  *ponteiro_OCR1A  = (volatile unsigned int  *) 0x88;

// Registradores de Interrupção Externa (INT0 -> pino 2 / PD2 para ECHO)
volatile unsigned char *ponteiro_EIMSK  = (volatile unsigned char *) 0x3D;
volatile unsigned char *ponteiro_EICRA  = (volatile unsigned char *) 0x69;

// Registradores das Portas B, C e D
volatile unsigned char *ponteiro_ddrb   = (volatile unsigned char *) 0x24;
volatile unsigned char *ponteiro_portb  = (volatile unsigned char *) 0x25;

volatile unsigned char *ponteiro_pinc   = (volatile unsigned char *) 0x26;
volatile unsigned char *ponteiro_ddrc   = (volatile unsigned char *) 0x27;
volatile unsigned char *ponteiro_portc  = (volatile unsigned char *) 0x28;

volatile unsigned char *ponteiro_pind   = (volatile unsigned char *) 0x29;
volatile unsigned char *ponteiro_ddrd   = (volatile unsigned char *) 0x2A;
volatile unsigned char *ponteiro_portd  = (volatile unsigned char *) 0x2B;

typedef enum { ST_PARADO, ST_FRENTE, ST_TRAS, ST_GIRANDO } estado_t;

volatile estado_t estado = ST_PARADO;
volatile unsigned int tempo_giro = 0;
volatile unsigned char fim_giro = 0;

volatile unsigned char comando_ativo = 'q';
volatile unsigned char novo_dado = 0;
volatile unsigned char indice = 0;
volatile const char *msg_TXC0 = 0;
volatile unsigned char rx_char = 'q';
volatile unsigned int tempo_msg = 0;

// Variáveis do módulo do sensor ultrassônico (HC-SR04) e LED A5
volatile unsigned int contador_sensor = 0;
volatile unsigned char disparar_sensor = 0;

volatile unsigned char echo_em_curso = 0;
volatile unsigned int echo_ms = 0;
volatile unsigned char echo_tick_inicio = 0;
volatile unsigned int T_us = 0;
volatile unsigned char nova_medicao = 0;
volatile int distancia_cm = 400;

const char msg_frente[]      = "FRENTE\n";
const char msg_tras[]        = "TRAS\n";
const char msg_antihorario[] = "ANTI-HORARIO\n";
const char msg_horario[]     = "HORARIO\n";
const char msg_parado[]      = "PARADO\n";
char msg_DDDcm[]             = "000cm\n";
const char msg_vel70[]       = "Velocidade 70%\n";
const char msg_vel80[]       = "Velocidade 80%\n";
const char msg_vel100[]      = "Velocidade 100%\n";
const char dist_25[]         = "OBSTACULO!\n";

void config(void);
void config_sensor(void);
void enviar_mensagem(const char *msg);
void escolher_mensagem(void);
void processar_entrada(void);
void maquina_estados(void);
void inicia_trigger(void);
void atualiza_temporizacao_sensor_1ms(void);
int calculo_distancia(unsigned int tempo_em_us);
void atualiza_velocidade_led(int dist_cm);
void formatar_distancia_ascii(int dist);


void config_sensor(void) {
    
    *ponteiro_ddrd  &= 0xFB;
    *ponteiro_portd &= 0xFB;


    *ponteiro_ddrc  |= 0x21;
    *ponteiro_portc &= 0xFE; 


    *ponteiro_EICRA = ((*ponteiro_EICRA) & 0xFC) | 0x01; 
    *ponteiro_EIMSK = (*ponteiro_EIMSK) | 0x01;          


    *ponteiro_TCCR1A = 0x00;
    *ponteiro_TCCR1B = 0x0C;
    *ponteiro_OCR1A  = 31250; 
    *ponteiro_TIMSK1 = 0x02;  
}


void config(void){
    cli();

    
    *ponteiro_UBRR0H = 0x00;
    *ponteiro_UBRR0L = 0x67;   
    *ponteiro_UCSR0A = 0x00;   
    *ponteiro_UCSR0B = 0xD8;    
    *ponteiro_UCSR0C = 0x06;   

    
    *ponteiro_TCNT0  = 0x00;
    *ponteiro_OCR0A  = 0xF9;     // 249 (250 ticks x 4 us = 1000 us = 1 ms)
    *ponteiro_TIMSK0 = 0x02;     // Habilita OCIE0A
    *ponteiro_TCCR0A = 0x02;     // Modo CTC (WGM01 = 1)
    *ponteiro_TCCR0B = 0x03;     // Prescaler = 64

    // Timer 2: PWM dos motores nos pinos 11 (OC2A) e 3 (OC2B), Prescaler = 32
    *ponteiro_OCR2A  = 0xB3;     // Inicia em 70%
    *ponteiro_OCR2B  = 0xB3;     // Inicia em 70%
    *ponteiro_TIMSK2 = 0x00;
    *ponteiro_TCCR2B = 0x03;     // Prescaler = 32
    *ponteiro_TCCR2A = 0xA3;     // Fast PWM não invertido em OC2A e OC2B

    // Pinos dos motores (PWM e direção A1..A4)
    *ponteiro_ddrb  |= 0x08;     // PB3 (pino 11 - OC2A) como saída
    *ponteiro_ddrd  |= 0x08;     // PD3 (pino 3 - OC2B) como saída
    *ponteiro_ddrc  |= 0x1E;     // PC1..PC4 (A1..A4) como saídas
    *ponteiro_portc &= 0xE1;     // Inicia motores parados (A1..A4 = 0)

    config_sensor();

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

void formatar_distancia_ascii(int dist) {
    if (dist < 0) dist = 0;
    if (dist > 999) dist = 999;

    msg_DDDcm[0] = (dist / 100) + '0';
    msg_DDDcm[1] = ((dist / 10) % 10) + '0';
    msg_DDDcm[2] = (dist % 10) + '0';
}

void processar_entrada(void){
    switch (rx_char){
        case 'w':
            // Bloqueia movimento para frente se houver obstáculo a <= 25 cm
            if (distancia_cm <= 25) {
                *ponteiro_portc &= 0xE1; // Para os motores (A1..A4 = 0)
                estado = ST_PARADO;
                comando_ativo = 'o';
                escolher_mensagem();
            } else {
                comando_ativo = 'w';
                escolher_mensagem();
                *ponteiro_portc &= 0xE1;
                *ponteiro_portc |= 0x18; // A1=0, A2=0, A3=1, A4=1 (Frente)
                estado = ST_FRENTE;
            }
            break;

        case 's':
            comando_ativo = 's';
            escolher_mensagem();
            *ponteiro_portc &= 0xE1;
            *ponteiro_portc |= 0x06; // A1=1, A2=1, A3=0, A4=0 (Trás)
            estado = ST_TRAS;
            break;

        case 'a':
            comando_ativo = 'a';
            escolher_mensagem();
            *ponteiro_portc &= 0xE1;
            *ponteiro_portc |= 0x14; // A1=0, A2=1, A3=0, A4=1 (Anti-horário)
            tempo_giro = 0;
            fim_giro = 0;
            estado = ST_GIRANDO;
            break;

        case 'd':
            comando_ativo = 'd';
            escolher_mensagem();
            *ponteiro_portc &= 0xE1;
            *ponteiro_portc |= 0x0A; // A1=1, A2=0, A3=1, A4=0 (Horário)
            tempo_giro = 0;
            fim_giro = 0;
            estado = ST_GIRANDO;
            break;

        case 'q':
            comando_ativo = 'q';
            escolher_mensagem();
            *ponteiro_portc &= 0xE1; // A1..A4 = 0 (Parar)
            estado = ST_PARADO;
            break;

        case 'e':
            comando_ativo = 'e';
            formatar_distancia_ascii(distancia_cm);
            escolher_mensagem();
            break;

        case '7':
            comando_ativo = '7';
            *ponteiro_OCR2A = 0xB3; // 70%
            *ponteiro_OCR2B = 0xB3;
            escolher_mensagem();
            break;

        case '8':
            comando_ativo = '8';
            *ponteiro_OCR2A = 0xCC; // 80%
            *ponteiro_OCR2B = 0xCC;
            escolher_mensagem();
            break;

        case '0':
            comando_ativo = '0';
            *ponteiro_OCR2A = 0xFF; // 100%
            *ponteiro_OCR2B = 0xFF;
            escolher_mensagem();
            break;
            
        default:
            // Comandos inválidos são ignorados e não alteram o comportamento
            break;
    }
}

void maquina_estados(void){
    switch (estado){
        case ST_PARADO:
        case ST_TRAS:
            break;

        case ST_FRENTE:
            // Se um obstáculo a <= 25 cm for detectado durante o avanço, para e avisa
            if (distancia_cm <= 25) {
                *ponteiro_portc &= 0xE1;
                estado = ST_PARADO;
                comando_ativo = 'o';
                escolher_mensagem();
            }
            break;
            
        case ST_GIRANDO:
            if (tempo_giro >= 50) {     //  50 ms
                *ponteiro_portc &= ~0x1E;  
                estado = ST_PARADO;  
                escolher_mensagem();

            }; 
            break;
    }
}

// ----------------------------------------------------------------------------
// Disparo do pulso TRIG (>= 10 us) em PC0 (A0) via Timer 0 COMPB
// ----------------------------------------------------------------------------
void inicia_trigger(void){
    unsigned char alvo;

    *ponteiro_portc |= 0x01;       // TRIG (PC0) em nível alto

    alvo = (*ponteiro_TCNT0) + 3;  // 3 ticks x 4 us = 12 us (>= 10 us)
    if (alvo >= 250) {
        alvo = alvo - 250;
    }
    *ponteiro_OCR0B  = alvo;
    *ponteiro_TIFR0  = 0x04;       // Limpa flag OCF0B (bit 2)
    *ponteiro_TIMSK0 |= 0x04;      // Habilita interrupção COMPB
}

ISR(TIMER0_COMPB_vect){
    *ponteiro_portc  &= 0xFE;      // TRIG (PC0) em nível baixo
    *ponteiro_TIMSK0 &= 0xFB;      // Desabilita interrupção COMPB até o próximo disparo
}

// ----------------------------------------------------------------------------
// Medição da largura T do pulso ECHO (PD2 / INT0) por borda de subida e descida
// ----------------------------------------------------------------------------
ISR(INT0_vect) {
    unsigned char tick_atual = *ponteiro_TCNT0;

    if (((*ponteiro_pind) & 0x04) != 0) {
        // Borda de subida: inicia contagem
        echo_tick_inicio = tick_atual;
        echo_ms = 0;
        echo_em_curso = 1;
    } else if (echo_em_curso) {
        // Borda de descida: calcula duração total em microssegundos
        echo_em_curso = 0;

        int delta_ticks = (int)tick_atual - (int)echo_tick_inicio;
        int ticks_totais = ((int)echo_ms * 250) + delta_ticks;

        if (ticks_totais < 0) {
            ticks_totais = 0;
        }

        T_us = (unsigned int)ticks_totais * 4; // 1 tick do Timer 0 = 4 us
        nova_medicao = 1;
    }
}

// ----------------------------------------------------------------------------
// Atualização periódica de 1 ms do sensor e da base de tempo geral
// ----------------------------------------------------------------------------
void atualiza_temporizacao_sensor_1ms(void) {
    contador_sensor++;

    if (echo_em_curso) {
        echo_ms++;
    }

    if (contador_sensor >= 200) { // Mede periodicamente a cada 200 ms
        contador_sensor = 0;
        disparar_sensor = 1;
    }
}

ISR(TIMER0_COMPA_vect){
    tempo_msg++;
    atualiza_temporizacao_sensor_1ms();

    if (estado == ST_GIRANDO){
        tempo_giro++;
   
    }

    if (tempo_msg >= 1000){ // Reenvia mensagem do comando ativo a cada 1 s
        tempo_msg = 0;
        escolher_mensagem();
    }
}

// ----------------------------------------------------------------------------
// Cálculo da distância em cm: distância = velocidade_do_som * (T / 2)
// ----------------------------------------------------------------------------
int calculo_distancia(unsigned int tempo_em_us){
    float t_segundos = (float)tempo_em_us / 1000000.0;
    float dist_metros = 343.0 * (t_segundos / 2.0);
    return (int)(dist_metros * 100.0);
}

// ----------------------------------------------------------------------------
// Controle do LED em A5 (PC5) com frequência inversamente proporcional à distância
// ----------------------------------------------------------------------------
void atualiza_velocidade_led(int dist_cm) {
    int dist_min = 4;
    int dist_max = 400;

    if (dist_cm < dist_min) dist_cm = dist_min;
    if (dist_cm > dist_max) dist_cm = dist_max;

    unsigned int ocr_min = 3000;   // Perto (4 cm)  -> pisca rápido (~10 Hz)
    unsigned int ocr_max = 31250;  // Longe (400 cm) -> pisca lento (1 Hz)

    // Mapeamento linear evitando estouro de 16 bits:
    // (ocr_max - ocr_min) / (dist_max - dist_min) = 28250 / 396 ~= 71
    unsigned int valor_ocr = ocr_min + ((unsigned int)(dist_cm - dist_min) * 71);

    *ponteiro_OCR1A = valor_ocr;
}

ISR(TIMER1_COMPA_vect) {
    *ponteiro_portc ^= 0x20; // Alterna PC5 (A5, bit 5 -> 0x20)
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
            formatar_distancia_ascii(distancia_cm);
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
    escolher_mensagem(); // Inicia enviando "PARADO\n"

    while(1){
        if (disparar_sensor == 1) {
            disparar_sensor = 0;
            inicia_trigger();
        }

        if (nova_medicao == 1) {
            cli();
            unsigned int tempo_lido = T_us;
            nova_medicao = 0;
            sei();

            distancia_cm = calculo_distancia(tempo_lido);
            atualiza_velocidade_led(distancia_cm);
        }

        if (novo_dado == 1){
            novo_dado = 0;
            processar_entrada();
        }

        maquina_estados();
    }

    return 0;
}
