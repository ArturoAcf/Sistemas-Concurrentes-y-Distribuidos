//Realizado por: Arturo Alonso Carbonero
//DNI: 75936665-A
//Grupo: 2ºC - C1
//Fumador - Estanquero con monitor SU
//g++ -std=c++11 -pthread -I. -o FumadoresSU FumadoresSU.cpp Semaphore.cpp HoareMonitor.cpp
//Solución basada en la implementación con semáforos anteriormente entregada

#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

const int numIngredientes=3; // Total de tipos disponibles y número de fumadores, no varía.

//Generador aleatorio de números
template<const int min, const int max> int aleatorio(){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min,max);
  return distribucion_uniforme(generador);
}

//Monitor
class Estanco: public HoareMonitor{
private:
  int mostrador; //Variable cuyo valor mostrará el estado del mostrador (Vacío - Lleno)
  CondVar mostradorVacio;
  CondVar buffer[numIngredientes]; // Cada posición equivale a un ingrediente.
                                   // Si buffer[i]=1 => ingrediente i disponible.
public:
  Estanco();
  void obtenerIngrediente(int i);    // Función para obtener ingrediente.
  void ponerIngrediente(int i);      // Función para poner ingrediente.
  void esperarRecogidaIngrediente(); // Función para esperar a que se recoga el ingrediente.
};

//Constructor del monitor
Estanco::Estanco(){
  mostrador=-1; // si mostrador=-1 => mostrador vacío.
  mostradorVacio=newCondVar();

  for(int i=0; i<numIngredientes; i++){
    buffer[i]=newCondVar();
  }
}

//Función para poner un ingrediente en el mostrador
void Estanco::ponerIngrediente(int i){
  esperarRecogidaIngrediente();
  mostrador=i; // El valor define el tipo de objeto que está actualmente en el mostrador.
  cout<<"Estanquero -> Ingrediente: "<<mostrador<<" colocado"<<endl;
  buffer[i].signal(); // Pongo el ingrediente i aumentando en 1 la pos i del buffer.
}

//Función para esperar la recogida del ingrediente que se encuentre en el mostrador
void Estanco::esperarRecogidaIngrediente(){
  if(mostrador!=-1){
    mostradorVacio.wait();
  }
}

//Función para obtener el ingrediente
void Estanco::obtenerIngrediente(int i){
  if(mostrador!=i){
    buffer[i].wait(); // Retiro el ingrediente i disminuyendo en 1 la pos i del buffer.
  }

  cout<<"                Ingrediente "<<i<<" retirado"<<endl;

  mostrador=-1; // Vacío el mostrador.
  mostradorVacio.signal();
}

//Función fumar -> Espera aleatoria
void fumar(int num_fum){
  cout<<"Fumador "<<num_fum<<" -> comienza a fumar"<<endl;
  this_thread::sleep_for(chrono::milliseconds(aleatorio<50,200>()));
  cout<<"Fumador "<<num_fum<<" -> termina de fumar"<<endl;
}

//Función para producir un ingrediente aleatorio
int producirIngrediente(){
  chrono::milliseconds tiempoProd(aleatorio<40,100>());
  cout<<"\nProduciendo..."<<endl;
  this_thread::sleep_for(tiempoProd); // Esperar para producir

  int ingr=aleatorio<0,2>();
  return ingr;
}

//Funcion que ejecuta la hebra estanquero
void funcionHebraEstanquero(MRef<Estanco> Estanco){
   int ingr=0; // Tipo de ingrediente.
// int control=0; -> Variable para detener el bucle infinito

  while(true){
    ingr=producirIngrediente(); // Producir ingrediente

    Estanco->ponerIngrediente(ingr);
    Estanco->esperarRecogidaIngrediente();

    /* control++;
    if(control==20){
      break;
    }
    *///-> Se detiene el bucle tras 20 ejecuciones, simplemente por comodidad a la hora de visualizar la solución, pero no es necesario.
  }
}

//Funcion que ejecuta la hebra fumador
void funcionHebraFumador(int dato, MRef<Estanco> Estanco){
  while(true){
    Estanco->obtenerIngrediente(dato);

    fumar(dato);
  }
}

//Función principal main
int main(){
  //Creo el monitor
  MRef<Estanco> estanco=Create<Estanco>();

  //Creo las hebras y las lanzo
  thread hebraEstanquera(funcionHebraEstanquero, estanco);
  thread hebrasFumadoras[numIngredientes];

  for(int i=0; i<numIngredientes; i++){
    hebrasFumadoras[i]=thread(funcionHebraFumador, i, estanco);
  }

  //Espera
  hebraEstanquera.join();
  for(int i=0; i<numIngredientes; i++){
    hebrasFumadoras[i].join();
  }

  return 0;
}
