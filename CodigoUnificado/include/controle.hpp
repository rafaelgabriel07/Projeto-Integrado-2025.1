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

    ControleUmidade(int sensorUmidade, int bombaAgua, unsigned int tempoIrrigacao, int tempoIntervaloLeitura, int leituraMinima, int leituraMaxima):
        
        _sensorUmidade(sensorUmidade),
        _bombaAgua(bombaAgua),
        _tempoIrrigacao(tempoIrrigacao), //Em segundos
        _tempoIntervaloLeitura(tempoIntervaloLeitura), //Em segundos
        _leituraMaxima(leituraMaxima),
        _leituraMinima(leituraMinima){}  

    //Faz a configuracao inicial dos sensores e atuadores
    void set(int umidadeMinima){

        //Definindo quantidade minima de umidade
        _umidadeMinima = umidadeMinima;

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
    int _intervaloDeLeitura;

    //Parametros da planta
    int _exposicaoMinima;
    int _exposicaoMaxima;

    //Fator de exposicao da lampada UV
    int _fatorUVLampada;
        
    public:
    
    //Variaveis auxiliares
    bool luzLigada = false;
    int exposicaoAcumulada = 0;
    unsigned long tempoAtual = millis();

    ControleUV(int sensorUV, int luzUV, int fatorUVLampada, int intervaloDeLeitura):
        
        _sensorUV(sensorUV),
        _luzUV(luzUV),
        _fatorUVLampada(fatorUVLampada),
        _intervaloDeLeitura(intervaloDeLeitura){}

    void set(int exposicaoMinima, int exposicaoMaxima){

        //Definindo a exposicao minima e maxima
        _exposicaoMaxima = exposicaoMaxima;
        _exposicaoMinima = exposicaoMinima;

        //Definindo os pinos dos sensores e atuadores
        pinMode(_sensorUV, INPUT);
        pinMode(_luzUV, OUTPUT);

        //Definindo o pino da luz para HIGH, pois a luz e ligada em LOW
        digitalWrite(_luzUV, HIGH);

    }

    void update(RtcDateTime dt){

        float fatorUV = 0.0;
        //Serial.print("Hora:");
        //Serial.print(dt.Hour());
        //Serial.print(" | Exposição acumulada: ");
        //Serial.println(exposicaoAcumulada);

        if (millis() - tempoAtual >= _intervaloDeLeitura*60000){
            tempoAtual = millis();

            //Verifica se estamos dentro do horario de sol ainda (entre as 6h as 18h)
            if (dt.Hour() <= 17 && dt.Hour() >= 6){

                fatorUV = calculoFator(uvReading(_sensorUV));
                exposicaoAcumulada += _intervaloDeLeitura * fatorUV;
                Serial.println("Indice UV: " + String(fatorUV));

            }

            //Caso esteja entre as 18h as 22h ele liga a luz uv se necessario
            else if (dt.Hour() >= 18 && dt.Hour() <= 21){

                if (exposicaoAcumulada < _exposicaoMinima){

                    digitalWrite(_luzUV, LOW);
                    luzLigada = true;
                    fatorUV = calculoFator(uvReading(_sensorUV));
                    exposicaoAcumulada += _intervaloDeLeitura * _fatorUVLampada;

                    // Verificar se ja chegou na exposicao acumulada minima
                    if (exposicaoAcumulada >= _exposicaoMinima){
                        digitalWrite(_luzUV, HIGH);
                        luzLigada = false;
                    }

                }

            }

            //Depois das 22h ele desliga a luz uv (se ainda estiver ligada) e reseta o contador para o dia seguinte as 6h
            else{

                digitalWrite(_luzUV, LOW);
                luzLigada = false;
                exposicaoAcumulada = 0;

            }
        }

    }

};