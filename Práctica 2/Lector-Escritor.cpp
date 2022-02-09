//Realizado por: Arturo Alonso Carbonero
//Grupo: 2ºC - C1
//Lector - Escritor con monitor SU
//g++ -std=c++11 -pthread -I. -o Lector-Escritor Lector-Escritor.cpp Semaphore.cpp HoareMonitor.cpp

#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

const int num_lectores=3;
const int num_escritores=3;

//Generador aleatorio de números
template<const int min, const int max> int aleatorio(){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min,max);
  return distribucion_uniforme(generador);
}

//Función leer
void leer(int lec){
  cout<<"Posible leer, lector "<< lec <<" leyendo"<<endl;
  chrono::milliseconds tiempoProd(aleatorio<40,100>());
  this_thread::sleep_for(tiempoProd);
  cout<<"Fin de lectura"<<endl;
}

//Función escribir
void escribir(int esc){
  cout<<"\nEscritor "<< esc <<", escribiendo..."<<endl;
  chrono::milliseconds tiempoProd(aleatorio<40,100>());
  this_thread::sleep_for(tiempoProd);
  cout<<"Fin de la escritura"<<endl;
}

//Monitor
class lecEsc: public HoareMonitor{
private:
  int n_lec;         // Número de lectores leyendo
  bool escrib;       // True si hay escritor escribiendo
  CondVar lectura;   // No hay escritoror escribiendo => lectura posible
  CondVar escritura; // No hay ni escritor ni lector => escritura posible
public:
  lecEsc();
  void ini_lectura();
  void fin_lectura();
  void ini_escritura();
  void fin_escritura();
};

//Constructor del monitor
lecEsc::lecEsc(){
  n_lec=0;
  escrib=false;
  lectura=newCondVar();
  escritura=newCondVar();
}

//Iniciar lectura
void lecEsc::ini_lectura(){
  if(escrib==true){
    lectura.wait();
  }

  n_lec+=1;
  lectura.signal();
}

//Finalizar lectura
void lecEsc::fin_lectura(){
  n_lec--;

  if(n_lec==0){
    escritura.signal();
  }
}

//Iniciar escritura
void lecEsc::ini_escritura(){
  if(escrib==true || n_lec>0){
    escritura.wait();
  }

  escrib=true;
}

//Finalizar escritura
void lecEsc::fin_escritura(){
  escrib=false;

  if(lectura.get_nwt()!=0){ // Si hay lectores, estos tienen prioridad
    lectura.signal();
  }else{
    escritura.signal();
  }
}

//Función que ejecutan los lectores
void funcionLector(int num_lec, MRef<lecEsc> lec_Esc){
  while(true){
    lec_Esc->ini_lectura();
    leer(num_lec);
    lec_Esc->fin_lectura();
  }
}

//Función que ejecutan los escritores
void funcionEscritor(int num_esc, MRef<lecEsc> lec_Esc){
  while(true){
    lec_Esc->ini_escritura();
    escribir(num_esc);
    lec_Esc->fin_escritura();
  }
}

//Función principal main
int main(){

  //Creo el monitor
  MRef<lecEsc> lec_Esc=Create<lecEsc>();

  //Creo las hebras y las lanzo
  thread hebrasEscritoras[num_escritores];
  thread hebrasLectoras[num_lectores];

  for(int i=0; i<num_escritores; i++){
    hebrasEscritoras[i]=thread(funcionEscritor, i, lec_Esc);
  }

  for(int i=0; i<num_lectores; i++){
    hebrasLectoras[i]=thread(funcionLector, i, lec_Esc);
  }

  //Espera
  for(int i=0; i<num_escritores; i++){
    hebrasEscritoras[i].join();
  }

  for(int i=0; i<num_lectores; i++){
    hebrasLectoras[i].join();
  }

  return 0;
}
