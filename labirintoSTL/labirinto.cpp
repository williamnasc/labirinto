#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>   /* srand, rand */
#include <string>
#include <time.h>
#include <cmath>
#include <list>
#include <algorithm>

#include "labirinto.h"

using namespace std;

/* ***************** */
/* CLASSE CELULA     */
/* ***************** */

string estadoCel2string(EstadoCel E)
{
  switch(E)
  {
  case EstadoCel::LIVRE:
    return "  ";
  case EstadoCel::OBSTACULO:
    return "##";
  case EstadoCel::ORIGEM:
    return "Or";
  case EstadoCel::DESTINO:
    return "De";
  case EstadoCel::CAMINHO:
    return "..";
  default:
    break;
  }
  return "??";
}

istream& operator>>(istream& I, Coord& C)
{
  I >> C.lin >> C.col;
  return I;
}

ostream& operator<<(ostream& O, const Coord& C)
{
  O << C.lin << ';' << C.col;
  return O;
}

/* ***************** */
/* CLASSE LABIRINTO  */
/* ***************** */

/// Torna o mapa vazio
void Labirinto::clear()
{
  // Esvazia o mapa de qualquer conteudo anterior
  NL = NC = 0;
  mapa.clear();
  // Apaga a origem e destino do caminho
  orig = dest = Coord();
}

/// Limpa a origem, destino e caminho anterior
void Labirinto::limpaCaminho()
{
  if (!empty()) for (unsigned i=0; i<NL; i++) for (unsigned j=0; j<NC; j++)
  {
    if (at(i,j) == EstadoCel::CAMINHO)
    {
      set(i,j) = EstadoCel::LIVRE;
    }
  }
}

/// Leh um mapa do arquivo nome_arq
/// Caso nao consiga ler do arquivo, cria mapa vazio
/// Retorna true em caso de leitura bem sucedida
bool Labirinto::ler(const string& nome_arq)
{
  // Limpa o mapa
  clear();

  // Abre o arquivo
  ifstream arq(nome_arq.c_str());
  if (!arq.is_open())
  {
    return false;
  }

  string prov;
  int numL, numC;
  int valor;

  // Leh o cabecalho
  arq >> prov >> numL >> numC;
  if (prov != "LABIRINTO" ||
      numL<ALTURA_MIN_MAPA || numL>ALTURA_MAX_MAPA ||
      numC<LARGURA_MIN_MAPA || numC>LARGURA_MAX_MAPA)
  {
    arq.close();
    return false;
  }

  // Redimensiona o mapa
  NL = numL;
  NC = numC;
  mapa.resize(NL*NC);

  // Leh as celulas do arquivo
  for (unsigned i=0; i<NL; i++) for (unsigned j=0; j<NC; j++)
  {
    arq >> valor;
    set(i,j) = (valor==0 ?
                EstadoCel::OBSTACULO :
                EstadoCel::LIVRE);
  }
  arq.close();
  return true;
}

/// Salva um mapa no arquivo nome_arq
/// Retorna true em caso de escrita bem sucedida
bool Labirinto::salvar(const string& nome_arq) const
{
  // Testa o mapa
  if (empty()) return false;

  // Abre o arquivo
  ofstream arq(nome_arq.c_str());
  if (!arq.is_open())
  {
    return false;
  }

  // Salva o cabecalho
  arq << "LABIRINTO " << NL << ' ' << NC << endl;

  // Salva as celulas do mapa
  for (unsigned i=0; i<NL; i++)
  {
    for (unsigned j=0; j<NC; j++)
    {
      arq << (at(i,j) == EstadoCel::OBSTACULO ? 0 : 1 ) << ' ';
    }
    arq << endl;
  }

  arq.close();
  return true;
}

