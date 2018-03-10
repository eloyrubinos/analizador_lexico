#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "gestionErrores.h"
#define N 64 // Tamaño de cada centinela del doble buffer. Debería ser un múltilplo de la unidad de asignación, en este caso 4096 bytes, para maximizar la eficiencia.
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0])) // Macro para obtener el número de elementos de un vector estático.

/* Estructura que implementa un buffer centinela. */
typedef struct{
    char almacen[N+1]; // Vector de caracteres de tamaño estático determinado por N y el símbolo "EOF" que se inserta al final (de ahí el +1).
    short inicio; // Puntero al inicio del lexema.
    short lectura; // Puntero al carácter actual.
}centinela;

/* Estructura que implementa el doble buffer centinela. */
typedef struct{
    centinela A;
    centinela B;
}buffer;

/* Variables globales del sistema de entrada. */
FILE *fuente; // Variable para el código fuente a compilar.
buffer buf; // Variable para el doble buffer centinela.
long pos = 0L; // Variable que me indica en qué posición del fichero me encuentro leyendo, inicialmente 0.
short activo = 0;  // Indicador de centinela activo. 0 = A, 1 = B.
short cargar = 1; // Indicador que me dice si tengo que cargar un bloque nuevo en el buffer o no.
                  // Uso esta variable para el caso en que tengo que devolver un carácter que estaba en el buffer anterior. La pongo a 0 para no pisar todo lo que aún no he leído al volver a cambiar de buffer.
int bytes = N; // Variable en la que voy a almacenar los bytes devueltos por fread en cada lectura. Relevante para el fin de fichero.

/* Función que se asegura de que los punteros a posición del buffer están a 0 antes de empezar a usarlos. */
void creaBuffer(){
    buf.A.inicio = 0;
    buf.A.lectura = 0;
    buf.B.inicio = 0;
    buf.B.lectura = 0;
    activo = 0; // Me aseguro de que cada vez que se inicializa el sistema de entrada, A es el buffer activo.
}

/* Función que inicializa el sistema de entrada. */
void iniciarSE(char* dir){ // Recibe como entrada la dirección del fichero a leer..
    creaBuffer(buf); // Preparo el buffer.
    
    if((fuente = fopen(dir, "r")) == NULL){ // Intentamos abrir el archivo que pasa como argumento desde el main, en modo solo lectura.
        errorIO("\nFallo al abrir el archivo.\n"); // Si hay algún problema aplicamos gestión de errores.
    } 
    else{ // Si se abre con éxito:
        // Lleno el primer centinela (A).
        fseek(fuente, pos, SEEK_SET); // Me coloco en el lugar correcto del fichero para empezar o seguir leyendo (pos inicialmente es 0).
        fread(buf.A.almacen, 1, NELEMS(buf.A.almacen)-1, fuente); // Lleno el centinela A, dejando sitio para agregar un EOF.
        buf.A.almacen[NELEMS(buf.A.almacen)-1] = EOF; // Agrego el EOF al final del centinela.
        if(ferror(fuente)){ // Compruebo si ha habido errores en la lectura.
            errorIO("\nFallo leyendo del archivo al centinela A.\n"); // En caso de que sí aplico gestión de errores.
        }
        else{ // Si todo ha ido bien:
            fflush(fuente); // Fuerzo el buffer de C a memoria para asegurarme de que ftell funciona correctamente.
            pos = ftell(fuente); // Guardo la posición actual en el fichero (tras la lectura).
        }
    }
}

/* Función que carga más caracteres en el centinela del que toca leer cuando se llega al EOF artificial. Es decir, si termina A, recargamos B, y si termina B recargamos A. */
void rellenarCentinela(){
    
    if(activo == 0){ // Si A activo:
        fseek(fuente, pos, SEEK_SET); // Me coloco en el lugar correcto del fichero para seguir leyendo.
        bytes = fread(buf.B.almacen, 1, NELEMS(buf.B.almacen)-1, fuente); // Lleno el centinela, dejando sitio para agregar un EOF.
        buf.B.almacen[bytes] = EOF; // Agrego el EOF al final de los bytes leídos por el fread.
        if(ferror(fuente)){ // Compruebo si ha habido errores en la lectura.
            errorIO("\nFallo rellenando el centinela.\n"); // En casoa firmativo aplico gestión de errores.
        } 
        else{ // Si todo ha ido bien:
            fflush(fuente); // Fuerzo el buffer de C a memoria para asegurarme de que ftell funciona correctamente.
            pos = ftell(fuente); // Guardo la posición actual en el fichero (tras la lectura).
            buf.B.inicio = 0; // Reinicio el puntero de inicio.
            buf.B.lectura = 0; // Reinicio el puntero a la posición de lectura.
        }
    }
    /* Fragmento de código equivalente para el centinela B. */
    else if(activo == 1){
        fseek(fuente, pos, SEEK_SET);
        bytes = fread(buf.A.almacen, 1, NELEMS(buf.A.almacen)-1, fuente);
        buf.A.almacen[bytes] = EOF;
        if(ferror(fuente)){
            errorIO("\nFallo rellenando el centinela.\n"); 
        } 
        else{
            fflush(fuente);
            pos = ftell(fuente);
            buf.A.inicio = 0;
            buf.A.lectura = 0;
        }
    }
}

