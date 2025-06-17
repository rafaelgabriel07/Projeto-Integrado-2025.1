#include <Arduino.h>
#include <RtcDS1302.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "iluminacao.h"
#include "controle.hpp"
#include <ArduinoJson.h>

// Wi-fi
// const char* ssid = "dlink-52EC";
// const char* password = "glhcp39210";

// Servidor (IP MUDA A DEPENDER DO COMPUTADOR)
// const char* serverUrl = "http://SEU_IPPPP:8000/dados";

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
#define INTERVALO_GET 40 // Segundos
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

String lastNomePopular = "N/A"; // Variável para armazenar o último nome popular recebido
int lastUmidadeMin = 0; // Variável para armazenar a última umidade mínima recebida
int lastUmidadeMax = 0; // Variável para armazenar a última umidade máxima recebida
int lastUvMin = 0; // Variável para armazenar o último UV mínimo recebido
int lastUvMax = 0; // Variável para armazenar o último UV máximo recebido

// Contador para controlar qual conjunto de variáveis será atualizado na próxima requisição
// 0: Inicial (ou sem dados recebidos ainda)
// 1: Última requisição foi para a Planta 1
// 2: Última requisição foi para a Planta 2
// Usaremos isso para a lógica de sobrescrita
int lastUpdatedPlant = 0;

#define FATOR_UV_LAMPADA 3 // Fator de exposicao da lampada UV

// Configurações do seu hotspot do celular
const char* ssid = "lucas";     // *** SUBSTITUA PELO NOME DO SEU HOTSPOT ***
const char* password = "12345678"; // *** SUBSTITUA PELA SENHA DO SEU HOTSPOT ***

// Endereço IP do seu PC na rede do hotspot do celular
// Você precisa descobrir esse IP (use ipconfig no Windows ou ifconfig/ip a no Linux/macOS)
const char* flaskServerIP = "192.168.90.60"; // *** SUBSTITUA PELO IP REAL DO SEU PC ***
const int flaskServerPort = 5000;          // A porta que o seu servidor Flask está rodando

// Variável para armazenar a informação recebida
String payload = "Nenhum dado recebido";

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

void setup(){
  
  Serial.begin(9600);
  delay(1000);

   // Configura o ESP32 para o modo Station (cliente Wi-Fi)
  WiFi.mode(WIFI_STA);

  // Conecta-se ao hotspot do seu celular
  Serial.println("Tentando conectar ao hotspot: "+ String(ssid));
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { // Tenta por 10 segundos
    delay(250);
    Serial.print(".");
    attempts++;
    if (attempts % 10 == 0) { // A cada 10 tentativas, imprime o status atual
        Serial.println("Tentativa" + String(attempts)+ "Status: " + WiFi.status());
    }
  }
  Serial.println(); // Nova linha

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado ao Wi-Fi! IP local do ESP32: "+ String(WiFi.localIP()));
    Serial.println("Servidor Flask esperado em: http://:/"+ String(flaskServerIP)+ ":"+ String(flaskServerPort));
  } else {
    Serial.println("Falha ao conectar ao Wi-Fi. Verifique SSID e senha do hotspot.");
    Serial.print("Status do WiFi: ");
    Serial.println(WiFi.status()); // Exibe o código de status da falha
    // Se não conectar, não há como buscar dados do Flask.
  }
  delay(10000);

  //Iniciando o RTC
  Rtc.Begin();

  //DESCOMENTAR CAS SEJA A PRIMEIRA VEZ USANDO O RTC
  //RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //Rtc.SetDateTime(compiled);

  Serial.println("Iniciando teste");

}

