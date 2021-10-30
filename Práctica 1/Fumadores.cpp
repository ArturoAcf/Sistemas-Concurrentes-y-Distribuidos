//Realizado por: Arturo Alonso Carbonero
//DNI: 75936665-A
//Grupo: 2ºC - C1
//Fumador - Estanquero
//g++ -std=c++11 -pthread -I. -o Fumadores Fumadores.cpp Semaphore.cpp

#include <iostream>
#include <random>
#include <thread>
#include "Semaphore.h"

using namespace std;
using namespace SEM;

//Generador aleatorio de números
template<const int min, const int max> int aleatorio(){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min,max);
  return distribucion_uniforme(generador);
}

//Variables compartidas
int mostrador=0;

//Semáforos
Semaphore mostradorVacio=1; //Si vale 0 el mostrador está vacío
Semaphore buffer[3]={0,0,0}; //Vector de semáforos, donde cada posición equivale a un ingrediente.
                             //Si buffer[i]=1 => ingrediente i disponible


//Función fumar
void fumar(int num_fum){
  cout<<"Fumador "<<num_fum<<" -> comienza a fumar"<<endl;
  this_thread::sleep_for(chrono::milliseconds(aleatorio<50,200>()));
  cout<<"Fumador "<<num_fum<<" -> termina de fumar"<<endl;
}

//Funcion que ejecuta la hebra estanquero
void funcionHebraEstanquero(){
  int ingr=0; //Ingrediente entre 0 y 2 (aleatoriamente) que se colcorá en al mostrador
  // int control=0; -> Variable para detener el bucle infinito

  while(true){
    ingr=aleatorio<0,2>();
    sem_wait(mostradorVacio); //Valor=1-1=0, se procede a colocar ingrediente:

    mostrador=ingr;
    cout<<"Estanquero -> Ingrediente: "<<mostrador<<" colcado sobre el mostrador"<<endl;

    sem_signal(buffer[mostrador]); //Valor=0+1=1; Ingrediente en la posición ingr del vector de hebras disponible para el fumador correspondiente

    /* control++;
    if(control==20){
      break;
    }
    *///-> Se detiene el bucle tras 20 ejecuciones, simplemente por comodidad a la hora de visualizar la solución, pero no es necesario.        
  }
}

//Funcion que ejecuta la hebra fumador
void funcionHebraFumador(int dato){
  while(true){
    sem_wait(buffer[dato]); //Valor=1-1=0; Valor en la posición dato (ingr) -1 para indicar que falta

    cout<<"                Ingrediente "<<dato<<" retirado"<<endl;

    sem_signal(mostradorVacio); //Valor=0+1=1 -> mostrador vacío
    fumar(dato);
  }
}

//Función principal main
int main(){
  //Creo las hebras y las lanzo
  thread hebraEstanquera(funcionHebraEstanquero);
  thread hebrasFumadoras[3];

  for(int i=0; i<3; i++){
    hebrasFumadoras[i]=thread(funcionHebraFumador, i);
  }

  //Espera
  hebraEstanquera.join();
  for(int i=0; i<3; i++){
    hebrasFumadoras[i].join();
  }

  return 0;
}