/// Gera um novo mapa aleatorio
/// numL e numC sao as dimensoes do labirinto
/// perc_obst eh o percentual de casas ocupadas no mapa. Se <=0, assume um valor aleatorio
/// entre PERC_MIN_OBST e PERC_MAX_OBST
/// Se os parametros forem incorretos, gera um mapa vazio
/// Retorna true em caso de geracao bem sucedida (parametros corretos)
bool Labirinto::gerar(unsigned numL, unsigned numC, double perc_obst)
{
  // Limpa o mapa
  clear();

  // Inicializa a semente de geracao de numeros aleatorios
  srand(time(NULL));

  // Calcula o percentual de obstaculos no mapa
  if (perc_obst <= 0.0)
  {
    perc_obst = PERC_MIN_OBST +
                (PERC_MAX_OBST-PERC_MIN_OBST)*(rand()/double(RAND_MAX));
  }

  // Testa os parametros
  if (numL<ALTURA_MIN_MAPA || numL>ALTURA_MAX_MAPA ||
      numC<LARGURA_MIN_MAPA || numC>LARGURA_MAX_MAPA ||
      perc_obst<PERC_MIN_OBST || perc_obst>PERC_MAX_OBST)
  {
    return false;
  }

  // Assume as dimensoes passadas como parametro
  NL = numL;
  NC = numC;

  // Redimensiona o mapa
  mapa.resize(NL*NC);

  // Preenche o mapa
  bool obstaculo;
  for (unsigned i=0; i<NL; i++) for (unsigned j=0; j<NC; j++)
  {
    obstaculo = (rand()/double(RAND_MAX) <= perc_obst);
    set(i,j) = (obstaculo ?
                EstadoCel::OBSTACULO :
                EstadoCel::LIVRE);
  }
  return true;
}

/// Testa se uma celula eh valida dentro de um mapa
bool Labirinto::coordValida(const Coord& C) const
{
  if (!C.valida()) return false;
  if (C.lin >= int(NL)) return false;
  if (C.col >= int(NC)) return false;
  return true;
}

/// Testa se uma celula estah livre (nao eh obstaculo) em um mapa
bool Labirinto::celulaLivre(const Coord& C) const
{
  if (!coordValida(C)) return false;
  if (at(C) == EstadoCel::OBSTACULO) return false;
  return true;
}

/// Fixa a origem do caminho a ser encontrado
bool Labirinto::setOrigem(const Coord& C)
{
  if (!celulaLivre(C)) return false;
  // Se for a mesma origen nao faz nada
  if (C==orig) return true;

  limpaCaminho();

  // Apaga a origem anterior no mapa, caso esteja definida
  if (coordValida(orig)) set(orig) = EstadoCel::LIVRE;

  // Fixa a nova origem
  orig = C;
  // Marca a nova origem no mapa
  set(orig) = EstadoCel::ORIGEM;

  return true;
}

/// Fixa o destino do caminho a ser encontrado
bool Labirinto::setDestino(const Coord& C)
{
  if (!celulaLivre(C)) return false;
  // Se for o mesmo destino nao faz nada
  if (C==dest) return true;

  limpaCaminho();

  // Apaga o destino anterior no mapa, caso esteja definido
  if (coordValida(dest)) set(dest) = EstadoCel::LIVRE;

  // Fixa o novo destino
  dest = C;
  // Marca o novo destino no mapa
  set(dest) = EstadoCel::DESTINO;

  return true;
}

/// Imprime o mapa no console
void Labirinto::imprimir() const
{
  if (empty())
  {
    cout << "+------------+" << endl;
    cout << "| MAPA VAZIO |" << endl;
    cout << "+------------+" << endl;
    return;
  }

  unsigned i,j;

  // Impressao do cabecalho
  cout << "    ";
  for (j=0; j<NC; j++)
  {
    cout << setfill('0') << setw(2) << j << setfill(' ') << setw(0) << ' ' ;
  }
  cout << endl;

  cout << "   +";
  for (j=0; j<NC; j++) cout << "--+" ;
  cout << endl;

  // Imprime as linhas
  for (i=0; i<NL; i++)
  {
    cout << setfill('0') << setw(2) << i << setfill(' ') << setw(0) << " |" ;
    for (j=0; j<NC; j++)
    {
      cout << estadoCel2string(at(i,j)) << '|' ;
    }
    cout << endl;

    cout << "   +";
    for (j=0; j<NC; j++) cout << "--+" ;
    cout << endl;
  }
}




