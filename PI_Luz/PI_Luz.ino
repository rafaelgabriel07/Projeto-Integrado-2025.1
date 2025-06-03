#include "iluminacao.h"
#include <DS1302.h>  

#define UV_PIN 25
#define RELE_PIN 26
#define INTERVALO_LEITURA 0.1  // Minutos

// RTC - Novos pinos (sem conflito)
#define RST_PIN 13
#define DAT_PIN 14
#define CLK_PIN 27

DS1302 rtc(RST_PIN, DAT_PIN, CLK_PIN);

// Parâmetros da planta
int exposicaoAcumulada = 0;
int uvMin = 1;
int uvMax = 5;
int tempoExposicaoMin = 120;  // tempo minimo para cálculo
int tempoExposicaoMax = 240;

int fatorUVLampada = 3;

unsigned long tempo_ultima_leitura = 0;

void setup() {
  pinMode(UV_PIN, INPUT);
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, LOW);

  Serial.begin(9600);

  // Inicia o RTC
  rtc.init();

  // ⚠️ Ajuste manual da hora se necessário — execute apenas uma vez
  // rtc.setDate(DIA, MES, ANO); rtc.setTime(HORA, MIN, SEG);
  // rtc.setDate(2, 6, 2025); rtc.setTime(15, 45, 0);  // Exemplo

  Serial.println("Sistema iniciado com RTC DS1302.");
}

void loop() {
  unsigned long tempo_atual = millis();

  // get the current time
  Ds1302::DateTime now;
  rtc.getDateTime(&now);

  int hora = now.hour;
  int minuto = now.minute;

  int ano = now.year;
  int mes = now.month;
  int dia = now.day;

  //Realiza a leitura a cada INTERVALO_LEITURA minutos
  if (tempo_atual - tempo_ultima_leitura >= INTERVALO_LEITURA * 60000) {
    tempo_ultima_leitura = tempo_atual;

    Serial.print("Hora atual: ");
    Serial.println(hora + ":" + minuto);
    Serial.print("Data: ");
    Serial.println(dia + "/" + mes + "/" ano);

    if (hora >= 5 && hora < 18) {
      int indiceUV = uvReading(UV_PIN);
      float fatorUV = calculoFator(indiceUV);

      Serial.print("Índice UV: ");
      Serial.println(indiceUV);

      if (digitalRead(RELE_PIN)) {
        exposicaoAcumulada += INTERVALO_LEITURA * fatorUVLampada;
      } else {
        exposicaoAcumulada += INTERVALO_LEITURA * fatorUV;
      }

      if (fatorUV > uvMax) {
        Serial.println("Índice UV muito alto para esta planta, coloque-a em ambiente menos ensolarado!");
      } else if (fatorUV < uvMin) {
        Serial.println("Índice UV muito baixo, acionando relé...");
        if (!digitalRead(RELE_PIN)) digitalWrite(RELE_PIN, HIGH);
      } else {
        Serial.println("Índice UV nos parâmetros ideais.");
        digitalWrite(RELE_PIN, LOW);
      }
    }

    if (hora >= 18 && hora < 24) {
      if (exposicaoAcumulada < tempoExposicaoMin) {
        digitalWrite(RELE_PIN, HIGH);
        Serial.println("Ligando luz artificial para compensar falta de sol.");
        exposicaoAcumulada += INTERVALO_LEITURA * fatorUVLampada;
      } else {
        digitalWrite(RELE_PIN, LOW);
        Serial.println("Dose diária de luz foi atingida, luz artificial não necessária.");
      }
    }

    if (hora >= 0 && hora < 5) {
      digitalWrite(RELE_PIN, LOW);
      if (exposicaoAcumulada < tempoExposicaoMin) {
        Serial.println("Dose solar diária não atingida ontem!");
      }
    }

    if (hora == 5 && minuto == 0) {
      exposicaoAcumulada = 0;
      Serial.println("Novo dia! Resetando exposição acumulada.");
    }
  }
}
