#include <Arduino.h>

#define SENSOR_UMIDADE 15
#define BOMBA_AGUA 4

void setup(){

  pinMode(SENSOR_UMIDADE, INPUT);
  pinMode(BOMBA_AGUA, OUTPUT);
  Serial.begin(9600);
  Serial.println("Iniciando teste\n");

}

bool bombaLigada = false;
bool primeiroLoop = true;
unsigned long tempoAtual = millis();

void loop(){

  // Fazer a Leitura dos sensores a cada 10s
  if (millis() - tempoAtual >= 10000){
    //Atualizacao do tempo
    tempoAtual = millis();
    //Leitura do sensor para ver se é necessario ligar a bomba
    if (analogRead(SENSOR_UMIDADE) >= 1000){
      
      digitalWrite(BOMBA_AGUA, HIGH);
      bombaLigada = true;

    }
    
  }

  if ((millis() - tempoAtual >= 5000) && bombaLigada){

    digitalWrite(BOMBA_AGUA, LOW);
    bombaLigada = false;
    tempoAtual = millis();

  }

  Serial.print("Umidade: ");
  Serial.print(analogRead(SENSOR_UMIDADE));
  Serial.print(" | ");
  Serial.print("Status bomba: ");
  Serial.println(bombaLigada ? "Ligada" : "Desligada");

}