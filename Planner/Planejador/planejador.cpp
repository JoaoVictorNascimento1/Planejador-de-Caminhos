#include <fstream>
#include <iostream>  // Entrada e saída de dados
#include <string>    // Manipulação de strings
#include <list>      // Contêiner para Aberto e Fechado
#include <vector>    // Contêiner para dados auxiliares, se necessário
#include <cmath>     // Funções matemáticas (haversine, etc.)
#include <algorithm> // Funções de busca e ordenação
#include <utility>   // Manipulação de pares (pair)

#include "planejador.h"

using namespace std;

/* *************************
   * CLASSE IDPONTO        *
   ************************* */

/// Atribuicao de string
void IDPonto::set(string&& S)
{
  t=move(S);
  if (!valid()) t.clear();
}

/* *************************
   * CLASSE IDROTA         *
   ************************* */

/// Atribuicao de string
void IDRota::set(string&& S)
{
  t=move(S);
  if (!valid()) t.clear();
}

/* *************************
   * CLASSE PONTO          *
   ************************* */

/// Distancia entre 2 pontos (formula de haversine)
double haversine(const Ponto& P1, const Ponto& P2)
{
  // Tratar logo pontos identicos
  if (P1.id == P2.id) return 0.0;

  static const double MY_PI = 3.14159265358979323846;
  static const double R_EARTH = 6371.0;
  // Conversao para radianos
  double lat1 = MY_PI*P1.latitude/180.0;
  double lat2 = MY_PI*P2.latitude/180.0;
  double lon1 = MY_PI*P1.longitude/180.0;
  double lon2 = MY_PI*P2.longitude/180.0;

  double cosseno = sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1-lon2);
  // Para evitar eventuais erros na funcao acos por imprecisao numerica
  // nas operacoes com double: acos(1.0000001) eh NAN
  if ( cosseno > 1.0 ) cosseno = 1.0;
  if ( cosseno < -1.0 ) cosseno = -1.0;
  // Distancia entre os pontos
  return R_EARTH*acos(cosseno);
}

/* *************************
   * CLASSE PLANEJADOR     *
   ************************* */

/// Torna o mapa vazio
void Planejador::clear()
{
  pontos.clear();
  rotas.clear();
}

/// Retorna um Ponto do mapa, passando a id como parametro.
/// Se a id for inexistente, retorna um Ponto vazio.
Ponto Planejador::getPonto(const IDPonto& Id) const {
    // Itera pela lista de pontos procurando o que possui o ID correspondente
    for (const auto& ponto : pontos) {
        if (ponto.id == Id) {
            return ponto; // Retorna o ponto encontrado
        }
    }
    // Caso nenhum ponto seja encontrado, retorna um ponto vazio
    return Ponto();
}

/// Retorna um Rota do mapa, passando a id como parametro.
/// Se a id for inexistente, retorna um Rota vazio.
Rota Planejador::getRota(const IDRota& Id) const {
    // Itera pela lista de rotas procurando a que possui o ID correspondente
    for (const auto& rota : rotas) {
        if (rota.id == Id) {
            return rota; // Retorna a rota encontrada
        }
    }
    // Caso nenhuma rota seja encontrada, retorna uma rota vazia
    return Rota();
}

/// Imprime os pontos do mapa no console
void Planejador::imprimirPontos() const
{
  for (const auto& P : pontos)
  {
    cout << P.id << '\t' << P.nome
         << " (" <<P.latitude << ',' << P.longitude << ")\n";
  }
}

/// Imprime as rotas do mapa no console
void Planejador::imprimirRotas() const
{
  for (const auto& R : rotas)
  {
    cout << R.id << '\t' << R.nome << '\t' << R.comprimento << "km"
         << " [" << R.extremidade[0] << ',' << R.extremidade[1] << "]\n";
  }
}

