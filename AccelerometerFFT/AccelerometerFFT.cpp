//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////    Programa em C para cálculo FFT dos dados de um acelerômetro para detecção das frequências         ////
////      de vibração de um motor.                                                                        ////
////    Mantendo a ideia minimalista, o programa não possui entrada de dados do usuário, simplesmente     ////
////      acessa os arquivos com nome pré estabelecido.                                                   ////
////                                                                                                      ////
////    Este código pode ser adaptado para utilização de um microcontrolador, com leitura em tempo real   ////
////      e cálculo FFT em tempo real e processar as informações, enviar dados ou aviso via IOT.          ////
////                                                                                                      ////
////    Para manter simplicidade, não executa filtragem windowing, mas pode ser facilmente implementado   ////
////                                                                                                      ////
////    Desenvolvido em C++ via Visual Studio 2019                                                        ////
////                                                                                                      ////
////                                                                                   eng. Bruno Bicalho ////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <math.h>
#include <fftw3.h>
#include <vector>
#include <set>
#include <iomanip>
#include <complex>
#include <cmath>
#include <string>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void arquivo_LeituraNome(string arquivo, int* amostraEpoch, int* amostraTempo, string* modeloSensor) {
    stringstream nomeArquivoLeitura(arquivo);
    string epoch;
    string tempo;

    getline(nomeArquivoLeitura, epoch, '-');
    getline(nomeArquivoLeitura, tempo, '-');
    getline(nomeArquivoLeitura, *modeloSensor, '.');

    *amostraEpoch = stoi(epoch);
    *amostraTempo = stoi(tempo);
}

void arquivo_GravaCSV(string arquivo, vector<double> celula1, vector<double> celula2, vector<double> celula3, float celula4) {
    ofstream arquivoSaida;
    
    arquivoSaida.open(arquivo);
    for (uint16_t i = 0; i < (celula1.size()); i++)
        arquivoSaida << (float)celula1[i] << ',' << (float)celula2[i] << ',' << (float)celula3[i] << ',' << (float)i * (float)celula4 << endl;

    arquivoSaida.close();
}



void arquivo_LeituraCSV(string arquivo, vector<double>* celula1, vector<double>* celula2, vector<double>* celula3) {
    std::ifstream       streamEntrada(arquivo);

    std::string         linha;
    std::stringstream   streamLinha;
    std::string         celula;

    while (std::getline(streamEntrada, linha)) {
        stringstream ss;

        streamLinha.clear();
        streamLinha.str(linha);

        double temporario;

        //Leitura dos dados separados por ',' e convertendo para double
        std::getline(streamLinha, celula, ',');
        ss << celula;
        ss >> temporario;
        ss.str("");
        ss.clear();
        (*celula1).push_back(temporario);

        std::getline(streamLinha, celula, ',');
        ss << celula;
        ss >> temporario;
        ss.str("");
        ss.clear();
        (*celula2).push_back(temporario);

        std::getline(streamLinha, celula, ',');
        ss << celula;
        ss >> temporario;
        (*celula3).push_back(temporario);

    }

    //Apaga as informações dos eixos do sensor, ex: (x, y, z) dados não numéricos
    (*celula1).erase((*celula1).begin());
    (*celula2).erase((*celula2).begin());
    (*celula3).erase((*celula3).begin());
}

void executarFFT(vector<double>* entrada, vector<double>* saida) {
    //Prepara o vetor para ser utilizad com FFTW
    (*entrada).resize((*entrada).size());
    (*saida).resize((*entrada).size());


    fftw_plan p = fftw_plan_dft_r2c_1d((*entrada).size(), (*entrada).data(), reinterpret_cast<fftw_complex*>(((*saida).data())), FFTW_ESTIMATE);

    fftw_execute(p);
    fftw_destroy_plan(p);
    fftw_cleanup();
}

int main() {
    /////////// Inicializando as variáveis que receberão os valores das funções ///////////
    // Variáveis que receberão dados do nome do arquivo
    int epoch = 0;
    int tempoAmostras = 0;
    string modeloSensor;

    // Variáveis que receberão valor CSV
    vector<double> leituraX;
    vector<double> leituraY;
    vector<double> leituraZ;

    // Variáveis que receberão saída do cálculo FFT
    std::vector<double> fftX;
    std::vector<double> fftY;
    std::vector<double> fftZ;

    // Variáveis que receberão scaling
    vector<double> finalX;
    vector<double> finalY;
    vector<double> finalZ;


    /////////// Iniciando processo de leitura de dados do arquivo ///////////
    // Lendo dados do arquivo
    arquivo_LeituraNome("1602245833-2715-NAO7856.txt", &epoch, &tempoAmostras, &modeloSensor);
    arquivo_LeituraCSV("1602245833-2715-NAO7856.txt", &leituraX, &leituraY, &leituraZ);

    /////////// Iniciando processo de cálculo FFT e interpretação dos dados ///////////
    // Executando FFT para os três eixos
    executarFFT(&leituraX, &fftX);
    executarFFT(&leituraY, &fftY);
    executarFFT(&leituraZ, &fftZ);

    // Realizando scaling, ajustando amplitude
    for (uint16_t i = 0; i < fftX.size(); i++) {
        finalX.push_back(2 * std::abs(fftX[i]) / leituraX.size());
        finalY.push_back(2 * std::abs(fftY[i]) / leituraX.size());
        finalZ.push_back(2 * std::abs(fftZ[i]) / leituraX.size());
    }

    // Remove a primeira linha 0hz (DC)
    finalX.erase(finalX.begin());
    finalY.erase(finalY.begin());
    finalZ.erase(finalZ.begin());

    // Realiza o cálculo para descobrir passo da frequência por bin do FFT, utilizando o nome do arquivo e quantidade de amostras
    float frequencia = ((float)leituraX.size() / ((float)tempoAmostras / 1000)) / (float)leituraX.size() / 2;


    /////////// Exportando dados para arquivo ///////////
    // Exporta os dados ao arquivo
    arquivo_GravaCSV("output.txt", finalX, finalY, finalZ, frequencia);

    return 0;
}