/* Función para cambiar el indicador de buffer activo. Recordemos que 0 = A, 1 = B. */
void switchActivo(){
    if(activo == 0) activo = 1; // Si A está activo, marco B como activo
    else if(activo == 1) activo = 0; // Si B está activo, marco A como activo
    else errorIO("\nError al identificar el buffer activo.\n");
}

/* Función para cerrar el sistema de entrada de forma correcta. */
void cerrarSE(){
    fclose(fuente); // Cierro el fichero.
}

/* Función que devuelve el siguiente carácter del buffer. */
char siguiente_Caracter(){
    char caracter; // Variable para almacenar el carácter leído.
    
    if(activo == 0){ // Si está activo el centinela A:
        if((caracter = buf.A.almacen[buf.A.lectura]) == EOF){ // Extraigo el carácter que toca y si es un EOF:
            if(!feof(fuente)){ // Si NO he llegado al fin de fichero, significa que he llegado al final del centinela A:
                if(cargar) rellenarCentinela(); // Así que relleno B con los siguientes caracteres del fichero según corresponda, si cargar == 1 (si no había retrocedido a este centinela desde el siguiente).
                else cargar = 1; // En caso de que, en efecto, no deba rellenar el centinela siguiente, vuelvo a poner cargar == 0.
                switchActivo(); // Cambio el centinela activo.
                caracter = siguiente_Caracter(); // Y vuelvo a llamar a esta misma función, porque sigo teniendo que devolver el siguiente carácter.
            }
            // En caso de haber lleagdo al fin de fichero, devuelvo el EOF.
            else buf.A.lectura++; // Avanzo el puntero de lectura.
        }
        // Si el carácter extraído NO es un EOF:
        else buf.A.lectura++; // Avanzo el puntero de lectura y lo devuelvo.
    }
    /* Fragmenmto de código equivalente para el centinela B. */
    else if(activo == 1){
        if((caracter = buf.B.almacen[buf.B.lectura]) == EOF){
            if(!feof(fuente)){
                if(cargar) rellenarCentinela();
                else cargar = 1;
                switchActivo();
                caracter = siguiente_Caracter();
            }
            else buf.B.lectura++;
        }
        else buf.B.lectura++;
    }
    
    return (caracter); // Devuelvo el carácter extraído.
}

/* Función que retrocede el puntero de lectura una posición, por si hay que volver a leer un carácter. */
void devolver_Caracter(){
    // Primero compruebo cuál es el centinela activo en el momento de la devolución.
    if(activo == 0){
        if(buf.A.lectura == 0){ // Si estaba en la primera posición del centinela, el retroceso me lleva al centinela anterior y por tanto:
            switchActivo(); // Cambio el centinela activo.
            buf.B.lectura--; // Retrocedo el puntero de lectura del centinela anterior para que no apunte al EOF.
            cargar = 0; // Pongo cargar a 0 para no pisar el centinela del que vuelvo cuando vuelva a ir a él.
        }
        else buf.A.lectura--; // Si no estaba en la primera posición del centinela, retrocedo sin más.
    }
    /* Fragmento de código equivalente para el caso en que el centinela activo en el momento de la devolución es el B. */
    else if(activo == 1){
        if(buf.B.lectura == 0){
            switchActivo();
            buf.A.lectura--;
            cargar = 0;
        }
        else buf.B.lectura--;
    }
    else errorIO("\nError al identificar el buffer activo.\n");
}

/* Función que avisa al centinela de que se ha reconocido un lexema válido, para que avance su puntero de inicio al carácter que toca leer, que será el inicio de otro (potencialmente). */
void aceptar_Lexema(){
    if(activo == 0) buf.A.inicio = buf.A.lectura;
    else if(activo == 1) buf.B.inicio = buf.B.lectura;
    else errorIO("\nError al identificar el buffer activo.\n");
}
