#include <Arduino.h>
#include <RtcDS1302.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "iluminacao.h"
#include "controle.hpp"

// Wi-fi
const char* ssid = "dlink-52EC";
const char* password = "glhcp39210";

// Servidor (IP MUDA A DEPENDER DO COMPUTADOR)
const char* serverUrl = "http://SEU_IPPPP:8000/dados";

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
#define INTERVALO_GET 10 // Segundos
#define TEMPO_IRRIGACAO 10 // Segundos

// Parametros da planta
#define UMIDADE_MAXIMA_1 3100
#define UMIDADE_MINIMA_1 1600
#define UMIDADE_MAXIMA_2 2100
#define UMIDADE_MINIMA_2 650

// --- Variáveis para armazenar os dados da Planta 1 ---
String planta1_NomePopular = "N/A";
int planta1_UmidadeMin = 0;
int planta1_UmidadeMax = 0;
int planta1_UvMin = 0;
int planta1_UvMax = 0;

// --- Variáveis para armazenar os dados da Planta 2 ---
String planta2_NomePopular = "N/A";
int planta2_UmidadeMin = 0;
int planta2_UmidadeMax = 0;
int planta2_UvMin = 0;
int planta2_UvMax = 0;

// Contador para controlar qual conjunto de variáveis será atualizado na próxima requisição
// 0: Inicial (ou sem dados recebidos ainda)
// 1: Última requisição foi para a Planta 1
// 2: Última requisição foi para a Planta 2
// Usaremos isso para a lógica de sobrescrita
int lastUpdatedPlant = 0;

#define FATOR_UV_LAMPADA 3 // Fator de exposicao da lampada UV

// Configurações do seu hotspot do celular
const char* ssid = "Matheus";     // *** SUBSTITUA PELO NOME DO SEU HOTSPOT ***
const char* password = "12345678"; // *** SUBSTITUA PELA SENHA DO SEU HOTSPOT ***

// Endereço IP do seu PC na rede do hotspot do celular
// Você precisa descobrir esse IP (use ipconfig no Windows ou ifconfig/ip a no Linux/macOS)
const char* flaskServerIP = "192.168.254.105"; // *** SUBSTITUA PELO IP REAL DO SEU PC ***
const int flaskServerPort = 5000;          // A porta que o seu servidor Flask está rodando

// Variável para armazenar a informação recebida
String receivedPlantData = "Nenhum dado recebido";

ControleUmidade controleUmidadeVaso1(
  SENSOR_UMIDADE_1,
  BOMBA_AGUA_1,
  TEMPO_IRRIGACAO,
  INTERVALO_BOMBA,
  UMIDADE_MINIMA_1,
  UMIDADE_MAXIMA_1
);

ControleUmidade controleUmidadeVaso2(
  SENSOR_UMIDADE_2,
  BOMBA_AGUA_2,
  TEMPO_IRRIGACAO,
  INTERVALO_BOMBA,
  UMIDADE_MINIMA_2,
  UMIDADE_MAXIMA_2
);

ControleUV controleUVVaso1(
  SENSOR_UV,
  FONTE_UV_1,
  FATOR_UV_LAMPADA,
  INTERVALO_LUZ
);

ControleUV controleUVVaso2(
  SENSOR_UV,
  FONTE_UV_2,
  FATOR_UV_LAMPADA,
  INTERVALO_LUZ
);

ThreeWire myWire(RTC_DAT, RTC_CLK, RTC_RST); 
RtcDS1302<ThreeWire> Rtc(myWire);

// Variaveis para manipular os dados do site
HTTPClient http;
int httpResponseCode;

