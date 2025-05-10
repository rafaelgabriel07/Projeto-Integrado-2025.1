#include <Arduino.h>
#include "iluminacao.h"
#include "controle.hpp"
#include <WiFi.h>

// Pinos dos sensores
#define SENSOR_UV 15
#define SENSOR_UMIDADE_1 4
#define SENSOR_UMIDADE_2 13

// Pinos dos atuadores
#define FONTE_UV_1 27
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
unsigned indiceMaximoUV = 10;

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

ControleUmidade controleUmidadeVaso2(
  SENSOR_UMIDADE_2,
  BOMBA_AGUA_2,
  TEMPO_IRRIGACAO,
  INTERVALO_BOMBA,
  umidadeMinimaPlanta
);

ControleUV controleUVVaso1(
  SENSOR_UV,
  FONTE_UV_1,
  indiceMinimoUV,
  indiceMaximoUV
);

void setup(){

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

  Serial.begin(9600);
  Serial.println("Iniciando teste\n");

}

void loop(){

  //Inicia a verificacao dos parametros de cada vaso
  controleUmidadeVaso1.update();
  controleUmidadeVaso2.update();
  controleUVVaso1.update();

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
  Serial.print("Status iluminacao: ");
  Serial.println(controleUVVaso1.luzLigada ? "Ligada" : "Desligada");

}