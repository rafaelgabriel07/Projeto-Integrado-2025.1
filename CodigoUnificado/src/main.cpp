#include <Arduino.h>
#include "iluminacao.h"
#include "controle.hpp"
#include <WiFi.h>
#include <time.h>  // Para usar NTP no NodeMCU

// Pinos dos sensores
#define SENSOR_UV 15
#define SENSOR_UMIDADE_1 4
#define SENSOR_UMIDADE_2 13

// Pinos dos atuadores
#define LUZ 27
#define BOMBA_AGUA_1 26
#define BOMBA_AGUA_2 14

// Macros de tempo
#define INTERVALO_BOMBA 10 // Segundos
#define INTERVALO_LUZ 10000 // Milissegundos
#define TEMPO_IRRIGACAO 10 // Segundos
#define TEMPO_ILUMINACAO 7000 // Millisegundos

// Parametros da planta
unsigned umidadeMinimaPlanta = 1000;
unsigned indiceMinimoUV = 5;

// Wi-Fi
const char* ssid = "dlink-52EC";
const char* password = "gkhcp39210";

ControleUmidade controleUmidadeVaso1(
  SENSOR_UMIDADE_1,
  BOMBA_AGUA_1,
  TEMPO_IRRIGACAO,
  INTERVALO_BOMBA,
  1000
);


void setup(){

  controleUmidadeVaso1.set();

  pinMode(SENSOR_UV, INPUT);
  pinMode(SENSOR_UMIDADE_2, INPUT);
  
  pinMode(LUZ, OUTPUT);
  pinMode(BOMBA_AGUA_2, OUTPUT);

  // Colocando os pinos dos atuadores em HIGH pois os reles sao ativados em LOW
  digitalWrite(LUZ, HIGH);
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

// Variaveis auxiliares
bool bomba2Ligada = false;
bool luzLigada = false;
bool intervaloBomba2 = false;
bool intervaloIluminacao = false;
unsigned long tempoAtualSensorUV;

void loop(){

  controleUmidadeVaso1.update();

  // Mesma logica que a de cima porem referente a iluminacao
  if ((!luzLigada) && (!intervaloIluminacao) && uvReading(SENSOR_UV) >= indiceMinimoUV){
    
     
    digitalWrite(LUZ, LOW);
    luzLigada = true;
    tempoAtualSensorUV = millis();

  }

  if (luzLigada && (millis() - tempoAtualSensorUV >= TEMPO_ILUMINACAO)){

    
    digitalWrite(LUZ, HIGH);
    luzLigada = false;
    intervaloIluminacao = true;
    tempoAtualSensorUV = millis();
    
  }

  if (intervaloIluminacao && (millis() - tempoAtualSensorUV >= INTERVALO_BOMBA)){

    intervaloIluminacao = false;

  }

  Serial.print("Umidade sensor 1: ");
  Serial.print(controleUmidadeVaso1.getUmidade());
  Serial.print(" | ");
  Serial.print("Status bomba 1: ");
  Serial.print(controleUmidadeVaso1.bombaLigada ? "Ligada" : "Desligada");
  Serial.print(" | ");
  Serial.print("Status intervalo 1: ");
  Serial.print(controleUmidadeVaso1.intervaloLeitura ? "Em intervalo" : "Lendo");
  Serial.print(" | ");
  Serial.print("Sensor UV: ");
  Serial.print(uvReading(SENSOR_UV));
  Serial.print(" | ");
  Serial.print("Status iluminacao: ");
  Serial.print(luzLigada ? "Ligada" : "Desligada");
  Serial.print(" | ");
  Serial.print("Status intervalo iluminacao: ");
  Serial.println(intervaloIluminacao ? "Em intervalo" : "Lendo");

}