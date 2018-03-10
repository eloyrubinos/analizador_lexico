/* Función que inicializa el sistema de entrada: pone los punteros a 0 y carga el primer centinela. */
void iniciarSE(char* dir);
/* Función que realiza las operaciones necesarias para cerrar de forma segura el sistema de entrada. */
void cerrarSE();
/* Función que me devuelve el siguiente carácter en el buffer. */
char siguiente_Caracter();
/* Función que retrocede el puntero de lectura del buffer. */
void devolver_Caracter();
/* Función que actualiza el puntero de inicio del buffer. */
void aceptar_Lexema();