/// Leh um mapa dos arquivos arq_pontos e arq_rotas.
/// Caso nao consiga ler dos arquivos, deixa o mapa inalterado e retorna false.
/// Retorna true em caso de leitura bem sucedida
bool Planejador::ler(const std::string& arq_pontos,
                     const std::string& arq_rotas)
{
  // Listas temporarias para armazenamento dos dados lidos
  list<Ponto> listP;
  list<Rota> listR;
  // Variaveis auxiliares para buscas nas listas
  list<Ponto>::iterator itr_ponto;
  list<Rota>::iterator itr_rota;
  // Variaveis auxiliares para leitura de dados
  Ponto P;
  Rota R;
  string prov;

  // Leh os pontos do arquivo
  try
  {
    // Abre o arquivo de pontos
    ifstream arq(arq_pontos);
    if (!arq.is_open()) throw 1;

    // Leh o cabecalho
    getline(arq,prov);
    if (arq.fail() ||
        prov != "ID;Nome;Latitude;Longitude") throw 2;

    // Leh os pontos
    do
    {
      // Leh a ID
      getline(arq,prov,';');
      if (arq.fail()) throw 3;
      P.id.set(move(prov));
      if (!P.valid()) throw 4;

      // Leh o nome
      getline(arq,prov,';');
      if (arq.fail() || prov.size()<2) throw 5;
      P.nome = move(prov);

      // Leh a latitude
      arq >> P.latitude;
      if (arq.fail()) throw 6;
      arq.ignore(1,';');

      // Leh a longitude
      arq >> P.longitude;
      if (arq.fail()) throw 7;
      arq >> ws;

      // Verifica se já existe ponto com a mesma ID no contêiner de pontos lidos (listP)
      // Caso exista, lança uma exceção (throw 8)
      auto it = find_if(listP.begin(), listP.end(), [&P](const Ponto& ponto) {
          return ponto.id == P.id; // Verifica se o ID do ponto atual é igual ao ID do novo ponto
      });

      if (it != listP.end()) {
          throw 8; // Lança a exceção 8 se o ID já existe
}

      // Inclui o ponto na lista de pontos
      listP.push_back(move(P));
    }
    while (!arq.eof());

    // Fecha o arquivo de pontos
    arq.close();
  }
  catch (int i)
  {
    cerr << "Erro " << i << " na leitura do arquivo de pontos "
         << arq_pontos << endl;
    return false;
  }

  // Leh as rotas do arquivo
  try
  {
    // Abre o arquivo de rotas
    ifstream arq(arq_rotas);
    if (!arq.is_open()) throw 1;

    // Leh o cabecalho
    getline(arq,prov);
    if (arq.fail() ||
        prov != "ID;Nome;Extremidade 1;Extremidade 2;Comprimento") throw 2;

    // Leh as rotas
    do
    {
      // Leh a ID
      getline(arq,prov,';');
      if (arq.fail()) throw 3;
      R.id.set(move(prov));
      if (!R.valid()) throw 4;

      // Leh o nome
      getline(arq,prov,';');
      if (arq.fail() || prov.size()<2) throw 4;
      R.nome = move(prov);

      // Leh a id da extremidade[0]
      getline(arq,prov,';');
      if (arq.fail()) throw 6;
      R.extremidade[0].set(move(prov));
      if (!R.extremidade[0].valid()) throw 7;

      // Verifica se a Id corresponde a um ponto no contêiner de pontos lidos (listP)
      // Caso ponto não exista, lança uma exceção (throw 8)
      auto it_ext0 = find_if(listP.begin(), listP.end(), [&R](const Ponto& ponto) {
          return ponto.id == R.extremidade[0];
      });

      if (it_ext0 == listP.end()) {
          throw 8; // Lança a exceção 8 se o ponto com extremidade[0] não for encontrado
      }

      // Leh a id da extremidade[1]
      getline(arq,prov,';');
      if (arq.fail()) throw 9;
      R.extremidade[1].set(move(prov));
      if (!R.extremidade[1].valid()) throw 10;

      // Verifica se a Id corresponde a um ponto no contêiner de pontos lidos (listP)
      // Caso ponto não exista, lança uma exceção (throw 11)
      auto it_ext1 = find_if(listP.begin(), listP.end(), [&R](const Ponto& ponto) {
          return ponto.id == R.extremidade[1];
      });

      if (it_ext1 == listP.end()) {
          throw 11; // Lança a exceção 11 se o ponto com extremidade[1] não for encontrado
      }


      // Leh o comprimento
      arq >> R.comprimento;
      if (arq.fail()) throw 12;
      arq >> ws;

      // Verifica se já existe rota com a mesma ID no contêiner de rotas lidas (listR)
      // Caso exista, lança uma exceção (throw 13)
      auto it_rota = find_if(listR.begin(), listR.end(), [&R](const Rota& rota) {
          return rota.id == R.id;
      });

if (it_rota != listR.end()) {
    throw 13; // Lança a exceção 13 se já existir uma rota com o mesmo ID
}

      // Inclui a rota na lista de rotas
      listR.push_back(move(R));
    }
    while (!arq.eof());

    // Fecha o arquivo de rotas
    arq.close();
  }
  catch (int i)
  {
    cerr << "Erro " << i << " na leitura do arquivo de rotas "
         << arq_rotas << endl;
    return false;
  }

  // Soh chega aqui se nao entrou no catch, jah que ele termina com return.
  // Move as listas de pontos e rotas para o planejador.
  pontos = move(listP);
  rotas = move(listR);

  return true;
}

/// *******************************************************************************
/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// *******************************************************************************

