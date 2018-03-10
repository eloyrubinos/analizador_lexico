/* Estructura para almacenar los lexemas y qué componente léxico son y pasárselos a la tabla de símbolos y al sintáctico para imprimirlos. */
typedef struct {
    short tipo;
    char* lexema;
}compLex;

/* Función principal del analizador léxico que itera sobre las distintas comprobaciones hasta encontrar un componente léxico válido o llegar a un estado de error. 
  Devuelve 1 si se puede seguir devolviendo o 0 si ha reconocido el EOF.*/
short siguiente_Lexema(compLex *lex);