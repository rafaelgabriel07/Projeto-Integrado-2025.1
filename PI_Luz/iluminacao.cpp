#include <Arduino.h>
#include "iluminacao.h"

int uvReading(int pinUV) {

  int numLeituras = 25;

  //delay(100); // DELAY PARA APROXIMAR AS MEDIDAS DO TEMPO DE RESPOSTA DO SENSOR DE 500 mS
  float leituraUV = 0;  // VARIÁVEL PARA ARMAZENAR A LEITURA DA PORTA ANALÓGICA
  for (int i = 0; i < numLeituras; i++) {
    //FIXME: Tensão de entrada do ESP e resolução 12 bits 
    int analog = analogRead(pinUV);
    leituraUV += analog * (3.3 / 4095) * 1000.;  // REALIZA A LEITURA MÉDIA DA PORTA ANALÓGICA // Conversão para tensão em mV
  }
  leituraUV /= numLeituras;
  

  int indiceUV = 0;
  if (leituraUV < 50) {
    indiceUV = 0;
  } else if (leituraUV >= 50 && leituraUV <= 226) {
    indiceUV = 1;
  } else if (leituraUV >= 227 && leituraUV <= 317) {
    indiceUV = 2;
  } else if (leituraUV >= 318 && leituraUV <= 407) {
    indiceUV = 3;
  } else if (leituraUV >= 408 && leituraUV <= 502) {
    indiceUV = 4;
  } else if (leituraUV >= 503 && leituraUV <= 605) {
    indiceUV = 5;
  } else if (leituraUV >= 606 && leituraUV <= 695) {
    indiceUV = 6;
  } else if (leituraUV >= 696 && leituraUV <= 794) {
    indiceUV = 7;
  } else if (leituraUV >= 795 && leituraUV <= 880) {
    indiceUV = 8;
  } else if (leituraUV >= 881 && leituraUV <= 975) {
    indiceUV = 9;
  } else if (leituraUV >= 976 && leituraUV <= 1078) {
    indiceUV = 10;
  } else{
    indiceUV = 11;
  }
  return indiceUV;
}

// Função que retorna o fator de intensidade do índice UV
float calculoFator(float indiceUV) {
  if (indiceUV < 2) return 0.2;
  else if (indiceUV < 5) return 0.4;
  else if (indiceUV < 7) return 0.6;
  else if (indiceUV < 9) return 0.8;
  else return 1.0;
}
