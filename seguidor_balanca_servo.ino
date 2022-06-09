//INCLUSÃO DE BIBLIOTECAS
#include <Servo.h> //Inclusão da biblioteca Servo.h
Servo servomotor; //Inicialização do objeto "servomotor" do tipo Servo

#include "HX711.h" //Inclusão da biblioteca do módulo da balança
HX711 scale; //Inclusão do objeto "scale" do tipo HX711 (célula de carga)

//DEFINIÇÃO PINOS
#define LED_VERM 12 //Led Vermelho
#define LED_AMAR 13//Led Amarelo
#define SERVO 9 //Servo
#define OPTICO_DIR A4 //Sensor Óptico Reflexivo Direita 
#define OPTICO_ESQ A0 //Sensor Óptico Reflexivo Esquerda
#define LOADCELL_DOUT_PIN A2 // Célula de carga: cabo branco
#define LOADCELL_SCK_PIN A3 //Célula de carga: cabo roxo

//Motor Direita
#define MOTOR_DIR1 5
#define MOTOR_DIR2 6

//Motor Esquerda
#define MOTOR_ESQ1 10
#define MOTOR_ESQ2 11

//DEFINIÇÃO DAS VARIÁVEIS
int leitura_dir, leitura_esq; //Variáveis da leitura do sensor óptico da direita e da esquerda
int pos; //variável posição do servo
double leitura_curta, leitura_celula; //variável leitura da celula de carga
double leitura_limiar = 3.1; //variável referencia de carga para inicio
int estado = 0; //variável para indicar o momento atual do ciclo
int limiar = 180; //variável para calibração do sensor óptico (limiar claro-escuro)
int pausa_motor = 40; //pausa no motor para controle artificial da velocidade em saída digital

void setup() {

  //Definição de pinos de entrada e pinos de saída
  Serial.begin(9600);
  pinMode(LED_VERM, OUTPUT);
  pinMode(LED_AMAR, OUTPUT);
  pinMode(OPTICO_DIR, INPUT);
  pinMode(OPTICO_ESQ, INPUT);
  pinMode(MOTOR_DIR1, OUTPUT);
  pinMode(MOTOR_DIR2, OUTPUT);
  pinMode(MOTOR_ESQ1, OUTPUT);
  pinMode(MOTOR_ESQ2, OUTPUT);

  digitalWrite(LED_VERM, HIGH); //LED Vermelho aceso para indicar SETUP (calibração da célula de carga)

  //Configuração inicial (tara) da célula de carga
  Serial.println("Inicializando célula de carga");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_scale(2280.f); // informando valor de calibração (escala) da célula de carga
  delay(100);
  scale.tare(); // reiniciar escala para zero (tara)

  Serial.println("After setting up the scale:");

  servomotor.attach(SERVO); //Utilização da função servomotor.attach da biblioteca para definir pino digital ao objeto "servomotor"

  delay(1000);

  //Inicia o código com os motores parados
  digitalWrite(MOTOR_DIR1, LOW);
  digitalWrite(MOTOR_DIR2, LOW);
  digitalWrite(MOTOR_ESQ1, LOW);
  digitalWrite(MOTOR_ESQ2, LOW);


  Serial.print("-------------INICIO!!!!!!---------------");
  Serial.print("\n");
  delay(1000);
}

