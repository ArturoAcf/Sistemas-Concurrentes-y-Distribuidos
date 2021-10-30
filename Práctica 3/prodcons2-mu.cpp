// -----------------------------------------------------------------------------
// Nombre: Arturo Alonso Carbonero - DNI:75936665-A / Grupo: 2ºC - C1
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodconsMPI.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que gestiona un buffer finito y recibe peticiones
// de varios productores y consumidores
//
// Compilar -> mpicxx -std=c++11 prodcons2-mu.cpp -o prodcons2-mu
// Ejecutar -> mpirun -np 10 ./prodcons2-mu (Si no se indica "-np 10" mostrará un mensaje de error)
//
// NOTA -> Al final de este fichero se incluye un comentario con un ejemplo de la salida del programa
// NOTA -> Puesto que el examen es sobre la práctica, es posible que algunos comentarios no concuerden con el contenido del mismo.
// -----------------------------------------------------------------------------

#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   np = 4,
   nc = 5,
   num_procesos_esperado = np + nc + 1,
   num_items = 120,
   tam_vector = 40,
   items_prod = num_items / np,
   items_cons = num_items / nc,
   id_buffer = np; // Debe ser el primer id directamente después del último productor

// Etiquetas
const int
   etiq_prod = 0,
   etiq_cons = 1;

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
// ptoducir produce los numeros en secuencia (1,2,3,....)
// y lleva espera aleatorio
int producir(int id_prod)
{
   static int contador = items_prod * id_prod ;
   sleep_for( milliseconds( aleatorio<10,100>()) );
   contador++ ;
   cout << "Productor " << id_prod << " ha producido valor " << contador << endl << flush;
   return contador ;
}
// ---------------------------------------------------------------------

void funcion_productor(int id_prod)
{
   for ( unsigned int i= 0 ; i < items_prod ; i++ )
   {
      // producir valor
      int valor_prod = producir(id_prod);
      // enviar valor
      cout << "Productor " << id_prod << " va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_prod, MPI_COMM_WORLD );
   }
}
// ---------------------------------------------------------------------

void consumir( int valor_cons, int id_cons )
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<110,200>()) );
   cout << "Consumidor " << id_cons << " ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor(int id_cons)
{
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < items_cons; i++ )
   {
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_cons, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_prod, MPI_COMM_WORLD, &estado );
      cout << "Consumidor " << id_cons << " ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec, id_cons );
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              etiq_aceptable ;           // etiqueta de emisor aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido

   for( unsigned int i=0 ; i < num_items*2 ; i++ )
   {
      // 1. determinar si puede enviar solo prod., solo cons, o todos

      if ( num_celdas_ocupadas == 0 )               // si buffer vacío
         etiq_aceptable = etiq_prod ;       // $~~~$ solo prod.
      else if ( num_celdas_ocupadas == tam_vector ) // si buffer lleno
         etiq_aceptable = etiq_cons ;      // $~~~$ solo cons.
      else                                          // si no vacío ni lleno
         etiq_aceptable = MPI_ANY_TAG ;     // $~~~$ cualquiera

      // 2. recibir un mensaje del emisor o emisores aceptables

      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado );

      // 3. procesar el mensaje recibido

      switch( estado.MPI_TAG ) // leer emisor del mensaje en metadatos
      {
         case etiq_prod: // si ha sido el productor: insertar en buffer
            buffer[primera_libre] = valor ;
            primera_libre = (primera_libre+1) % tam_vector ;
            num_celdas_ocupadas++ ;
            cout << "Buffer ha recibido valor " << valor << endl ;
            break;

         case etiq_cons: // si ha sido el consumidor: extraer y enviarle
            valor = buffer[primera_ocupada] ;
            primera_ocupada = (primera_ocupada+1) % tam_vector ;
            num_celdas_ocupadas-- ;
            cout << "Buffer va a enviar valor " << valor << endl ;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_prod, MPI_COMM_WORLD);
            break;
      }
   }
}

// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{
   int id_propio, num_procesos_actual;

   // Comprobar si es válido num_items
   if(num_items % np != 0 || num_items % nc != 0){
     cout<<"\nError, número de items no válido"<<endl;
     return 1;
   }

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual )
   {
      // ejecutar la operación apropiada a 'id_propio'
      // Para este ejercicio:
      // Prodcutor  ->  0, 1, ..., id_buffer - 1
      // Buffer     ->  id_buffer
      // Consumidor ->  id_buffer + 1, id_buffer + 2, ..., id_buffer + nc - 1
      if ( id_propio < id_buffer )
         funcion_productor(id_propio);
      else if ( id_propio == id_buffer )
         funcion_buffer();
      else
         funcion_consumidor(id_propio);
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "El número de procesos esperados es:    " << num_procesos_esperado << endl
             << "El número de procesos en ejecución es: " << num_procesos_actual << endl
             << "Añada -np n (número de procesos esperados) a la ejecución" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}

// Ejemplo de salida del programa (directamente copiado de la terminal)

/*
Productor 0 ha producido valor 1
Productor 0 va a enviar valor 1
Productor 1 ha producido valor 31
Productor 1 va a enviar valor 31
Buffer ha recibido valor 1
Buffer va a enviar valor 1
Buffer ha recibido valor 31
Buffer va a enviar valor 31
Consumidor 5 ha recibido valor 1
Consumidor 7 ha recibido valor 31
Productor 2 ha producido valor 61
Productor 2 va a enviar valor 61
Buffer ha recibido valor 61
Buffer va a enviar valor 61
Consumidor 6 ha recibido valor 61
Productor 3 ha producido valor 91
Productor 3 va a enviar valor 91
Buffer ha recibido valor 91
Buffer va a enviar valor 91
Consumidor 9 ha recibido valor 91

...

*/
