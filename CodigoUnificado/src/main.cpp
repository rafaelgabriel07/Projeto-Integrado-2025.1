#include <Arduino.h>
#include <RtcDS1302.h>
#include "iluminacao.h"
#include "controle.hpp"

// Pinos dos sensores
#define SENSOR_UV 27
#define SENSOR_UMIDADE_1 34
#define SENSOR_UMIDADE_2 33
#define RTC_CLK 19
#define RTC_DAT 18
#define RTC_RST 5

// Pinos dos atuadores
#define FONTE_UV_1 21
#define FONTE_UV_2 32
#define BOMBA_AGUA_1 26
#define BOMBA_AGUA_2 14

// Macros de tempo
#define INTERVALO_BOMBA 10 // Segundos
#define INTERVALO_LUZ 1 // Minutos
#define TEMPO_IRRIGACAO 10 // Segundos

// Parametros da planta
#define UMIDADE_MAXIMA_1 3100
#define UMIDADE_MINIMA_1 1600
#define UMIDADE_MAXIMA_2 2100
#define UMIDADE_MINIMA_2 650
unsigned umidadeMinimaPlanta = 50;
unsigned indiceMinimoUV = 5;
unsigned indiceMaximoUV = 10;

#define FATOR_UV_LAMPADA 3 // Fator de exposicao da lampada UV


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
  indiceMaximoUV,
  FATOR_UV_LAMPADA,
  INTERVALO_LUZ
);

ControleUV controleUVVaso2(
  SENSOR_UV,
  FONTE_UV_2,
  indiceMinimoUV,
  indiceMaximoUV,
  FATOR_UV_LAMPADA,
  INTERVALO_LUZ
);

ThreeWire myWire(RTC_DAT, RTC_CLK, RTC_RST); 
RtcDS1302<ThreeWire> Rtc(myWire);

void setup(){
  
  Serial.begin(9600);

  //Iniciando o RTC
  Rtc.Begin();

  //DESCOMENTAR CAS SEJA A PRIMEIRA VEZ USANDO O RTC
  //RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //Rtc.SetDateTime(compiled);

  //Iniciando o controle de umidade e uv dos vasos
  controleUmidadeVaso1.set();
  controleUmidadeVaso2.set();
  controleUVVaso1.set();
  controleUVVaso2.set();

  Serial.println("Iniciando teste\n");

}

void loop(){

  RtcDateTime now = Rtc.GetDateTime();

  //Inicia a verificacao dos parametros de cada vaso
  controleUmidadeVaso1.update();
  controleUmidadeVaso2.update();
  controleUVVaso1.update(now);
  controleUVVaso2.update(now);
/*
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
*/
}