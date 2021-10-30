// -----------------------------------------------------------------------------
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
// Implementación de ejecutivo cíclico
//
// Nombre: Arturo Alonso Carbonero
// DNI: 75936665-A
// Grupo: 2ºC - C1
//
//   Planificación:
//   ------------
//   Ta.  T    C
//   ------------
//   A  500   100
//   B  500   150
//   C  1000  200
//   D  2000  240
//  -------------
//
//  Tm = mcm(500,500,1000,2000) = 2000
//
//  A y B deben aparece 4 veces, C debe aparecer 2 veces y D solo una.
//
//  Planificación (con Ts == 500 ms)
//  *---------*------------*----------*
//  | A B C  | A B D  | A B C  | A B |
//  *---------*------------*----------*
//
// -----------------------------------------------------------------------------
/*
  -Pregunta 1: mínimo tiempo: 500ms.
  -Pregunta 2: Sí seguiría siendo planificable.
               100/500 + 150/500 + 200/1000 + 250/2000 = 1650/2000 = 33/40 = 0.825
               0.825 <= 1   =>   Es planificable.
*/

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", milliseconds(100) );  }
void TareaB() { Tarea( "B", milliseconds(150) );  }
void TareaC() { Tarea( "C", milliseconds(200) );  }
void TareaD() { Tarea( "D", milliseconds(240) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario
   const milliseconds Ts( 500 );
   const milliseconds retraso(20); // Variable con el máximo retraso permitido.

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaA(); TareaB(); TareaC();    break ;
            case 2 : TareaA(); TareaB(); TareaD();    break ;
            case 3 : TareaA(); TareaB(); TareaC();    break ;
            case 4 : TareaA(); TareaB();              break ;
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );

         // Cálculo y comparación del retraso.
         time_point<steady_clock> fin = steady_clock::now();
         steady_clock::duration duracion = fin-ini_sec;

         if(milliseconds_f(duracion).count() > milliseconds_f(retraso).count()){ // Si se supera 20ms, se termina (no suele ser demasiado).
           cout << "Error -> El retraso(" << milliseconds_f(duracion).count() << ") es mayor que " << milliseconds_f(retraso).count() << endl;
           exit(0);
         }
      }
   }
}
