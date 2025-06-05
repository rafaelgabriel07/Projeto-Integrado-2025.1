#pragma once

#include <Arduino.h>
#include <RtcDS1302.h>
#include "iluminacao.h"

class ControleUmidade{

    private:

    int _sensorUmidade;
    int _bombaAgua;
    unsigned int _tempoIrrigacao;
    int _tempoIntervaloLeitura;
    int _umidadeMinima;
    int _leituraMaxima;
    int _leituraMinima;
    
    public:
    
    //Variaveis auxiliares
    bool bombaLigada = false;
    bool intervaloLeitura = false;
    unsigned long tempoAtual = millis();
    int leituraConvertida;

    ControleUmidade(int sensorUmidade, int bombaAgua, unsigned int tempoIrrigacao, int tempoIntervaloLeitura, int umidadeMinima, int leituraMinima, int leituraMaxima):
        
        _sensorUmidade(sensorUmidade),
        _bombaAgua(bombaAgua),
        _tempoIrrigacao(tempoIrrigacao), //Em segundos
        _tempoIntervaloLeitura(tempoIntervaloLeitura), //Em segundos
        _umidadeMinima(umidadeMinima), //Na escala de 0 a 100
        _leituraMaxima(leituraMaxima),
        _leituraMinima(leituraMinima){}  

    //Faz a configuracao inicial dos sensores e atuadores
    void set(){

        //Definindo os pinos
        pinMode(_sensorUmidade, INPUT);
        pinMode(_bombaAgua, OUTPUT);

        //Definindo a saida da bomba para HIGH, pois ela liga em LOW
        digitalWrite(_bombaAgua, HIGH);

    }

    void update(){

        leituraConvertida = map(analogRead(_sensorUmidade), _leituraMaxima, _leituraMinima, 0, 100);

        if ((!bombaLigada) && (!intervaloLeitura) && leituraConvertida <= _umidadeMinima){
    
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
        
        int umidade = leituraConvertida;
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

    //Fator de exposicao da lampada UV
    int _fatorUVLampada;
    
    
    //Variaveis para adquirir a hora
    struct tm _infoTempo;
    const char* _ntpServer = "pool.ntp.org";
    const long _fusoHorario = -6*3600; //Fuso horario referente ao horario de Brasilia
    const int _horarioDeVerao = 0; //Indicando que nao estamos em horario de verao
    
    
    public:
    
    //Variaveis auxiliares
    bool luzLigada = false;
    int exposicaoAcumulada = 0;

    ControleUV(int sensorUV, int luzUV, int exposicaoMinima, int exposicaoMaxima, int fatorUVLampada):
        
        _sensorUV(sensorUV),
        _luzUV(luzUV),
        _fatorUVLampada(fatorUVLampada),
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

    void update(RtcDateTime dt, int intervaloLeitura){

        float fatorUV = 0.0;

        //Verifica se estamos dentro do horario de sol ainda (entre as 6h as 18h)
        if (dt.Hour() <= 17 && dt.Hour() >= 6){
            
            fatorUV = calculoFator(uvReading(_sensorUV));
            exposicaoAcumulada += intervaloLeitura * fatorUV;

        }

        //Caso esteja entre as 18h as 22h ele liga a luz uv se necessario
        else if (dt.Hour() >= 18 && dt.Hour() <= 21){

            if (exposicaoAcumulada < _exposicaoMinima){

                digitalWrite(_luzUV, LOW);
                luzLigada = true;
                fatorUV = calculoFator(uvReading(_sensorUV));
                exposicaoAcumulada += intervaloLeitura * fatorUV;

            }

            else if ((exposicaoAcumulada >= _exposicaoMaxima) && luzLigada){

                digitalWrite(_luzUV, HIGH);
                luzLigada = false;
                exposicaoAcumulada += intervaloLeitura * _fatorUVLampada;

            }

        }

        //Depois das 22h ele desliga a luz uv (se ainda estiver ligada) e reseta o contador para o dia seguinte as 6h
        else{

            digitalWrite(_luzUV, LOW);
            luzLigada = false;
            exposicaoAcumulada = 0;

        }

    }

};