#pragma once

#include <Arduino.h>

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