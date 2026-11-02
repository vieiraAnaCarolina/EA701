PROBLEMA 1: UART E COMANDOS
-microcontrolador recebe caractere e atualiza o estado, além de disparar o envio periódico da 
mensagem de confirmação a cada 1 segundo.
-provavelmente criar uma máquina de estados

PROBLEMA 2: MOTOR + PMW
-implementar tabela de direções A1-A4 
-configurar timer 2 para aceitar comandos de velocidade comandado pelo dutycycle

PROBLEMA 3: SENSOR + SEGURANÇA
-capturar eco no pino 2 com interrupção
-calcular distância e aplicar lógica de segurança

PROBLEMA 4: LED VARIÁVEL A5
-modular frequência de intermitência do led com base na última distância lida

Máquina de estados:
-movimento do carrinho
-implementação sensor ultrassônico
