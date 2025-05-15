#include <Arduino.h>
#include "iluminacao.h"
#include "controle.hpp"
#include <WiFi.h>

// Pinos dos sensores
#define SENSOR_UV 27
#define SENSOR_UMIDADE_1 34
#define SENSOR_UMIDADE_2 33

// Pinos dos atuadores
#define FONTE_UV_1 21
#define FONTE_UV_2 32
#define BOMBA_AGUA_1 26
#define BOMBA_AGUA_2 14

// Macros de tempo
#define INTERVALO_BOMBA 10 // Segundos
#define INTERVALO_LUZ 10000 // Milissegundos
#define TEMPO_IRRIGACAO 10 // Segundos
#define TEMPO_ILUMINACAO 7000 // Millisegundos

// Parametros da planta
#define UMIDADE_MAXIMA_1 3100
#define UMIDADE_MINIMA_1 1600
#define UMIDADE_MAXIMA_2 2100
#define UMIDADE_MINIMA_2 650
unsigned umidadeMinimaPlanta = 50;
unsigned indiceMinimoUV = 5;
unsigned indiceMaximoUV = 10;

// Wi-Fi
const char* ssid = "dlink-52EC";
const char* password = "gkhcp39210";

ControleUmidade controleUmidadeVaso1(
  SENSOR_UMIDADE_1,
  BOMBA_AGUA_1,
  TEMPO_IRRIGACAO,
  INTERVALO_BOMBA,
  umidadeMinimaPlanta,
  UMIDADE_MINIMA_1,
  UMIDADE_MAXIMA_1
);

ControleUmidade controleUmidadeVaso2(
  SENSOR_UMIDADE_2,
  BOMBA_AGUA_2,
  TEMPO_IRRIGACAO,
  INTERVALO_BOMBA,
  umidadeMinimaPlanta,
  UMIDADE_MINIMA_2,
  UMIDADE_MAXIMA_2
);

ControleUV controleUVVaso1(
  SENSOR_UV,
  FONTE_UV_1,
  indiceMinimoUV,
  indiceMaximoUV
);

ControleUV controleUVVaso2(
  SENSOR_UV,
  FONTE_UV_2,
  indiceMinimoUV,
  indiceMaximoUV
);

void setup(){
  
  Serial.begin(9600);

  // Conecta no Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi...");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

  //Após passar muito tempo tentando conectar, reinicia o ESP
  if (WiFi.status() != WL_CONNECTED) ESP.restart();
  Serial.println("Conectado!");

  //Iniciando o controle de umidade e uv dos vasos
  controleUmidadeVaso1.set();
  controleUmidadeVaso2.set();
  controleUVVaso1.set();
  controleUVVaso2.set();

  Serial.println("Iniciando teste\n");

}

void loop(){

  //Inicia a verificacao dos parametros de cada vaso
  controleUmidadeVaso1.update();
  controleUmidadeVaso2.update();
  controleUVVaso1.update();
  controleUVVaso2.update();

  Serial.print("Umidade sensor 1: ");
  Serial.print(controleUmidadeVaso1.getUmidade());
  Serial.print(" | ");
  Serial.print("Status bomba 1: ");
  Serial.print(controleUmidadeVaso1.bombaLigada ? "Ligada" : "Desligada");
  Serial.print(" | ");
  Serial.print("Status intervalo 1: ");
  Serial.print(controleUmidadeVaso1.intervaloLeitura ? "Em intervalo" : "Lendo");
  Serial.print(" | ");
  Serial.print("Umidade sensor 2: ");
  Serial.print(controleUmidadeVaso2.getUmidade());
  Serial.print(" | ");
  Serial.print("Status bomba 2: ");
  Serial.print(controleUmidadeVaso2.bombaLigada ? "Ligada" : "Desligada");
  Serial.print(" | ");
  Serial.print("Status intervalo 2: ");
  Serial.print(controleUmidadeVaso2.intervaloLeitura ? "Em intervalo" : "Lendo");
  Serial.print(" | ");
  Serial.print("Exposicao acumulada vaso 1: ");
  Serial.print(controleUVVaso1.exposicaoAcumulada);
  Serial.print(" | ");
  Serial.print("Status iluminacao 1: ");
  Serial.print(controleUVVaso1.luzLigada ? "Ligada" : "Desligada");
  Serial.print(" | ");
  Serial.print("Exposicao acumulada vaso 2: ");
  Serial.print(controleUVVaso2.exposicaoAcumulada);
  Serial.print(" | ");
  Serial.print("Status iluminacao 2: ");
  Serial.println(controleUVVaso2.luzLigada ? "Ligada" : "Desligada");

}