// Função auxiliar para processar e armazenar os dados de uma planta
void processAndStorePlantData(String payload, int plantNumber) {
  DynamicJsonDocument doc(512); // Ajuste o tamanho se seus dados JSON forem maiores
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("Falha ao parsear JSON para Planta "));
    Serial.print(plantNumber);
    Serial.print(F(": "));
    Serial.println(error.f_str());
    return;
  }

  const char* newNomePopular = doc["nomePopular"];
  JsonArray newUmidadeSoloArray = doc["umidadeSolo"];
  JsonArray newUvDiaArray = doc["uvDia"];

  bool changed = false; // Flag para verificar se alguma coisa mudou

  int newPlantNumber;

  if (plantNumber == 1) {
    // Comparar e atualizar Planta 1
    if (planta1_NomePopular != newNomePopular) {
      planta1_NomePopular = newNomePopular;
      changed = true;
    }
    if (planta1_UmidadeMin != newUmidadeSoloArray[0].as<int>()) {
      planta1_UmidadeMin = newUmidadeSoloArray[0].as<int>();
      changed = true;
    }
    if (planta1_UmidadeMax != newUmidadeSoloArray[1].as<int>()) {
      planta1_UmidadeMax = newUmidadeSoloArray[1].as<int>();
      changed = true;
    }
    if (planta1_UvMin != newUvDiaArray[0].as<int>()) {
      planta1_UvMin = newUvDiaArray[0].as<int>();
      changed = true;
    }
    if (planta1_UvMax != newUvDiaArray[1].as<int>()) {
      planta1_UvMax = newUvDiaArray[1].as<int>();
      changed = true;
    }

    if (changed) {
      Serial.println("--- Dados da Planta 1 ATUALIZADOS ---");
      Serial.printf("  Nome Popular: %s\n", planta1_NomePopular.c_str());
      Serial.printf("  Umidade Min: %d, Max: %d\n", planta1_UmidadeMin, planta1_UmidadeMax);
      Serial.printf("  UV Min: %d, Max: %d\n", planta1_UvMin, planta1_UvMax);
      newPlantNumber = 0;
    } else {
      Serial.println("--- Dados da Planta 1 INALTERADOS ---");
      newPlantNumber = plantNumber;
    }

  } else if (plantNumber == 2) {
    // Comparar e atualizar Planta 2
    if (planta2_NomePopular != newNomePopular) {
      planta2_NomePopular = newNomePopular;
      changed = true;
    }
    if (planta2_UmidadeMin != newUmidadeSoloArray[0].as<int>()) {
      planta2_UmidadeMin = newUmidadeSoloArray[0].as<int>();
      changed = true;
    }
    if (planta2_UmidadeMax != newUmidadeSoloArray[1].as<int>()) {
      planta2_UmidadeMax = newUmidadeSoloArray[1].as<int>();
      changed = true;
    }
    if (planta2_UvMin != newUvDiaArray[0].as<int>()) {
      planta2_UvMin = newUvDiaArray[0].as<int>();
      changed = true;
    }
    if (planta2_UvMax != newUvDiaArray[1].as<int>()) {
      planta2_UvMax = newUvDiaArray[1].as<int>();
      changed = true;
    }

    if (changed) {
      Serial.println("--- Dados da Planta 2 ATUALIZADOS ---");
      Serial.printf("  Nome Popular: %s\n", planta2_NomePopular.c_str());
      Serial.printf("  Umidade Min: %d, Max: %d\n", planta2_UmidadeMin, planta2_UmidadeMax);
      Serial.printf("  UV Min: %d, Max: %d\n", planta2_UvMin, planta2_UvMax);
      newPlantNumber = 1;
    } else {
      Serial.println("--- Dados da Planta 2 INALTERADOS ---");
      newPlantNumber = plantNumber;
    }
  }

  return newPlantNumber;
}

void setup(){
  
  Serial.begin(9600);
  delay(1000);

   // Configura o ESP32 para o modo Station (cliente Wi-Fi)
  WiFi.mode(WIFI_STA);

  // Conecta-se ao hotspot do seu celular
  Serial.printf("Tentando conectar ao hotspot: %s\n", ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { // Tenta por 10 segundos
    delay(250);
    Serial.print(".");
    attempts++;
    if (attempts % 10 == 0) { // A cada 10 tentativas, imprime o status atual
        Serial.printf("\nTentativa %d, Status: %d\n", attempts, WiFi.status());
    }
  }
  Serial.println(); // Nova linha

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Conectado ao Wi-Fi! IP local do ESP32: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Servidor Flask esperado em: http://%s:%d/\n", flaskServerIP, flaskServerPort);
  } else {
    Serial.println("Falha ao conectar ao Wi-Fi. Verifique SSID e senha do hotspot.");
    Serial.print("Status do WiFi: ");
    Serial.println(WiFi.status()); // Exibe o código de status da falha
    // Se não conectar, não há como buscar dados do Flask.
  }


  //Iniciando o RTC
  Rtc.Begin();

  //DESCOMENTAR CAS SEJA A PRIMEIRA VEZ USANDO O RTC
  //RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //Rtc.SetDateTime(compiled);

  Serial.println("Iniciando teste\n");

  unsigned long lastGet = 0;

}

