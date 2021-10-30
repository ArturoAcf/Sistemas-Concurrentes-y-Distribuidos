// -----------------------------------------------------------------------------
// Nombre: Arturo Alonso Carbonero - DNI:75936665-A / Grupo: 2ºC - C1
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-interb.cpp
// Implementación del problema de los filósofos (sin camarero).
// Plantilla completada - solución con interbloqueo.
//
// Compilar -> mpicxx -std=c++11 filosofos-interb.cpp -o filosofos-interb
// Ejecutar -> mpirun -np 10 ./filosofos-interb (Si no se indica "-np 10" mostrará un mensaje de error)
//
// NOTA -> Al final de este fichero se incluye un comentario con un ejemplo de la salida del programa
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5,
   num_procesos  = 2*num_filosofos ;


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_procesos, // id. tenedor izq.
      id_ten_der = (id+num_procesos-1) % num_procesos, // id. tenedor der.
      pedir;                                           // Petición

  while ( true )
  {
    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    // ... solicitar tenedor izquierdo (completar)
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD); // Solución

    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    // ... solicitar tenedor derecho (completar)
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD); // Solución

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    // ... soltar el tenedor izquierdo (completar)
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD); // Solución

    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    // ... soltar el tenedor derecho (completar)
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD); // Solución

    cout << "Filosofo " << id << " comienza a pensar... " << endl;
 }
}
// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     // ...... recibir petición de cualquier filósofo (completar)
     MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado); // Solución
     // ...... guardar en 'id_filosofo' el id. del emisor (completar)
     id_filosofo = estado.MPI_SOURCE; // Solución
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     // ...... recibir liberación de filósofo 'id_filosofo' (completar)
     MPI_Recv(&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado); // Solución
     cout <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}
// ---------------------------------------------------------------------

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "El número de procesos esperados es:    " << num_procesos << endl
             << "El número de procesos en ejecución es: " << num_procesos_actual << endl
             << "Añada -np n (número de procesos esperados) a la ejecución" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------

// Ejemplo de salida del programa (directamente copiado de la terminal)

/*
Ten. 1 ha sido cogido por filo. 2
Filósofo 2 solicita ten. izq.3
Filósofo 2 solicita ten. der.1
Filósofo 2 comienza a comer
Ten. 3 ha sido cogido por filo. 2
Filósofo 4 solicita ten. izq.5
Filósofo 6 solicita ten. izq.7
Filósofo 0 solicita ten. izq.1
Filósofo 6 solicita ten. der.5
Ten. 7 ha sido cogido por filo. 6
Filósofo 4 solicita ten. der.3
Ten. 5 ha sido cogido por filo. 4
Filósofo 8 solicita ten. izq.9
Filósofo 8 solicita ten. der.7
Ten. 9 ha sido cogido por filo. 8
Filósofo 2 suelta ten. izq. 3
Ten. 3 ha sido liberado por filo. 2
Ten. 3 ha sido cogido por filo. 4
Filósofo 4 comienza a comer
Ten. 1 ha sido liberado por filo. 2

...

*/
