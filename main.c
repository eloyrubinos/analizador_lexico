#include <stdio.h>
#include <stdlib.h>
#include "AnalizadorSintactico.h"
#include "SistemaDeEntrada.h"
#include "TablaSimbolos.h"

/*
 * 
 */
int main() {

    /* Inicialización de todo el sistema. */
    iniciarSE("concurrentSum.go"); // Inicializo el sistema de entrada.
    iniciarTabla(); // Inicializo la tabla de símbolos.

    /* Ejecución del analizador sintáctico. */
    analizar(); // Ejecuto el analizador sintáctico.
    imprimirTabla(); // Imprimo la tabla de símbolos.

    /* Cierre y limpieza del sistema. */
    cerrarSE(); // Libero memoria del sistema de entrada.
    eliminarTabla(); // Libero memoria de la tabla de símbolos.

    return (EXIT_SUCCESS);
}