unsigned long lastGet = 0;

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
      String serverPath = "http://" + String(flaskServerIP) + ":" + String(flaskServerPort) + "/dados" ;

      Serial.println("Buscando dados da planta:" + String(targetPlantId) + " (para armazenar em Planta )"+ currentPlantSetToUpdate);
      Serial.println(serverPath);

      http.begin(serverPath);

      // Faz a requisição GET
      int httpResponseCode = http.GET();

      if (httpResponseCode == HTTP_CODE_OK) { // Se o código de resposta for 200 OK
        String payload = http.getString(); // Lê o corpo da resposta
        Serial.print("Payload recebido: ");
        Serial.println(payload);
        int firstDelimiter = payload.indexOf(';');
        int secondDelimiter = payload.indexOf(';', firstDelimiter + 1);

        if (firstDelimiter != -1 && secondDelimiter != -1) {
          String nomePopular = payload.substring(0, firstDelimiter);
          String umidadeSoloStr = payload.substring(firstDelimiter + 1, secondDelimiter);
          String uvDiaStr = payload.substring(secondDelimiter + 1);

          Serial.println("  Nome Popular: "+ String(nomePopular));
          Serial.println("  Umidade Solo: "+ String(umidadeSoloStr));
          Serial.println("  UV Dia: "+ String(uvDiaStr));

          // Agora você pode parsear umidadeSoloStr e uvDiaStr para extrair os números
          // e usar esses valores para controlar seus sensores/atuadores.
          // Exemplo de como extrair valores de "[40,60]"
          umidadeSoloStr.replace("[", "");
          umidadeSoloStr.replace("]", "");
          int umidadeMin = umidadeSoloStr.substring(0, umidadeSoloStr.indexOf(',')).toInt();
          int umidadeMax = umidadeSoloStr.substring(umidadeSoloStr.indexOf(',') + 1).toInt();
          Serial.println("  Umidade Min:" + String(umidadeMin)+ ", Max: " + String(umidadeMax));

          uvDiaStr.replace("[", "");
          uvDiaStr.replace("]", "");
          int uvMin = uvDiaStr.substring(0, uvDiaStr.indexOf(',')).toInt();
          int uvMax = uvDiaStr.substring(uvDiaStr.indexOf(',') + 1).toInt();
          Serial.println("  UV Min:" + String(umidadeMin)+ ", UV Max: " + String(umidadeMax));


          bool changed = false; // Flag para verificar se alguma coisa mudou

          if(nomePopular == lastNomePopular && 
              umidadeMin == lastUmidadeMin && 
              umidadeMax == lastUmidadeMax && 
              uvMin == lastUvMin && 
              uvMax == lastUvMax) {
            Serial.println("Dados recebidos são iguais aos últimos. Nenhuma atualização necessária.");
            changed = false; // Nenhuma mudança
          }else{
            changed = true; // Alguma coisa mudou
            lastNomePopular = nomePopular;
            lastUmidadeMin = umidadeMin;
            lastUmidadeMax = umidadeMax;
            lastUvMin = uvMin;
            lastUvMax = uvMax;
          }

          if (currentPlantSetToUpdate == 1 && changed) {
            // Comparar e atualizar Planta 1
            planta1_NomePopular = nomePopular;
            planta1_UmidadeMin = umidadeMin;
            planta1_UmidadeMax = umidadeMax;
            planta1_UvMin = uvMin;
            planta1_UvMax = uvMax;
  
            Serial.println("\n--- Dados da Planta 1 ATUALIZADOS ---");
            Serial.println("  Nome Popular: "+ String(planta1_NomePopular));
            Serial.println("  Umidade Min:" + String(planta1_UmidadeMin)+ " Umidade Max: " + String(planta1_UmidadeMax));
            Serial.println("  UV Min:" + String(planta1_UvMin) + " UV Max:" + String(planta1_UvMax));
            lastUpdatedPlant = 1;
          } else {
              Serial.println("\n--- Dados da Planta 1 INALTERADOS ---");
              
          }

          
          if (currentPlantSetToUpdate == 2 && changed) {
            planta2_NomePopular = nomePopular;
            planta2_UmidadeMin = umidadeMin;
            planta2_UmidadeMax = umidadeMax;
            planta2_UvMin = uvMin;
            planta2_UvMax = uvMax;

            Serial.println("\n--- Dados da Planta 2 ATUALIZADOS ---");
            Serial.println("  Nome Popular: "+ String(planta2_NomePopular));
            Serial.println("  Umidade Min:" + String(planta2_UmidadeMin)+ " Umidade Max: " + String(planta2_UmidadeMax));
            Serial.println("  UV Min:" + String(planta2_UvMin) + " UV Max:" + String(planta2_UvMax));
            lastUpdatedPlant = 2;
          } else {
            Serial.println("\n--- Dados da Planta 2 INALTERADOS ---");
            
          }
        }
      }
    } else {
      // Serial.println("Erro na requisição HTTP para Planta :  (código: )"+String(targetPlantId)+ http.errorToString(String(httpResponseCode)+ httpResponseCode);
      Serial.println(" ERRO HTTP");
      ESP.restart();
    }

    http.end(); // Fecha a conexão
      
    lastGet = millis();

  } else {
    Serial.println("ESP32 não conectado ao Wi-Fi. Não é possível buscar dados.");
    WiFi.begin(ssid, password); // Tenta reconectar
    delay(1000);
  }

  
  // Você pode exibir os dados atuais de ambas as plantas aqui, se desejar
  Serial.println();
  Serial.println("--- Resumo dos Dados Atuais ---");
  Serial.println("Planta 1 - Nome:"+String(planta1_NomePopular)+ " , Umidade: Min: "+ String(planta1_UmidadeMin)+ " Max: "+
                String(planta1_UmidadeMax)+ ", UV: Min: "+ planta1_UvMin + " Max: "+ planta1_UvMax);
  Serial.println("Planta 2 - Nome:"+String(planta2_NomePopular)+ " , Umidade: Min: "+ String(planta2_UmidadeMin)+ " Max: "+
                String(planta2_UmidadeMax)+ ", UV: Min: "+ planta2_UvMin + " Max: "+ planta2_UvMax);

  Serial.println("Próxima requisição atualizará Planta "+(lastUpdatedPlant == 1) ? 2 : 1);

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
  delay(7000);

}