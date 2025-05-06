#include <Arduino.h>
#include "iluminacao.h"
#include <WiFi.h>
#include <time.h>  // Para usar NTP no NodeMCU

#define SENSOR_UV 15
#define LUZ 27
#define INTERVALO_LEITURA 0.1  // Minutos
#define SENSOR_UMIDADE_1 4
#define SENSOR_UMIDADE_2 13
#define BOMBA_AGUA_1 26
#define BOMBA_AGUA_2 14


// Parâmetros da planta
int exposicaoAcumulada = 0;
int uvMin = 1;
int uvMax = 5;
int tempoExposicaoMin = 120;  // tempo minimo para cálculo
int tempoExposicaoMax = 240;
int fatorUVlampada = 3;
unsigned long tempo_ultima_leitura = 0;

// Wi-Fi
const char* ssid = "dlink-52EC";
const char* password = "gkhcp39210";


void setup(){

  pinMode(SENSOR_UMIDADE_1, INPUT);
  pinMode(BOMBA_AGUA_1, OUTPUT);
  pinMode(SENSOR_UMIDADE_2, INPUT);
  pinMode(BOMBA_AGUA_2, OUTPUT);
  pinMode(SENSOR_UV, INPUT);
  pinMode(LUZ, OUTPUT);

  digitalWrite(LUZ, HIGH);
  digitalWrite(BOMBA_AGUA_1,HIGH);
  digitalWrite(BOMBA_AGUA_2,HIGH);

  Serial.begin(9600);
  Serial.println("Iniciando teste\n");

  // Conecta no Wi-Fi
  //WiFi.begin(ssid, password);
  //Serial.print("Conectando ao WiFi...");
  //unsigned long start = millis();
  //while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
  //  delay(500);
  //  Serial.print(".");
  //}
  ////Após passar muito tempo tentando conectar, reinicia o ESP
  //if (WiFi.status() != WL_CONNECTED) ESP.restart();
//
  //Serial.println("Conectado!");
//
  //// Configura NTP
  //configTime(-3 * 3600, 0, "pool.ntp.org");  // GMT-3 para o Brasil (horário padrão)

}

bool bomba1Ligada = false;
bool bomba2Ligada = false;
bool luzLigada = false;
unsigned long tempoAtualBomba = millis();
unsigned long tempoAtualSensorUV = millis();

void loop(){

  // Fazer a Leitura dos sensores a cada 10s
  if (millis() - tempoAtualBomba >= 10000){
    //Atualizacao do tempo
    tempoAtualBomba = millis();

    //Leitura do sensor 1 para ver se é necessario ligar a bomba
    if (analogRead(SENSOR_UMIDADE_1) >= 1000){
      
      digitalWrite(BOMBA_AGUA_1, LOW);
      bomba1Ligada = true;

    }
    
    //Leitura do sensor 1 para ver se é necessario ligar a bomba
    if (analogRead(SENSOR_UMIDADE_2) >= 1000){
      
      digitalWrite(BOMBA_AGUA_2, LOW);
      bomba2Ligada = true;

    }
    
  }

  // Leitura do Sensor UV
  if ((millis() - tempoAtualSensorUV >= 13000) && !luzLigada){
    //Atualizacao do tempo
    tempoAtualSensorUV = millis();

    //Leitura do sensor 1 para ver se é necessario ligar a bomba
    if (uvReading(SENSOR_UV) <= 5){
      
      digitalWrite(LUZ, LOW);
      luzLigada = true;

    }    
  }

  // Desligando a bomba caso passe o tempo necessario
  if ((millis() - tempoAtualBomba >= 5000) && (bomba1Ligada || bomba2Ligada)){

    digitalWrite(BOMBA_AGUA_1, HIGH);
    bomba1Ligada = false;
    digitalWrite(BOMBA_AGUA_2, HIGH);
    bomba2Ligada = false;
    tempoAtualBomba = millis();

  }

  // Desligado a luz caso passe o tempo
  if ((millis() - tempoAtualSensorUV >= 5000) && luzLigada){

    digitalWrite(LUZ, HIGH);
    luzLigada = false;
    tempoAtualSensorUV = millis();

  }



  Serial.print("Umidade sensor 1: ");
  Serial.print(analogRead(SENSOR_UMIDADE_1));
  Serial.print(" | ");
  Serial.print("Umidade sensor 2: ");
  Serial.print(analogRead(SENSOR_UMIDADE_2));
  Serial.print(" | ");
  Serial.print("Sensor luz: ");
  Serial.print(uvReading(SENSOR_UV));
  Serial.print(" | ");
  Serial.print("Status bomba 1: ");
  Serial.print(bomba1Ligada ? "Ligada" : "Desligada");
  Serial.print(" | ");
  Serial.print("Status bomba 2: ");
  Serial.print(bomba2Ligada ? "Ligada" : "Desligada");
  Serial.print(" | ");
  Serial.print("Status luz: ");
  Serial.println(luzLigada ? "Ligada" : "Desligada");

}