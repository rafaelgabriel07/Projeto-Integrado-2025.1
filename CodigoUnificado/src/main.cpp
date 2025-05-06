#include <Arduino.h>
#include "iluminacao.h"
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
#define INTERVALO_BOMBA 10000 // Milissegundos
#define INTERVALO_LUZ 10000 // Milissegundos
#define TEMPO_IRRIGACAO 10000 // Milissegundos

// Parametros da planta
unsigned umidadeMinimaPlanta = 1000;
unsigned indiceMinimoUV = 5;

// Wi-Fi
const char* ssid = "dlink-52EC";
const char* password = "gkhcp39210";


void setup(){

  pinMode(SENSOR_UV, INPUT);
  pinMode(SENSOR_UMIDADE_1, INPUT);
  pinMode(SENSOR_UMIDADE_2, INPUT);
  
  pinMode(LUZ, OUTPUT);
  pinMode(BOMBA_AGUA_1, OUTPUT);
  pinMode(BOMBA_AGUA_2, OUTPUT);

  // Colocando os pinos dos atuadores em HIGH pois os reles sao ativados em LOW
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

// Variaveis auxiliares
bool bomba1Ligada = false;
bool bomba2Ligada = false;
bool luzLigada = false;
bool intervaloBomba1 = false;
bool intervaloBomba2 = false;
bool intervaloIluminacao = false;
unsigned long tempoAtualBomba1;


void loop(){

  //Faz a leitura somente se o intervalo pos bomba ligada tenha passado
  if (!intervaloBomba1 && analogRead(SENSOR_UMIDADE_1) <= umidadeMinimaPlanta){
    
    // Ativamos a bomba caso a umidade esteja menor que a umidade minima  
    digitalWrite(BOMBA_AGUA_1, LOW);
    intervaloBomba1 = true;
    bomba1Ligada = true;
    tempoAtualBomba1 = millis();

  }

  if ((millis() - tempoAtualBomba1 >= TEMPO_IRRIGACAO)){

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