/// Calcula o caminho entre a origem e o destino do labirinto usando o algoritmo A*
///
/// Retorna o comprimento do caminho (<0 se nao existe)
///
/// O parametro NC retorna o numero de nos no caminho encontrado (profundidade da busca)
/// O parametro NA retorna o numero de nos em aberto ao termino do algoritmo A*
/// O parametro NF retorna o numero de nos em fechado ao termino do algoritmo A*
/// Os 3 parametros NA, NC e NF retornam <0 caso nao exista caminho
double Labirinto::calculaCaminho(int& NC, int& NA, int& NF)
{
  if (empty() || !origDestDefinidos())
  {
    // Nao hah caminho
    NC = NA = NF = -1;
    return -1.0;
  }

  // Apaga um eventual caminho anterior
  limpaCaminho();

  // Testa se origem igual a destino
  if (orig==dest)
  {
    // Caminho tem profundidade nula
    NC = 0;
    // Algoritmo de busca nao gerou nenhum noh
    NA = NF = 0;
    // Caminho tem comprimento nulo
    return 0.0;
  }

  /*************************/
  /****** FALTA FAZER ******/
  /*************************/

    noh origem;
    origem.pos = orig;
    origem.ant = Coord();
    origem.g = 0;
    origem.h = heuristica(origem.pos,dest); //sqrt(pow((dest.lin-origem.pos.lin),2) + pow((dest.col-origem.pos.col),2));

    list<noh> aberto;
    list<noh> fechado;

    aberto.push_back(origem);

    list<noh>::iterator ptr_fechado;
    list<noh>::iterator ptr_aberto;


    noh atual;
    bool encontrou_solucao = false;
    do{

        ///ENCONTRA O MENOR CUSTO
        double menorCT = 0;         ///guarda o menor custo da lista
        double CustoT;              ///guarta o custo total do iterador
        ///Percorre a lista a procura qual é o menor custo total
        for (ptr_aberto = aberto.begin();ptr_aberto != aberto.end(); ptr_aberto++) {
            CustoT = ptr_aberto->g + ptr_aberto->h;
            if(ptr_aberto == aberto.begin())
                menorCT = CustoT;
            else {
                if(CustoT < menorCT)
                    menorCT = CustoT;
            }
        }
        ///objeto funcao para encontar o ojeto de menor custo
        menor MenorCusto;
        MenorCusto.X = menorCT;
        ///busca o objeto de menor custo na lista
        ptr_aberto = find_if(aberto.begin(),aberto.end(),MenorCusto);
        atual = *ptr_aberto;

        ///DELETA DE ABERTO E SALVA EM FECHADO
        aberto.erase(ptr_aberto);
        fechado.push_back(atual);

        if(atual.pos == dest){
            ///É SOLUCAO
            encontrou_solucao = true;

        }else {
            /// NAO EH SOLUCAO, LOGO
            /// GERA NOVOS NOHS

            Coord dir;              /// representa direcao do novo noh
            noh suc,old;            /// guarda os nohs analisados
            bool descarta = false;  /// guarda se deve salvar ou nao o novo noh

            /// PARA TODAS AS DIRECOES POSSIVEIS
            for (dir.lin = -1;dir.lin < 2;dir.lin++){
                for (dir.col = -1;dir.col < 2;dir.col++){
                  ///TESTA SE A POSSICAO NESSA DIRECAO EH VALIDA
                  if(coordValida(atual.pos + dir) && celulaLivre(atual.pos + dir) && (atual.pos != atual.pos + dir)){
                      /// TESTA SE FOR EM UMA ESQUINA Q N PODE SE MOVER NAS DIAGONAIS DA ESQUINA
                      if(!(
                            norm(dir)>1 && (
                            !celulaLivre(Coord(atual.pos.lin + dir.lin,atual.pos.col) ) ||
                            !celulaLivre(Coord(atual.pos.lin,atual.pos.col + dir.col) ))
                        )
                       ){

                     /// GERA NOH SUCESSOR
                     suc.pos = atual.pos + dir;
                     suc.ant = atual.pos;
                     suc.g = atual.g + norm(dir);
                     suc.h = heuristica(suc.pos,dest);

                     /// PROCURA SE EXISTE OUTRO NO NA MESMA POSICAO Q ESSE NOVO,
                     /// E GUARDA NO NOH OLD
                     ptr_fechado = find(fechado.begin(),fechado.end(),suc.pos);
                     old = *ptr_fechado;

                     ///SE OLD EXISTIR VER QUAL DOS 2 EH O MELHOR
                     if(old.pos == suc.pos){
                         if(suc < old){
                            fechado.erase(ptr_fechado);
                         }else {
                            ///descarta suc
                            descarta = true;
                         }
                     }

                     /// PROCURA SE EXISTE OUTRO NO NA MESMA POSICAO Q ESSE NOVO,
                     /// E GUARDA NO NOH OLD
                     ptr_aberto = find(aberto.begin(),aberto.end(),suc.pos);
                     old = *ptr_aberto;

                     ///SE OLD EXISTIR VER QUAL DOS 2 EH O MELHOR
                     if(old.pos == suc.pos){
                         if(suc < old){
                            aberto.erase(ptr_aberto);
                         }else {
                            ///descarta suc
                            descarta = true;
                         }
                     }

                     /// SE FOR O CASO, GUARDA O NOVO NOH EM ABERTO
                     if(!descarta){
                         aberto.push_front(suc);
                     }
                     descarta = false;  ///RESETA A VARIAVEL PARA PROXIMO LOOP

                   } }  // FECHA SE
                }       //FECHA LOOP
            }           //FECHA LOOP
        }               // FECHA SENAO
    /// ENQUANTO EXISTIR NOHS EM ABERTO E N ACHOU SOLUCAO
    }while(aberto.size() > 0 && encontrou_solucao == false);
/*
    cout << "\nTAMANHO DE ABERTO: " <<aberto.size() << endl;
    cout << "\nTAMANHO DE FECHADO: " <<fechado.size()<< endl;
*/
    /// CHECA SE ACHOU SOLUCAO
    if(!encontrou_solucao){
        //cout << "NÃO EXISTE SOLUCAO !!!" << endl;

        // Nao hah caminho
        NC = NA = NF = -1;
        return -1.0;

    }else {
        NC = 0;
        double dist=0;                          /// guarda a distancia da origem ate o destino
        //cout << "EXISTE SOLUCAO !!!" << endl;
        /// ENQUANTO NOH ATUAL N FOR A ORIGEM
        while (atual.ant != Coord() ) {

            /// PINTA NO MAPA O CAMINHO
            if(! (atual.ant == orig))
                set(atual.ant) = EstadoCel::CAMINHO;
            /// ADICIONA A DISTANCIA DO CAMINHO
            if(!(atual == orig))
                dist += heuristica(atual.pos,atual.ant);

            ptr_fechado = find(fechado.begin(),fechado.end(),atual.ant);
            atual = *ptr_fechado;
            NC++;

        }
        /// GUARDA OS TAMANHOS DAS LISTAS
        NA = aberto.size();
        NF = fechado.size();
        cout << "\nNum nohs em caminho (profundidade da busca): " << NC << endl;

        return dist;

    }



}