void loop() {
  //ESTADO 0 - aguardando carga suficiente
  while (estado == 0) {
    digitalWrite(LED_VERM, LOW);
    digitalWrite(LED_AMAR, HIGH); //Combinação de LEDS utilizada para indicar espera de carga suficiente para iniciar ciclo de descarga

    //Leitura constante da célula de carga
    Serial.print("leitura média de carga:\t");
    leitura_celula = scale.get_units(30) * (-1);
    Serial.println(leitura_celula, 1);
    scale.power_down(); // Colocar a balança em modo "suspenso" (não é necessário)
    delay(10);
    scale.power_up();

    //AFERIR SE ATINGIU A CARGA SUFICIENTE PARA INICIAR CICLO DE DESCARGA
    if (leitura_celula > leitura_limiar) {
      estado = 1; //Mudança do estado 0 para o 1
      digitalWrite(LED_VERM, HIGH);
      digitalWrite(LED_AMAR, HIGH); //Combinação de LEDs utilizada para indicar momentaneamente que o AGV atingiu o peso para descarga
      Serial.println("CARGA LIMIAR ATINGIDA");
      Serial.println("INICIANDO CICLO DE DESCARGA");
      delay(1000);

      //Para garantir que o carrinho saia da marcação de parada e comece a ler novamente a linha
      digitalWrite(MOTOR_DIR1, HIGH);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, HIGH);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay (3 * pausa_motor);
    }
  }

  //ESTADO 1 - Movimentação até parada de descarga
  while (estado == 1) {
    digitalWrite(LED_VERM, LOW);
    digitalWrite(LED_AMAR, LOW); //Combinação de LEDs utilizada para indicar que o AGV está trafegando até o ponto de descarga

    leitura_dir = analogRead(OPTICO_DIR);
    leitura_esq = analogRead(OPTICO_ESQ); //Informações obtidas pelos sensores ópticos (claro-escuro)

    //Print de informações dos sensores ópticos
    Serial.println(analogRead(OPTICO_DIR));
    Serial.println(analogRead(OPTICO_ESQ));
    Serial.print("\n");

    //CÓDIGO SEGUIDOR DE LINHA
    if (leitura_esq >= limiar && leitura_dir >= limiar) {
      //explicação lógica: se leitura do seguidor de linha da esquerda e da direita forem maior que o limiar, significa que os sensores estão posicionados sobre a região preta, e não sobre a linha branca, ou seja, o AGV deve continuar em linhar reta, para isso os dois motores são ativados simultaneamente
      //Frente
      Serial.println("ESTADO 1 => FRENTE");
      digitalWrite(MOTOR_DIR1, HIGH);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, HIGH);
      digitalWrite(MOTOR_ESQ2, LOW);

      delay(pausa_motor);
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay(pausa_motor);
      //como os motores tiveram de ser controlados por saída digital (limitação quanto as saídas PWM), foram inseridos delays para que
      //o AGV tenha sua velocidade média limitada
    }
    else if (leitura_esq < limiar && leitura_dir >= limiar) {
      //explicação lógica: se leitura do seguidor de linha da esquerda for inferior ao limiar (claro)e da direita for maior que o limiar (escuro), significa que o sensor esquerdo está posicionado sobre a região branca, e o sensor direito sobre a região preta da pista, ou seja, para continuar a seguir o trajeto demarcado com linha branca, o AGV deve realizar um ajuste para acompanhar a curva para esquerda da linha. Esse ajuste é representado pelo desligamento momentâneo do motor da esquerda, até que o sensor verifique uma combinação diferente dos sensores ópticos.
      //Curva à esquerda
      Serial.println("ESTADO 1 => CURVA ESQUERDA");
      digitalWrite(MOTOR_DIR1, HIGH);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);

      delay(pausa_motor);
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay(pausa_motor);

    }
    else if (leitura_esq >= limiar && leitura_dir < limiar) {
      //explicação lógica: mesmo procedimento que para a curva a esquerda, com atualização para desligamento temporário do motor da direita ao registrar claro no sensor óptico da direita.
      //Curva à direita
      Serial.println("ESTADO 1 => CURVA DIREITA");
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, HIGH);
      digitalWrite(MOTOR_ESQ2, LOW);

      delay(pausa_motor);
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay(pausa_motor);

    }
    else {
      //explicação lógica: dado o escopo de possibilidades de leitura para os sensores ópticos: ambos escuro, esquerda claro e direita escuro, esquerda escuro e direita claro, e ambos claro, o único não utilizado para definir uma ação é "ambos claro", sendo assim, programa-se que, ao ler ambos os sensores ópticos claro, o AGV atingiu o ponto objetivo, e, assim, deve trocar o seu estado, para realizar a próxima tarefa.
      //Carrinho chegou no ponto de parada.
      Serial.println("ESTADO 1 => PARADO");
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      estado = 2; //Mudança do estado 1 para o estado 2
      delay(1000);
    }
  }

  //ESTADO 2 - Realizando descarga
  while (estado == 2) {
    digitalWrite(LED_VERM, LOW);
    digitalWrite(LED_AMAR, HIGH); //Apenas LED amarelo ativo, para indicar descarga em andamento

    //Movimentação do servo de 90° a 0° para tombar o conteúdo da caçamba.
    for (pos = 90; pos >= 0; pos -= 1) {
      servomotor.write(pos); //comando para servo atualizar sua posição (passo)
      delay(50); //tempo de espera em cada ciclo, para evitar movimentos bruscos do servo, serve como limitador de velocidade.
    }

    delay (1500); //Mantém a caçamba de máximo tombamento por 1.5 segundos.

    //Realiza o movimento de retorno da caçamba.
    for (pos = 0; pos <= 90; pos += 1) {
      servomotor.write(pos);
      delay(50);
    }

    //Após tombamento e retorno, o estado é alterado, pois o descargue foi concluído.
    estado = 3; //Mudança do estado 2 para o 3
    Serial.println("DESCARGA COMPLETA");
    Serial.println("RETORNANDO PARA MARCAÇÃO DE CARGA");
    scale.tare(); // após a descarga, zera (tara) a balança, com finalidade de evitar leituras de cargas inexistentes ou leitura de forças ocasionadas pelo impacto da caçamba. Obs.: apesar de não ser ideal zerar a célula de carga, levando em consideração que, na prática, podem haver resquícios materiais na caçamba, foi a solução encontrada para contornar a sensiblidade excessiva do sensor e medições residuais inexistentes.

    //Para garantir que o carrinho saia da marcação de parada e comece a ler novamente a linha:
    digitalWrite(MOTOR_DIR1, HIGH);
    digitalWrite(MOTOR_DIR2, LOW);
    digitalWrite(MOTOR_ESQ1, HIGH);
    digitalWrite(MOTOR_ESQ2, LOW);
    delay (3 * pausa_motor);

  }
  //ESTADO 3 - Movimentação de retorno para região de carga.
  while (estado == 3) {
    //A primeira parte desse estado é semelhante ao início while do estado 1, portanto, não serão comentadas novamente.
    digitalWrite(LED_VERM, HIGH);
    digitalWrite(LED_AMAR, LOW);
    //Leitura inicial
    leitura_dir = analogRead(OPTICO_DIR);
    leitura_esq = analogRead(OPTICO_ESQ);

    //Informações dos sensores ópticos
    Serial.println(analogRead(OPTICO_DIR));
    Serial.println(analogRead(OPTICO_ESQ));
    Serial.print("\n");

    //Controle dos motores DC independentes
    if (leitura_esq >= limiar && leitura_dir >= limiar) {
      Serial.println("ESTADO 3 => FRENTE");
      //Frente
      digitalWrite(MOTOR_DIR1, HIGH);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, HIGH);
      digitalWrite(MOTOR_ESQ2, LOW);

      delay(pausa_motor);
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay(pausa_motor);

    }

    else if (leitura_esq < limiar && leitura_dir >= limiar) {
      Serial.println("ESTADO 3 => CURVA ESQUERDA");
      //Curva à esquerda
      digitalWrite(MOTOR_DIR1, HIGH);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);

      delay(pausa_motor);
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay(pausa_motor);

    }

    else if (leitura_esq >= limiar && leitura_dir < limiar) {
      Serial.println("ESTADO 3 => CURVA DIREITA");
      //Curva à direita
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, HIGH);
      digitalWrite(MOTOR_ESQ2, LOW);

      delay(pausa_motor);
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay(pausa_motor);
    }

    else { //MARCAÇÃO BRANCA AMBOS OS LADOS NA PISTA
      //Nesse estado, ao atingir a marcação de fim de trajeto, o AGV deve parar a movimentação e retornar para o estado 0, onde aguarda carga suficiente para iniciar um novo ciclo.

      //Para garantir que o carrinho saia da marcação de parada e comece a ler novamente a linha
      digitalWrite(MOTOR_DIR1, HIGH);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, HIGH);
      digitalWrite(MOTOR_ESQ2, LOW);
      delay (3 * pausa_motor);

      Serial.println("ESTADO 3 => PARADO");
      //Carrinho chegou no ponto de parada!
      digitalWrite(MOTOR_DIR1, LOW);
      digitalWrite(MOTOR_DIR2, LOW);
      digitalWrite(MOTOR_ESQ1, LOW);
      digitalWrite(MOTOR_ESQ2, LOW);
      estado = 0; //Mudança de estado 3 para 0
      Serial.println("PONTO DE CARGA ENCONTRADO");
      Serial.println("AGUARDANDO CARGA SUFICIENTE");
      delay(1000);
      scale.tare();
    }
  }
}
