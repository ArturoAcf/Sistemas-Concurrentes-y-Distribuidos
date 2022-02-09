//Realizado por: Arturo Alonso Carbonero
//Grupo: 2ºC - C1
//Productor - Consumidor (LIFO)
//g++ -std=c++11 -pthread -I. -o ProdconsFIFO ProdconsFIFO.cpp Semaphore.cpp

#include <iostream>
#include <random>
#include <thread>
#include "Semaphore.h"

using namespace std;
using namespace SEM;

//Generador aleatorio de números entre min y max
template<const int min, const int max> int aleatorio(){
  static default_random_engine generador((random_device())());
  static uniform_int_distribution<int> distribucion_uniforme(min,max);
  return distribucion_uniforme(generador);
}

//Variables compartidas
const int tam_vector=10; //K
const int num_items=60; //Cantidad de datos que se van a producir/consumir
int buffer[tam_vector]; //Array donde se cargarán y leerán los datos
int p_libre=0; //LIFO

//Semáforos
Semaphore puede_leer=0; //Si vale 0 se puede leer el dato del buffer
Semaphore puede_escribir=1; //Ídem, pero escribiendo

//Función para producir datos
int producirDato(){
  static int contador=0; //Datos que se irán produciendo/consumiendo
  this_thread::sleep_for(chrono::milliseconds(aleatorio<20,100>()));
  contador+=1; //Se produce el dato siguiente
  cout<<"Productor: dato producido: "<<contador<<endl;
  return contador;
}

//Función para consumir datos
void consumirDato(int dato){
  this_thread::sleep_for(chrono::milliseconds(aleatorio<20,100>()));
  cout<<"                Consumidor: dato consumido: "<<dato<<endl;
}

//Hebra prodcutora
void funcionHebraProductora(){
  for(int i=0; i<num_items; i++){
    int a=producirDato(); //1...num_items=60

    sem_wait(puede_escribir); //Valor=1-1=0, puede cargar el dato en el buffer

    buffer[p_libre]=a;
    p_libre+=1; //Nueva posición libre

    sem_signal(puede_leer); //Valor=0+1=1, para que la otra hebra lea el dato del buffer
  }
}

//Hebra consumidora
void funcionHebraConsumidora(){
  for(int i=0; i<num_items; i++){
    sem_wait(puede_leer); //Valor=1-1=0, puede leer el dato del buffer

    int b;
    p_libre-=1; //Posición ya cargada, decremento p_libre
    b=buffer[p_libre];

    sem_signal(puede_escribir); //Valor=0+1=1, para que la otra hebra pueda cargar otro dato en el buffer
    consumirDato(b); //1...num_items=60
  }
}

//Función principal main
int main(){
  //Creo las hebras y las lanzo
  thread hebraProductora(funcionHebraProductora);
  thread hebraConsumidora(funcionHebraConsumidora);
  hebraProductora.join();
  hebraConsumidora.join();

  //Terminan las hebras y muestro FIN
  cout<<"\n----------------------FIN----------------------"<<endl;

  return 0;
}
