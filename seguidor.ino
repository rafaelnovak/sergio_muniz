
//DEFINIÇÃO PINOS
//CONFERIR SE TÁ CERTO A DEFINIÇÃO DO SENSOR DIREITA E ESQUERDA (talvez eu tenha invertido no código)
#define OPTICO_DIR A1 //Sensor Óptico Reflexido Direita 
#define OPTICO_ESQ A0 //Sensor Óptico Reflexido Esquerda

//Motor Direita
#define MOTOR_DIR1 5
#define MOTOR_DIR2 6

//Motor Esquerda
#define MOTOR_ESQ1 10
#define MOTOR_ESQ2 11

//DEFINIÇÃO DAS VARIÁVEIS AUXILIARES
int leitura_dir, leitura_esq; //Variável da leitura do sensor
int velocidade_alta = 200, velocidade_baixa = 70;

void setup() {
  Serial.begin(9600);
  //pinMode(LED,OUTPUT);
  pinMode(OPTICO_DIR, INPUT);
  pinMode(OPTICO_ESQ, INPUT);
  pinMode(MOTOR_DIR1, OUTPUT);
  pinMode(MOTOR_DIR2, OUTPUT);
  pinMode(MOTOR_ESQ1, OUTPUT);
  pinMode(MOTOR_ESQ2, OUTPUT);


  //Inicia o código com os motores parados
  digitalWrite(MOTOR_DIR1, LOW);
  digitalWrite(MOTOR_DIR2, LOW);
  digitalWrite(MOTOR_ESQ1, LOW);
  digitalWrite(MOTOR_ESQ2, LOW);

  Serial.print("-------------INICIO!!!!!!---------------");
  Serial.print("\n");
}

void loop() {
  //Leitura inicial
  leitura_dir = analogRead(OPTICO_DIR);
  leitura_esq = analogRead(OPTICO_ESQ);

  if (leitura_esq >= 150 && leitura_dir >= 150) { //Leituras baixas = claro
    //Frente
    analogWrite(MOTOR_DIR1, velocidade_alta);
    digitalWrite(MOTOR_DIR2, LOW);
    analogWrite(MOTOR_ESQ1, velocidade_alta);
    digitalWrite(MOTOR_ESQ2, LOW);
  }

  else if (leitura_esq < 150 && leitura_dir >= 150) {
    //Curva à esquerda
    analogWrite(MOTOR_DIR1, velocidade_alta);
    digitalWrite(MOTOR_DIR2, LOW);
    analogWrite(MOTOR_ESQ1, velocidade_baixa);
    digitalWrite(MOTOR_ESQ2, LOW);
  }

  else if (leitura_esq >= 150 && leitura_dir < 150) {
    //Curva à direita
    analogWrite(MOTOR_DIR1, velocidade_baixa);
    digitalWrite(MOTOR_DIR2, LOW);
    analogWrite(MOTOR_ESQ1, velocidade_alta);
    digitalWrite(MOTOR_ESQ2, LOW);
  }

  else {
    //Carrinho chegou no ponto de parada!!!!!!!
    analogWrite(MOTOR_DIR1, 0);
    digitalWrite(MOTOR_DIR2, LOW);
    analogWrite(MOTOR_ESQ1, 0);
    digitalWrite(MOTOR_ESQ2, LOW);

    delay(2000);

    analogWrite(MOTOR_DIR1, velocidade_alta);
    digitalWrite(MOTOR_DIR2, LOW);
    analogWrite(MOTOR_ESQ1, velocidade_alta);
    digitalWrite(MOTOR_ESQ2, LOW);

    delay(100);

  }

  Serial.print("Direita ");
  Serial.print(analogRead(OPTICO_DIR));
  Serial.print("Esquerda ");
  Serial.print(analogRead(OPTICO_ESQ));
  Serial.print("\n");

}
