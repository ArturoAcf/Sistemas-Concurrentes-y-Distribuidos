//Realizado por: Arturo Alonso Carbonero
//Grupo: 2ºC - C1
//Ejercicio: Implementación de la solución al problema Productor-Consumidor con varios productores y consumidores
//           haciendo uso de monitores con semántica SU (señal espera ugrente).
//Gestión FIFO. Solución basada en mi Implementación con monitores SC para varios productores y consumidores (FIFO),
//              a su vez basada en la Implementación con monitores SC para un productor y un consumidor proporcionada.

#include <iostream>
#include <iomanip>
#include <cassert>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"

using namespace HM;
using namespace std;

//Variables comunes
constexpr int num_items=40;
mutex mtx;
unsigned cont_prod[num_items];
unsigned cont_cons[num_items];
static const int num_productores=2; //Debe ser divisor de num_items=40
int num_consumidores=5;             //ídem. Además, no es necesariamente igual al número de productores
int vector_prod[num_productores];

///////////////////////////////////////////////////////////////////////////////////////

//Funciones comunes a ambas implementaciones (LIFO y FIFO)
template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

int producir_dato(int np) //Recibe el número de la hebra que lo invoca
{
   int contador = vector_prod[np]+np*num_items/num_productores ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "producido: " << contador << endl << flush;
   mtx.unlock();
   vector_prod[np]++;
   cont_prod[contador] ++ ;
   return contador ;
}

void consumir_dato( unsigned dato )
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

///////////////////////////////////////////////////////////////////////////////////////

//Monitor
class ProdconsM_SU: public HoareMonitor{
private:
  static const int num_celdas_total=10;
  int buffer[num_celdas_total];
  int primera_libre;   //LIFO y FIFO
  int primera_ocupada; //FIFO
  int val_n;           //Valor de n (celdas ocupadas) que, en la versión FIFO, no nos proporcionan primera_libre y primera_ocupada

  CondVar ocupadas, libres;
public:
  ProdconsM_SU();
  int leer();
  void escribir(int valor);
};

///////////////////////////////////////////////////////////////////////////////////////

//Constructor del monitor
ProdconsM_SU::ProdconsM_SU(){
  primera_libre=0;
  primera_ocupada=0;
  val_n=0;
  ocupadas=newCondVar();
  libres=newCondVar();
}

///////////////////////////////////////////////////////////////////////////////////////

//Función leer
int ProdconsM_SU::leer(){
  // esperar bloqueado hasta que 0 < num_celdas_ocupadas
  while ( val_n == 0 ) //Sustituir if por while
     ocupadas.wait();

   int valor=buffer[primera_ocupada];
   primera_ocupada++;
   primera_ocupada=primera_ocupada%num_celdas_total;
   val_n--;

  // señalar al productor que hay un hueco libre, por si está esperando
  libres.signal();

  // devolver valor
  return valor ;
}

//Función escribir
void ProdconsM_SU::escribir(int valor){
  // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total

  while ( primera_libre == num_celdas_total ) //Sustituir if por while
     libres.wait();

  assert( primera_libre < num_celdas_total );

  // hacer la operación de inserción, actualizando estado del monitor
  buffer[primera_libre] = valor ;
  primera_libre++ ;
  primera_libre=primera_libre%num_celdas_total;
  val_n++;

  // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
  ocupadas.signal();
}

///////////////////////////////////////////////////////////////////////////////////////

//Funciones de las hebras
void funcion_hebra_productora( MRef<ProdconsM_SU> monitor, int np )
{
   for( unsigned i = 0 ; i < num_items/num_productores ; i++ )
   {
      int valor = producir_dato(np) ;
      cout<<"Producido -> "<<valor<<" productor nº -> "<<np<<endl;
      monitor->escribir( valor );
   }
}

void funcion_hebra_consumidora( MRef<ProdconsM_SU> monitor, int np )
{
   for( unsigned i = 0 ; i < num_items/num_consumidores ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}

///////////////////////////////////////////////////////////////////////////////////////

//Función principal main
int main(){
  //Monitor
  MRef<ProdconsM_SU> ref=Create<ProdconsM_SU>();

  //Creo y lanzo las hebras
  for(int i=0; i<num_productores; i++){
    vector_prod[i]=0;
  }

  thread hebra_productora[num_productores];
  for(int i=0; i<num_productores; i++){
    hebra_productora[i]=thread (funcion_hebra_productora, ref, i);
  }

  thread hebra_consumidora[num_consumidores];
  for(int i=0; i<num_consumidores; i++){
    hebra_consumidora[i]=thread (funcion_hebra_consumidora, ref, i);
  }

  //Espero a que terminen las hebras
  for(int i=0; i<num_consumidores; i++){
    hebra_consumidora[i].join();
  }

  for(int i=0; i<num_productores; i++){
    hebra_productora[i].join();
  }

  //Comprobar solución
  test_contadores();
}
