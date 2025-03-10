#ifndef _PLANEJADOR_H_
#define _PLANEJADOR_H_

#include <string>
#include <list>
#include <ostream>

/* *************************
   * CLASSE IDPONTO        *
   ************************* */

/// Identificador de um Ponto
class IDPonto
{
private:
  std::string t;
public:
  // Construtor
  IDPonto(): t("") {}
  // Atribuicao de string
  void set(std::string&& S);
  // Teste de validade
  bool valid() const
  {
    return (t.size()>=2 && t[0]=='#');
  }
  // Comparacao
  bool operator==(const IDPonto& ID) const
  {
    return t==ID.t;
  }
  bool operator!=(const IDPonto& ID) const
  {
    return !operator==(ID);
  }
  // Impressao
  friend std::ostream& operator<<(std::ostream& X, const IDPonto& ID)
  {
    return X<<ID.t;
  }
};

/* *************************
   * CLASSE IDROTA         *
   ************************* */

/// Identificador de uma Rota
class IDRota
{
private:
  std::string t;
public:
  // Construtor
  IDRota(): t("") {}
  // Atribuicao de string temporaria
  void set(std::string&& S);
  // Teste de validade
  bool valid() const
  {
    return (t.size()>=2 && t[0]=='&');
  }
  // Comparacao
  bool operator==(const IDRota& ID) const
  {
    return t==ID.t;
  }
  bool operator!=(const IDRota& ID) const
  {
    return !operator==(ID);
  }
  // Impressao
  friend std::ostream& operator<<(std::ostream& X, const IDRota& ID)
  {
    return X<<ID.t;
  }
};

/* *************************
   * CLASSE PONTO          *
   ************************* */

/// Um ponto no mapa
struct Ponto
{
  IDPonto id;        // Identificador do ponto
  std::string nome;  // Denominacao usual do ponto
  double latitude;   // Em graus: -90 polo sul, +90 polo norte
  double longitude;  // Em graus: de -180 a +180 (positivos a leste de Greenwich,
                     //                           negativos a oeste de Greenwich)
  // Construtor default
  Ponto(): id(), nome(""), latitude(0.0), longitude(0.0) {}
  // Teste de validade
  bool valid() const
  {
    return id.valid();
  }
  // Sobrecarga de operadores
  // Utilizados pelos algoritmos STL
  // Sobrecarga do operador de comparação de igualdade (==)
bool operator==(const Ponto& outro) const {
    return id == outro.id && nome == outro.nome &&
           latitude == outro.latitude && longitude == outro.longitude;
}

// Sobrecarga do operador de comparação de desigualdade (!=)
bool operator!=(const Ponto& outro) const {
    return !(*this == outro); // A desigualdade é a negação da igualdade
}
};

/// Distancia entre 2 pontos (formula de haversine)
double haversine(const Ponto& P1, const Ponto& P2);

/* *************************
   * CLASSE ROTA           *
   ************************* */

/// Uma rota no mapa
struct Rota
{
  IDRota id;              // Identificador da rota
  std::string nome;       // Denominacao usual da rota
  IDPonto extremidade[2]; // Ids dos pontos extremos da rota
  double comprimento;     // Comprimento da rota (em km)

  // Construtor default
  Rota(): id(), nome(""), extremidade(), comprimento(0.0) {}
  // Teste de validade
  bool valid() const
  {
    return id.valid();
  }
  // Sobrecarga de operadores
  // Utilizados pelos algoritmos STL
  // Sobrecarga do operador de comparação de igualdade (==)
bool operator==(const Rota& outra) const {
    return id == outra.id && nome == outra.nome &&
           extremidade[0] == outra.extremidade[0] &&
           extremidade[1] == outra.extremidade[1] &&
           comprimento == outra.comprimento;
}

// Sobrecarga do operador de comparação de desigualdade (!=)
bool operator!=(const Rota& outra) const {
    return !(*this == outra); // A desigualdade é a negação da igualdade
}
};

/* *************************
   * CLASSE CAMINHO        *
   ************************* */

/// Um caminho encontrado entre dois pontos: uma lista de pares <IDRota,IDPonto>
/// No 1o elemento (1o par) do Caminho, a rota eh vazia == Rota() e o ponto eh a origem.
/// Cada elemento, exceto o primeiro, eh composto pela rota que trouxe do
/// elemento anterior ateh ele e pelo ponto que faz parte do caminho.
/// No ultimo elemento, o ponto eh o destino.
using Caminho = std::list< std::pair<IDRota,IDPonto> >;

/* *************************
   * CLASSE PLANEJADOR     *
   ************************* */

/// A classe que armazena os pontos e as rotas do mapa do Planejador
/// e calcula caminho mais curto entre pontos.
class Planejador
{
private:
  std::list<Ponto> pontos;
  std::list<Rota> rotas;

public:
  /// Cria um mapa vazio
  Planejador(): pontos(), rotas() {}

  /// Cria um mapa com o conteudo dos arquivos arq_pontos e arq_rotas
  Planejador(const std::string& arq_pontos,
             const std::string& arq_rotas): Planejador()
  {
    ler(arq_pontos,arq_rotas);
  }

  /// Destrutor (nao eh obrigatorio...)
  ~Planejador()
  {
    clear();
  }

  /// Torna o mapa vazio
  void clear();

  /// Testa se um mapa estah vazio
  bool empty() const
  {
    return pontos.empty();
  }

  /// Retorna um Ponto do mapa, passando a id como parametro.
  /// Se a id for inexistente, retorna um Ponto vazio.
  Ponto getPonto(const IDPonto& Id) const;

  /// Retorna um Rota do mapa, passando a id como parametro.
  /// Se a id for inexistente, retorna um Rota vazio.
  Rota getRota(const IDRota& Id) const;

  /// Imprime o mapa no console
  void imprimirPontos() const;
  void imprimirRotas() const;

  /// Leh um mapa dos arquivos arq_pontos e arq_rotas.
  /// Caso nao consiga ler dos arquivos, deixa o mapa inalterado e retorna false.
  /// Retorna true em caso de leitura bem sucedida.
  bool ler(const std::string& arq_pontos,
           const std::string& arq_rotas);

  /// Calcula o caminho mais curto no mapa entre origem e destino, usando o algoritmo A*
  /// Retorna o comprimento do caminho encontrado.
  /// (<0 se parametros invalidos ou se nao existe caminho).
  /// O parametro C retorna o caminho encontrado
  /// (vazio se parametros invalidos ou se nao existe caminho).
  /// O parametro NA retorna o numero de nos em aberto ao termino do algoritmo A*
  /// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
  /// O parametro NF retorna o numero de nos em fechado ao termino do algoritmo A*
  /// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
  double calculaCaminho(const IDPonto& id_origem,
                        const IDPonto& id_destino,
                        Caminho& C, int& NA, int& NF);
};

#endif // _PLANEJADOR_H_
