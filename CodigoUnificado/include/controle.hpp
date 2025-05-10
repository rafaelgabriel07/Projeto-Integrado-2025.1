#pragma once

#include <Arduino.h>
#include <time.h>
#include "iluminacao.h"

class ControleUmidade{

    private:

    int _sensorUmidade;
    int _bombaAgua;
    unsigned int _tempoIrrigacao;
    int _tempoIntervaloLeitura;
    int _umidadeMinima;
    
    public:
    
    //Variaveis auxiliares
    bool bombaLigada = false;
    bool intervaloLeitura = false;
    unsigned long tempoAtual = millis();

    ControleUmidade(int sensorUmidade, int bombaAgua, unsigned int tempoIrrigacao, int tempoIntervaloLeitura, int umidadeMinima):
        
        _sensorUmidade(sensorUmidade),
        _bombaAgua(bombaAgua),
        _tempoIrrigacao(tempoIrrigacao), //Em segundos
        _tempoIntervaloLeitura(tempoIntervaloLeitura), //Em segundos
        _umidadeMinima(umidadeMinima){} //Na escala de 0 a 3000 (fazer um map depois) 

    //Faz a configuracao inicial dos sensores e atuadores
    void set(){

        //Definindo os pinos
        pinMode(_sensorUmidade, INPUT);
        pinMode(_bombaAgua, OUTPUT);

        //Definindo a saida da bomba para HIGH, pois ela liga em LOW
        digitalWrite(_bombaAgua, HIGH);

    }

    void update(){

        if ((!bombaLigada) && (intervaloLeitura) && analogRead(_sensorUmidade) >= _umidadeMinima){
    
            // Ativamos a bomba caso a umidade esteja menor que a umidade minima  
            digitalWrite(_bombaAgua, LOW);
            bombaLigada = true;
            tempoAtual = millis();
        
        }
        
        if (bombaLigada && (millis() - tempoAtual >= _tempoIrrigacao*1000)){
        
            // Desliga a bomba e inicia a contagem no intervalo de medicao
            digitalWrite(_bombaAgua, HIGH);
            bombaLigada = false;
            intervaloLeitura = true;
            tempoAtual = millis();
            
        }
        
        if (intervaloLeitura && (millis() - tempoAtual >= _tempoIntervaloLeitura*1000)){
        
            intervaloLeitura = false;
        
        }

    }

    int getUmidade(){
        
        int umidade = analogRead(_sensorUmidade);
        return umidade;
        
    }

};

class ControleUV{

    private:
    
    //Variaveis referentes ao sensor e atuador
    int _sensorUV;
    int _luzUV;

    //Parametros da planta
    int _exposicaoMinima;
    int _exposicaoMaxima;
    int _exposicaoAcumulada = 0;

    //Variaveis para adquirir a hora
    struct tm _infoTempo;
    const char* _ntpServer = "pool.ntp.org";
    const long _fusoHorario = -3*3600; //Fuso horario referente ao horario de Brasilia
    const int _horarioDeVerao = 0; //Indicando que nao estamos em horario de verao

    
    public:

    //Variaveis auxiliares
    bool luzLigada = false;

    ControleUV(int sensorUV, int luzUV, int exposicaoMinima, int exposicaoMaxima):
        
        _sensorUV(sensorUV),
        _luzUV(luzUV),
        _exposicaoMinima(exposicaoMinima),
        _exposicaoMaxima(exposicaoMaxima){}

    void set(){

        //Definindo os pinos dos sensores e atuadores
        pinMode(_sensorUV, INPUT);
        pinMode(_luzUV, OUTPUT);

        //Definindo o pino da luz para HIGH, pois a luz e ligada em LOW
        digitalWrite(_luzUV, HIGH);

        //Configuracao para adquirir o tempo
        configTime(_fusoHorario, _horarioDeVerao, _ntpServer);

    }

    void update(){

        //Pegando a hora do dia
        getLocalTime(&_infoTempo);

        //Verifica se estamos dentro do horario de sol ainda (entre as 6h as 18h)
        if (_infoTempo.tm_hour <= 17 && _infoTempo.tm_hour >= 6){
            
            float fatorUV = calculoFator(uvReading(_sensorUV));
            _exposicaoAcumulada += fatorUV;

        }

        //Caso esteja fora da hora do sol, atraves da exposicao acumulada vemos se precisamos ou nao ligar a luz
        else{

            if (_exposicaoAcumulada > _exposicaoMinima){

                digitalWrite(_luzUV, LOW);
                luzLigada = true;

            }

            else if (_exposicaoAcumulada >= _exposicaoMaxima && luzLigada){

                digitalWrite(_luzUV, HIGH);
                luzLigada = false;

            }

        }

    }

};