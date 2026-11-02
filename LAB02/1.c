/* Maria Letícia Gomes Braga dos Reis - 281318 e Ana Carolina Vieira de Araújo - 248734 */

#define F_CPU 16000000UL

int main(void){

    unsigned char *ponteiro_ddrb = (unsigned char *) 0x24;
    unsigned char *ponteiro_ddrb = (unsigned char *) 0x25;

    unsigned int i = 0;

    *ponteiro_ddrb = *ponteiro_ddrb | 0x20;

    while(1){

        *ponteiro_portb &= ~(0x20);
        i = 0;

        while(i<0xFFFF) i = i + 1;
            *ponteiro_ddrb = *ponteiro_ddrb | 0x20;
            i=0;
            while(i < 0xFFFF) i = i + 1;

    }

}