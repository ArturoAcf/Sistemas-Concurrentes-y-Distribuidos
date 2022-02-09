// -----------------------------------------------------------------------------
// Nombre: Arturo Alonso Carbonero - Grupo: 2ºC - C1
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-cam.cpp
// Implementación del problema de los filósofos (con camarero).

// Compilar -> mpicxx -std=c++11 filosofos-cam.cpp -o filosofos-cam
// Ejecutar -> mpirun -np 11 ./filosofos-cam (Si no se indica "-np 11" mostrará un mensaje de error)
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
   num_procesos  = 2*num_filosofos,
   num_procesos_esperados = num_procesos + 1, // Ya que hay un camarero, se espera un proceso más
   id_camarero = num_procesos;

// Etiquetas
const int
   etiq_sentar = 0,
   etiq_levantar = 1;

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
    cout <<"Filósofo " <<id <<"solicita permiso para poder tomar asiento" <<endl;
    MPI_Ssend(&pedir, 1, MPI_INT, id_camarero, etiq_sentar, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);
    cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<" comienza a comer" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD);
    cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
    MPI_Ssend(&pedir, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD);

    cout <<"Filósofo " <<id <<"Pide permiso para poder levantarse" <<endl;
    MPI_Ssend(&pedir, 1, MPI_INT, id_camarero, etiq_levantar, MPI_COMM_WORLD);

    cout << "Filosofo " << id << " comienza a pensar... " << endl;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
  }
}
// ---------------------------------------------------------------------

void funcion_camarero(){ // Es similar en cierta medida a la función del buffer de prodcons
  int fil_sentados = 0,
      pedir,
      id_filosofo,
      etiq_aceptable;
  MPI_Status estado;

  while(true){
    // Determinar qué tipo de petición acepta de los filósofos
    if(fil_sentados < num_filosofos - 1){
      etiq_aceptable = MPI_ANY_TAG;
    }else{
      etiq_aceptable = etiq_levantar;
    }

    // Recibio de petición
    MPI_Recv(&pedir, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado);
    id_filosofo = estado.MPI_SOURCE;

    // Procesar mensaje recibido
    switch(estado.MPI_TAG)
    {
      case etiq_levantar:
        cout <<"\tFilósofo " <<id_filosofo <<" se levanta de la mesa" <<endl;
        fil_sentados--;
        break;

      case etiq_sentar:
        cout <<"\tFilósfo " <<id_filosofo <<" toma asiento en la mesa" <<endl;
        fil_sentados++;
        break;
    }

    cout <<"\n- - -Número de filósofos sentados -> " <<fil_sentados <<endl;
  }
}

// ---------------------------------------------------------------------

void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true )
  {
     MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado); // Solución
     id_filosofo = estado.MPI_SOURCE;
     cout <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     MPI_Recv(&valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado);
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


   if ( num_procesos_esperados == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if( id_propio == id_camarero){
         funcion_camarero();
      }else if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "El número de procesos esperados es:    " << num_procesos_esperados << endl
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
Filósofo 0solicita permiso para poder tomar asiento
Filósofo 0 solicita ten. izq.1
Filósofo 2solicita permiso para poder tomar asiento
Filósofo 4solicita permiso para poder tomar asiento
Filósofo 4 solicita ten. izq.5
Filósofo 6solicita permiso para poder tomar asiento
Filósofo 6 solicita ten. izq.7
Filósofo 8solicita permiso para poder tomar asiento
Filósofo 8 solicita ten. izq.9
        Filósfo 0 toma asiento en la mesa

- - -Número de filósofos sentados -> 1
        Filósfo 4 toma asiento en la mesa

- - -Número de filósofos sentados -> 2
        Filósfo 6 toma asiento en la mesa

- - -Número de filósofos sentados -> 3
        Filósfo 8 toma asiento en la mesa

- - -Número de filósofos sentados -> 4
Filósofo 0 solicita ten. der.9
Ten. 1 ha sido cogido por filo. 0

...

*/