/// Noh: os elementos dos conjuntos de busca do algoritmo A*
struct Noh {
    IDPonto id_pt;    // Identificador do ponto
    IDRota id_rt;     // Identificador da rota até o ponto
    double g;         // Custo acumulado do caminho
    double h;         // Heurística (estimativa do custo restante)
    
    // Função custo total
    double f() const { 
        return g + h; 
    }

    // Construtor padrão
    Noh(const IDPonto& idP = IDPonto(), const IDRota& idR = IDRota(),
        double custoG = 0.0, double custoH = 0.0)
        : id_pt(idP), id_rt(idR), g(custoG), h(custoH) {}
};

/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// Retorna o comprimento do caminho encontrado.
/// (<0 se  parametros invalidos ou nao existe caminho).
/// O parametro C retorna o caminho encontrado
/// (vazio se  parametros invalidos ou nao existe caminho).
/// O parametro NA retorna o numero de nos em aberto ao termino do algoritmo A*
/// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
/// O parametro NF retorna o numero de nos em fechado ao termino do algoritmo A*
/// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
double Planejador::calculaCaminho(const IDPonto& id_origem,
                                  const IDPonto& id_destino,
                                  Caminho& C, int& NA, int& NF)
{
    // Zera o caminho resultado
    C.clear();

    try {
        // Verificações iniciais
        if (empty()) throw 1;

        Ponto pt_orig = getPonto(id_origem);
        if (!pt_orig.valid()) throw 4;

        Ponto pt_dest = getPonto(id_destino);
        if (!pt_dest.valid()) throw 5;

        // Contêineres do algoritmo
        list<Noh> Aberto, Fechado;

        // Noh inicial
        Noh atual(id_origem, IDRota(), 0.0, haversine(pt_orig, pt_dest));
        Aberto.push_back(atual);

        // Laço principal
        while (!Aberto.empty()) {
            // Encontra o nó com menor custo total f()
            Aberto.sort([](const Noh& a, const Noh& b) {
                return a.f() < b.f();
            });

            atual = Aberto.front();
            Aberto.pop_front();

            // Verifica se o destino foi alcançado
            if (atual.id_pt == id_destino) {
                // Reconstrói o caminho a partir do Fechado
                C.clear();
                double comprimento_total = atual.g;

                while (atual.id_rt != IDRota()) {
                    C.push_front({atual.id_rt, atual.id_pt});
                    Rota rota_ant = getRota(atual.id_rt);

                    // Identifica o antecessor
                    atual.id_pt = (rota_ant.extremidade[0] == atual.id_pt)
                                      ? rota_ant.extremidade[1]
                                      : rota_ant.extremidade[0];
                    
                    auto it = find_if(Fechado.begin(), Fechado.end(),
                                      [&atual](const Noh& n) {
                                          return n.id_pt == atual.id_pt;
                                      });
                    if (it != Fechado.end())
                        atual = *it;
                }

                // Calcula nós em Aberto e Fechado
                NA = Aberto.size();
                NF = Fechado.size()+1;

                return comprimento_total;
            }

            // Move o nó atual para Fechado
            Fechado.push_back(atual);

            // Gera sucessores
            for (const Rota& rota : rotas) {
                if (rota.extremidade[0] != atual.id_pt && rota.extremidade[1] != atual.id_pt)
                    continue;

                // Define a extremidade do sucessor
                IDPonto id_suc = (rota.extremidade[0] == atual.id_pt)
                                     ? rota.extremidade[1]
                                     : rota.extremidade[0];

                if (find_if(Fechado.begin(), Fechado.end(),
                            [&id_suc](const Noh& n) {
                                return n.id_pt == id_suc;
                            }) != Fechado.end()) {
                    continue; // Ignora nós já processados
                }

                Ponto pt_suc = getPonto(id_suc);
                double custo_g = atual.g + rota.comprimento;
                double custo_h = haversine(pt_suc, pt_dest);

                // Verifica se o nó já está em Aberto
                auto it = find_if(Aberto.begin(), Aberto.end(),
                                  [&id_suc](const Noh& n) {
                                      return n.id_pt == id_suc;
                                  });

                if (it != Aberto.end()) {
                    if (custo_g + custo_h < it->f()) {
                        Aberto.erase(it);
                        Aberto.emplace_back(id_suc, rota.id, custo_g, custo_h);
                    }
                } else {
                    Aberto.emplace_back(id_suc, rota.id, custo_g, custo_h);
                }
            }
        }

        // Não há solução
        NA = Aberto.size();
        NF = Fechado.size();
        return -1.0;
    } catch (int i) {
        cerr << "Erro " << i << " no calculo do caminho\n";
        NA = NF = -1;
        return -1.0;
    }
}
