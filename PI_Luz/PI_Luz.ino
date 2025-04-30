#include "iluminacao.h"
#include <WiFi.h>
#include <time.h>  // Para usar NTP no NodeMCU

#define UV_PIN 25
#define RELE_PIN 26
#define INTERVALO_LEITURA 0.1  // Minutos

// Parâmetros da planta
int exposicaoAcumulada = 0;
int uvMin = 1;
int uvMax = 5;
int tempoExposicaoMin = 120;  // tempo minimo para cálculo
int tempoExposicaoMax = 240;

int fatorUVlampada = 3;

unsigned long tempo_ultima_leitura = 0;

// Wi-Fi
const char* ssid = "CEPEL-GUEST";
const char* password = "cepel51anos";

void setup() {

  //Definição dos pinos e setando o Relé pra zero
  pinMode(UV_PIN, INPUT);
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, LOW);

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

  // Configura NTP
  configTime(-3 * 3600, 0, "pool.ntp.org");  // GMT-3 para o Brasil (horário padrão)
}

void loop() {
  unsigned long tempo_atual = millis();

  //Obter a hora atual
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter o tempo");
    delay(1000);
    return;
  }

  int hora = timeinfo.tm_hour;
  int minuto = timeinfo.tm_min;

  //Realiza a leitura a cada um intervalo de leitura
  if (tempo_atual - tempo_ultima_leitura >= INTERVALO_LEITURA * 60000) {
    tempo_ultima_leitura = tempo_atual;

    if (hora >= 5 && hora < 18) {
      //Calculo do fator uv recebido no sensor
      int indiceUV = uvReading(UV_PIN);
      float fatorUV = calculoFator(indiceUV);

      Serial.print("Hora atual: ");
      Serial.print(hora);
      Serial.print(":");
      Serial.println(minuto);
      Serial.print("Indice UV: ");
      Serial.println(indiceUV);

      //Calculo da exposição em função do fatorUV
      //Se o relé estiver ligado, o uv utilizado na conta é o da lampada
      if (digitalRead(RELE_PIN)) {
        exposicaoAcumulada += INTERVALO_LEITURA * fatorUVLampada;
      } else {
        exposicaoAcumulada += INTERVALO_LEITURA * fatorUV;  // acumula exposição diária ponderada
      }

      //Verifica se o UV está dentro dos limites da planta
      //UV alto envia mensagem de perigo
      if (fatorUV > uvMax) {
        Serial.println("Indice UV muito alto para esta planta, coloque-a em ambiente menos ensolarado!");
        //UV baixo aciona o RELE
      } else if (fatorUV < uvMin) {
        Serial.println("Indice UV muito baixo, acionando relé...");
        if (!digitalRead(RELE_PIN)) digitalWrite(RELE_PIN, HIGH);
        //Em casos normais, mantem a lampada desligada
      } else {
        Serial.println("Indice UV nos parâmetros ideais.");
        digitalWrite(RELE_PIN, LOW);
      }
    }

    // Após as 18h — decide se liga a luz artificial caso não tenha atingido o tempo de exposição
    if (hora >= 18 && hora < 24) {
      if (exposicaoAcumulada < tempoExposicaoMin) {
        digitalWrite(RELE_PIN, HIGH);  // liga luz artificial
        Serial.println("Ligando luz artificial para compensar falta de sol.");
        exposicaoAcumulada += INTERVALO_LEITURA * fatorUVLampada;
      } else {
        digitalWrite(RELE_PIN, LOW);
        Serial.println("Dose diária de luz foi atingida, luz artificial não necessária.");
      }
    }

    // Entre 0h e 5h — espera até amanhecer
    if (hora >= 0 && hora < 5) {
      digitalWrite(RELE_PIN, LOW);  // garante que está desligado
      if (exposicaoAcumulada < tempoExposicaoMin) {
        Serial.println("Dose solar diária não atingida ontem!");
      }
    }

    // 🌅 Às 5h — zera a dose diária e recomeça a leitura
    if (hora == 5 && minuto == 0) {
      exposicaoAcumulada = 0;
      Serial.println("Novo dia! Resetando exposição acumulada.");
    }
  }
}