void loop(){

  
  if (millis() - lastGet >= INTERVALO_GET){

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      // A lógica aqui é que cada vez que o loop roda, ele tenta pegar dados
      // e alterna qual conjunto de variáveis será atualizado.
      // Você pode querer disparar essas requisições de forma diferente,
      // talvez com base em um input externo ou um sensor.

      String targetPlantId;
      int currentPlantSetToUpdate;

      // Lógica para determinar qual conjunto de variáveis será atualizado
      if (lastUpdatedPlant == 0 || lastUpdatedPlant == 2) { // Se ainda não atualizou ou a última foi a 2
        targetPlantId = "planta1";
        currentPlantSetToUpdate = 1;
      } else { // Se a última foi a 1 (próxima será a 2)
        targetPlantId = "planta2";
        currentPlantSetToUpdate = 2;
      }
      
      // Constrói a URL para a rota /dados/<id_da_planta> do seu servidor Flask
      String serverPath = "http://" + String(flaskServerIP) + ":" + String(flaskServerPort) + "/dados/" + targetPlantId;

      Serial.printf("\nBuscando dados da planta: %s (para armazenar em Planta %d)\n", targetPlantId.c_str(), currentPlantSetToUpdate);
      Serial.println(serverPath);

      http.begin(serverPath);

      // Faz a requisição GET
      int httpResponseCode = http.GET();

      if (httpResponseCode == HTTP_CODE_OK) { // Se o código de resposta for 200 OK
        String payload = http.getString(); // Lê o corpo da resposta
        Serial.print("Payload recebido: ");
        Serial.println(payload);

        // Processa e armazena os dados no conjunto de variáveis correto
        lastUpdatedPlant = processAndStorePlantData(payload, currentPlantSetToUpdate);

      } else {
        Serial.printf("Erro na requisição HTTP para Planta %s: %s (código: %d)\n", targetPlantId.c_str(), http.errorToString(httpResponseCode).c_str(), httpResponseCode);
      }

      http.end(); // Fecha a conexão
      
      lastGet = millis();

    } else {
      Serial.println("ESP32 não conectado ao Wi-Fi. Não é possível buscar dados.");
      WiFi.begin(ssid, password); // Tenta reconectar
      delay(1000);
    }
  }

  // Você pode exibir os dados atuais de ambas as plantas aqui, se desejar
  Serial.println("\n--- Resumo dos Dados Atuais ---");
  Serial.printf("Planta 1 - Nome: %s, Umidade: [%d,%d], UV: [%d,%d]\n", 
                planta1_NomePopular.c_str(), planta1_UmidadeMin, planta1_UmidadeMax, 
                planta1_UvMin, planta1_UvMax);
  Serial.printf("Planta 2 - Nome: %s, Umidade: [%d,%d], UV: [%d,%d]\n", 
                planta2_NomePopular.c_str(), planta2_UmidadeMin, planta2_UmidadeMax, 
                planta2_UvMin, planta2_UvMax);
  Serial.printf("Próxima requisição atualizará Planta %d\n", (lastUpdatedPlant == 1) ? 2 : 1);

  RtcDateTime now = Rtc.GetDateTime();

  //Iniciando o controle de umidade e uv dos vasos
  controleUmidadeVaso1.set(planta1_UmidadeMin);
  controleUmidadeVaso2.set(planta2_UmidadeMin);
  controleUVVaso1.set(planta1_UvMin, planta1_UvMax);
  controleUVVaso2.set(planta2_UvMin, planta2_UvMax);

  //Inicia a verificacao dos parametros de cada vaso
  controleUmidadeVaso1.update();
  controleUmidadeVaso2.update();
  controleUVVaso1.update(now);
  controleUVVaso2.update(now);

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