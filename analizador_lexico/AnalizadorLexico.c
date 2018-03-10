#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SistemaDeEntrada.h"
#include "Definiciones.h"
#include "TablaSimbolos.h"
#include "gestionErrores.h"

/* Como voy almacenando todo lo devuelto por el sistema de entrada, necesito unos criterios para alojar y ampliar memoria del char* en el que guardo las cadenas (en adelante, por simplificar, referidas como "lexema" incluso sin estar terminadas). */
#define _INCREMENTO 8 // Cantidad de memoria que amplío de cada vez.
#define _MEMINICIAL 8 // Cantidad de memoria que reservo inicialmente.

/* Estructura para almacenar los lexemas y qué componente léxico son y pasárselos a la tabla de símbolos y al sintáctico para imprimirlos. */
typedef struct {
    short tipo;
    char* lexema;
}compLex;

int linea = 1; // Variable que me indica en qué línea del fichero estoy leyendo, empezando en 1 para ser más legible a nivel de usuario.
int memoria; // Variable en que voy actualizando el tamaño actual del lexema.

/* Función para inicializar el compLex. */
void creaLex(compLex *lex){
    memoria = _MEMINICIAL; // Pongo memoria al tamaño base.
    if(lex->lexema != NULL){ // Si no es nulo:
        free(lex->lexema); // Libero la memoria apuntada,
        lex->lexema = NULL;; // y quito el puntero.
    }
    lex->lexema = (char*) malloc(memoria * sizeof(char)); // Reservo memoria para el lexema.
    memset(lex->lexema, 0, memoria); // Vacío la memoria de lo que tuviese antes de la reserva.
    lex->tipo = -1; // Pongo el componente léxico a -1 a modo de indicador (si veo un -1 impreso en algún sitio sé que ha habido algún error).
}

/* Función para guardar un carácter en el lexema, comprobando si he llegado al límite de espacio. */
void guardar(compLex *lex, char c){
    strncat(lex->lexema, &c, 1);
    if(memoria == strlen(lex->lexema)+1){ // Si he llegado al límite:
        memoria += _INCREMENTO; // Amplío la memoria actual en base al incremento,
        lex->lexema = (char*) realloc(lex->lexema, memoria * sizeof(char)); // y realojo el lexema con más memoria.
    }
}

/* Función principal del analizador léxico que itera sobre las distintas comprobaciones hasta encontrar un componente léxico válido o llegar a un estado de error. */
short siguiente_Lexema(compLex *lex){
    short encontrado = 0; // Variable que uso para avisar de si he encontrado una equivalencia.
    short estado; // Variable que devuelve la función, para determinar el estado del análisis: '1' significa que debe continuar, '0' significa que he encontrado el EOF del fichero y por tanto el análisis ha terminado.
    char new; // Variable en la que voy cargando cada carácter que leo del sistema de entrada.
    
    // Mientras no encuentre un "autómata" que satisfaga la cadena:
    while(!encontrado){
        
        // Inicializo este valor con cada pasada.
        estado = 1;
        
        creaLex(lex);
        
        new = siguiente_Caracter(); // Extraigo primer carácter de la entrada.
        guardar(lex, new); // Lo guardo.
        
        // Si empieza por un número -> voy a identificar si es un entero, real o imaginario.
        if(isdigit(new)){
            while(isdigit(new)){ // Mientras reciba números ->
                new = siguiente_Caracter(); // Extraigo caracteres.
                guardar(lex, new); // Los guardo en el lexema.
                if(lex->lexema[0] == '0' && isdigit(new) && new > '7'){ // Si empieza por 0 y me paso de 7 en algún dígito, era un número octal. Tengo que volver a comprobar si es un número con isdigit por si acabo de leer otro símbolo que pueda tener un código ascii superior a 7 y no sea un número.
                    devolver_Caracter(); // Devuelvo el último carácter al sistema de entrada (el que ya no es un dígito octal).
                    lex->lexema[strlen(lex->lexema)-1] = '\0'; // Borro el último carácter del lexema.
                    aceptar_Lexema(); // Acepto el lexema.
                    lex->tipo = _ENTERO; // Como número entero.
                    encontrado++; // Aviso de que he encontrado un lexema válido.
                    break; // Dejo de leer números en este bucle while, porque si hubiese más números seguidos formarían parte de un nuevo componente léxico.
                }
            }
            if(encontrado) continue; // Si he encontrado un octal, entonces no tiene sentido seguir procesando este autómata. Salgo del while (vuelvo al principio, pero como "encontrado" ahora es > 0, saldré del while).
            
            /* Si he ejecutado el "conitnue" de aquí arriba significa que he encontrado un número octal y he devuelto el último carácter leído.
             * Si por el contrario he llegado aquí, significa que por ahora he leído una secuencia de números enteros.
              Según lo que reciba a continuación, podría determinar que se trata de un hexadecimal, un real, un imaginario o que ha terminado. */
            
            // Si me encuentro un punto, se trata de un número real bien construido, pero todavía puede haber más números, una 'e' o una 'i'.
            if(new == '.'){
                new = siguiente_Caracter();
                guardar(lex, new);
                // Si es un número, continúo si encuentro más números, una 'e' o una 'i'.
                if(isdigit(new)){ // Si encuentro otro número:
                    // Mientras reciba más números.
                    while(isdigit(new)){
                        new = siguiente_Caracter();
                        guardar(lex, new);
                    }
                    // Después de recibir números tras un '.' solo puedo encontrar una 'e' o una 'i' para poder seguir añadiendo al lexema actual.
                    if(new == 'e' || new == 'E'){ // Si me encuentro una 'e', puede haber números o '+' o '-'.
                        new = siguiente_Caracter();
                        guardar(lex, new);
                        // Si me encuentro un '+' o '-', solo puede haber números.
                        if(new == '+' || new == '-'){
                            new = siguiente_Caracter();
                            guardar(lex, new);
                            // Si me encuentro un número, solo puede haber más números o 'i'.
                            if(isdigit(new)){
                                while(isdigit(new)){
                                    new = siguiente_Caracter();
                                    guardar(lex, new);
                                }
                                // Si me encuentro una i.
                                if(new == 'i'){
                                    aceptar_Lexema(); // Acepto el lexema.
                                    lex->tipo = _IMAGINARIO; // Como número imaginario.
                                    encontrado++; // Aviso de que he encontrado un lexema válido.
                                }
                                
                                /* En este punto acabo de leer un número imaginario de la forma xxxx.xxE+xxxi */
                                
                                // Sin embargo si no me encuentro una 'i', sino cualquier otra cosa no numérica, entonces es un número real.
                                else{ 
                                    devolver_Caracter();
                                    lex->lexema[strlen(lex->lexema)-1] = '\0';
                                    aceptar_Lexema();
                                    lex->tipo = _REAL;
                                    encontrado++;
                                }
                                
                                /* En este punto acabo de leer un número real de la forma xxxx.xxE+xxx */
                                
                                continue; // Vuelvo al inicio del while, lo que me sacará del bucle al ser encontrado != 0.
                            }
                            
                            /* Si después de la sucesión de 'E' y '+'/'-' no hay un número, retrocedo tres caracteres 
                             * ya que el número había terminado antes de la 'E', y reconozco dicho número. */
                            
                            else{
                                devolver_Caracter();
                                devolver_Caracter();
                                devolver_Caracter(); // Devuelvo los tres últimos caracteres por lo antes expuesto: el recién leído que no era número, el 'x'/'-' y el 'E'.
                                lex->lexema[strlen(lex->lexema)-3] = '\0'; // Borro los tres últimos caracteres.
                                
                                aceptar_Lexema();
                                lex->tipo = _REAL;
                                encontrado++;
                            }
                            continue;
                            
                            /* Estoy aceptando un real de la forma xxxx.xx */
                        }
                        
                        // Si después de la 'e' me encuentro directamente números en vez de un '+' o '-'.
                        else if(isdigit(new)){
                            while(isdigit(new)){
                                new = siguiente_Caracter();
                                guardar(lex, new);
                            }
                            
                            // Si me encuentro una 'i'.
                            if(new == 'i'){
                                aceptar_Lexema();
                                lex->tipo = _IMAGINARIO;
                                encontrado++;
                            }
                            
                            /* Acabo de aceptar un imaginario de la forma xxxx.xxExxxi */
                            
                            else{ // Si no me encuentro una 'i', sino cualquier otra cosa, entonces es un número real.
                                devolver_Caracter();
                                lex->lexema[strlen(lex->lexema)-1] = '\0';
                                aceptar_Lexema();
                                lex->tipo = _REAL;
                                encontrado++;
                            }
                            continue;
                            
                            /* Acabo de aceptar un real del tipo xxxx.xxExxx */
                        }
                        
                        // Si después de la 'E' no tengo ni un '+'/'-' ni un número, retrocedo dos caracteres ya que el número había acabado antes de la 'E'
                        else{
                            devolver_Caracter();
                            devolver_Caracter(); // Devuelvo los dos últimos caracteres por lo antes expuesto: el que no es ni '+'/'-' y 'E'.
                            lex->lexema[strlen(lex->lexema)-2] = '\0'; // Borro los dos últimos caracteres.
                            aceptar_Lexema();
                            lex->tipo = _REAL;
                            encontrado++;
                        }
                        continue;
                        
                        /* Acabo de aceptar un real del tipo xxxx.xx */
                    }
                    // Si después de los dígitos posteriores al '.' me encuentro una 'i', es imaginario.
                    else if(new == 'i'){
                        aceptar_Lexema();
                        lex->tipo = _IMAGINARIO;
                        encontrado++;
                    }
                    
                    /* Acabo de aceptar un imaginario del tipo xxxx.xxi*/
                    
                    // Si después de los dígitos posteriores al '.' no me encuentro ni 'e' ni 'i'.
                    else{
                        devolver_Caracter();
                        lex->lexema[strlen(lex->lexema)-1] = '\0';
                        aceptar_Lexema();
                        lex->tipo = _REAL;
                        encontrado++;
                    }
                    continue;
                    
                    /* Acabo de aceptar un real del tipo xxxx.xx */
                }
                
                // Si después del punto en vez de números encuentro una 'i'.
                else if(new == 'i'){
                    aceptar_Lexema();
                    lex->tipo = _IMAGINARIO;
                    encontrado++;
                }
                
                /* Acabo de aceptar un imaginario de la forma xxxx.i */
                
                // Si después del punto me encuentro directamente una 'E', repito el mismo proceso que cuando me la encontré tras más dígitos.
                else if(new == 'e' || new == 'E'){
                    new = siguiente_Caracter();
                    guardar(lex, new);
                    // Si me encuentro un '+' o '-', solo puede haber números.
                    if(new == '+' || new == '-'){
                        new = siguiente_Caracter();
                        guardar(lex, new);
                        // Si me encuentro un número, solo puede haber más números o 'i'.
                        if(isdigit(new)){
                            while(isdigit(new)){
                                new = siguiente_Caracter();
                                guardar(lex, new);
                            }
                            // Si me encuentro una 'i'.
                            if(new == 'i'){
                                aceptar_Lexema();
                                lex->tipo = _IMAGINARIO;
                                encontrado++;
                            }
                            
                            /* Acabo de aceptar un imaginario del tipo xxxx.E+xxxi */
                            
                            else{ // Si no me encuentro una 'i', sino cualquier otra cosa, entonces es un número real.
                                devolver_Caracter();
                                lex->lexema[strlen(lex->lexema)-1] = '\0';
                                aceptar_Lexema();
                                lex->tipo = _REAL;
                                encontrado++;
                            }
                            continue;
                            
                            /* Acabo de aceptar un real del tipo xxxx.E+xxx */
                            
                        }
                        // Si después de la 'E' y el '+'/'-' no hay un número, retrocedo tres caracteres ya que el número había terminado antes de la 'E', y reconozco dicho número.
                        else{
                            devolver_Caracter();
                            devolver_Caracter();
                            devolver_Caracter(); // Devuelvo los tres últimos caracteres por lo antes expuesto
                            lex->lexema[strlen(lex->lexema)-3] = '\0'; // Borro los tres últimos caracteres
                            aceptar_Lexema();
                            lex->tipo = _REAL;
                            encontrado++;
                        }
                        continue;
                        
                        /* Acabo de aceptar un real del tipo xxxx. */
                        
                    }
                    // Si después de la 'e' me encuentro directamente números en vez de un '+' o '-'.
                    else if(isdigit(new)){
                        while(isdigit(new)){
                            new = siguiente_Caracter();
                            guardar(lex, new);
                        }
                        // Si me encuentro una i.
                        if(new == 'i'){
                            aceptar_Lexema();
                            lex->tipo = _IMAGINARIO;
                            encontrado++;
                        }
                        
                        /* Acabo de aceptar un imaginario del tipo xxxx.Exxxi */
                        
                        else{ // Si no me encuentro una 'i', sino cualquier otra cosa, entonces es un número real.
                            devolver_Caracter();
                            lex->lexema[strlen(lex->lexema)-1] = '\0';
                            aceptar_Lexema();
                            lex->tipo = _REAL;
                            encontrado++;
                        }
                        continue;
                        
                        /* Acabo de aceptar un real del tipo xxxx.Exxx */
                        
                    }
                    // Si después de la 'E' no tengo ni un '+'/'-' ni un número, retrocedo dos caracteres ya que el número había acabado antes de la 'E'.
                    else{
                        devolver_Caracter();
                        devolver_Caracter(); // Devuelvo los dos últimos caracteres por lo antes expuesto
                        lex->lexema[strlen(lex->lexema)-2] = '\0'; // Borro los dos últimos caracteres
                        aceptar_Lexema();
                        lex->tipo = _REAL;
                        encontrado++;
                    }
                    continue;
                    
                    /* Acabo de aceptar un real del tipo xxxx. */
                    
                }
                // Si tras el punto no hay ni más dígitos, ni 'E', ni 'i', termina el número.
                else{
                    devolver_Caracter();
                    lex->lexema[strlen(lex->lexema)-1] = '\0';
                    aceptar_Lexema();
                    lex->tipo = _REAL;
                    encontrado++;
                }
                continue;
                
                /* Acabo de aceptar un real del tipo xxxx. */
                
            }
            
            /* Hasta aquí he aceptado números compuestos por una cadena numérica más un punto más otras cosas.
              Ahora voy a empezar a analizar números que no tienen un punto.*/
            
            // Si no me encuentro un punto tras la primera cadena de números, puede haber una 'i', una 'x' o una 'E'.
            else if(new == 'i'){
                aceptar_Lexema();
                lex->tipo = _IMAGINARIO;
                encontrado++;
            }
            
            /* Acabo de aceptar un imaginario del tipo xxxxi */
            
            // Si me encuentro una 'x' y solo había UN dígito antes, y ese dígito era 0, entonces puede ser un hexadecimal.
            else if((new == 'x' || new == 'X') && lex->lexema[0] == '0' && strlen(lex->lexema) == 2){
                new = siguiente_Caracter();
                guardar(lex, new);
                // Mientras lea dígitos o letras de la 'A' a la 'F', estaré leyendo un hexadecimal
                if(isxdigit(new)){
                    while(isxdigit(new)){
                        new = siguiente_Caracter();
                        guardar(lex, new);
                    }
                    devolver_Caracter();
                    lex->lexema[strlen(lex->lexema)-1] = '\0';
                    aceptar_Lexema();
                    lex->tipo = _ENTERO;
                    encontrado++;
                }
                
                /*Acabo de aceptar un entero del tipo 0xA067B*/
                
                // Sin embargo si después de la 'x' no hay un dígito hexadecimal, entonces retrocedo dos caracteres y acepto el 0 como entero.
                else{
                    devolver_Caracter();
                    devolver_Caracter(); // Devuelvo el dígito no hexadecimal y la 'x', quedándome con el 0.
                    lex->lexema[strlen(lex->lexema)-2] = '\0'; // Los borro.
                    aceptar_Lexema();
                    lex->tipo = _ENTERO;
                    encontrado++;
                }
                
                /* Acabo de aceptar el entero 0. */
            }
            // Tras la primera cadena de números también puede haber una 'E', y de nuevo repito todas las posibilidades ya vistas.
            else if(new == 'e' || new == 'E'){
                new = siguiente_Caracter();
                guardar(lex, new);
                // Si me encuentro un '+' o '-', solo puede haber números.
                if(new == '+' || new == '-'){
                    new = siguiente_Caracter();
                    guardar(lex, new);
                    // Si me encuentro un número, solo puede haber más números o 'i'.
                    if(isdigit(new)){
                        while(isdigit(new)){
                            new = siguiente_Caracter();
                            guardar(lex, new);
                        }
                        // Si me encuentro una 'i'.
                        if(new == 'i'){
                            aceptar_Lexema();
                            lex->tipo = _IMAGINARIO;
                            encontrado++;
                        }
                        
                        /* Acabo de aceptar un imaginario de la forma xxxxE+xxxi */
                        
                        else{ // Si no me encuentro una 'i', sino cualquier otra cosa, entonces es un número real.
                            devolver_Caracter();
                            lex->lexema[strlen(lex->lexema)-1] = '\0';
                            aceptar_Lexema();
                            lex->tipo = _REAL;
                            encontrado++;
                        }
                        continue;
                        
                        /* Acabo de aceptar un real del tipo xxxxE+xxx */
                        
                    }
                    // Si después de la 'E' y el '+'/'-' no hay un número, retrocedo tres caracteres ya que el número había terminado antes de la 'E', y reconozco dicho número.
                    else{
                        devolver_Caracter();
                        devolver_Caracter();
                        devolver_Caracter(); // Devuelvo los tres últimos caracteres por lo antes expuesto.
                        lex->lexema[strlen(lex->lexema)-3] = '\0'; // Borro los tres últimos caracteres.
                        aceptar_Lexema();
                        lex->tipo = _ENTERO;
                        encontrado++;
                    }
                    continue;
                    
                    /* Acabo de aceptar un entero del tipo xxxx */
                    
                }
                // Si después de la 'e' me encuentro directamente números en vez de un '+' o '-'.
                else if(isdigit(new)){
                    while(isdigit(new)){
                        new = siguiente_Caracter();
                        guardar(lex, new);
                    }
                    // Si me encuentro una 'i'.
                    if(new == 'i'){
                        aceptar_Lexema();
                        lex->tipo = _IMAGINARIO;
                        encontrado++;
                    }
                    
                    /* Acabo de aceptar un imaginario del tipo xxxxExxxi */
                    
                    else{ // Si no me encuentro una 'i', sino cualquier otra cosa, entonces es un número real.
                        devolver_Caracter();
                        lex->lexema[strlen(lex->lexema)-1] = '\0';
                        aceptar_Lexema();
                        lex->tipo = _REAL;
                        encontrado++;
                    }
                    continue;
                    
                    /* Acabo de aceptar un real del tipo xxxxExxx */
                    
                }
                // Si después de la 'E' no tengo ni un '+'/'-' ni un número, retrocedo dos caracteres ya que el número había acabado antes de la 'E'.
                else{
                    devolver_Caracter();
                    devolver_Caracter(); // Devuelvo los dos últimos caracteres por lo antes expuesto.
                    lex->lexema[strlen(lex->lexema)-2] = '\0'; // Borro los dos últimos caracteres.
                    aceptar_Lexema();
                    lex->tipo = _ENTERO;
                    encontrado++;
                }
                continue;
                
                /* Acabo de aceptar un entero del tipo xxxx */
                
            }
            // Si después de uno o más números no hay ninguno de los símbolos que pueden formar parte de un número ('.', 'x', 'i', 'E'), el número ha acabado.
            else{
                devolver_Caracter(); // Devuelvo el último carácter al sistema de entrada (el que ya no es un dígito).
                lex->lexema[strlen(lex->lexema)-1] = '\0';
                aceptar_Lexema();
                lex->tipo = _ENTERO;
                encontrado++;
            }
            continue;
            
            /* Acabo de reconocer un número del tipo xxxx */
            
        }
        
        /* Hasta aquí he reconocido números que no tienen punto. Ahora voy recnocer números que empiezan por punto.*/
        
        // Si empieza por '.' puede ser un número... o un punto xD
        else if(new == '.'){
            new = siguiente_Caracter();
            guardar(lex, new);
            // Si es un número, puedo encontrar más números, una 'e' o una 'i'.
            if(isdigit(new)){
                while(isdigit(new)){
                    new = siguiente_Caracter();
                    guardar(lex, new);
                }
                // Después de recibir números tras un '.' solo puedo encontrar una 'e' o una 'i' para poder seguir añadiendo al número actual.
                if(new == 'e' || new == 'E'){ // Si me encuentro una 'e', puede haber números o '+' o '-'.
                    new = siguiente_Caracter();
                    guardar(lex, new);
                    // Si me encuentro un '+' o '-', solo puede haber números.
                    if(new == '+' || new == '-'){
                        new = siguiente_Caracter();
                        guardar(lex, new);
                        // Si me encuentro un número, solo puede haber más números o 'i'.
                        if(isdigit(new)){
                            while(isdigit(new)){
                                new = siguiente_Caracter();
                                guardar(lex, new);
                            }
                            // Si me encuentro una 'i'.
                            if(new == 'i'){
                                aceptar_Lexema();
                                lex->tipo = _IMAGINARIO;
                                encontrado++;
                            }
                            
                            /* Acabo de aceptar un imaginario del tipo .xxxxE+xxxi */
                            
                            else{ // Si no me encuentro una 'i', sino cualquier otra cosa, entonces es un número real.
                                devolver_Caracter();
                                lex->lexema[strlen(lex->lexema)-1] = '\0';
                                aceptar_Lexema();
                                lex->tipo = _REAL;
                                encontrado++;
                            }
                            continue;
                            
                            /* Acabo de leer un real del tipo .xxxxE+xxx */
                            
                        }
                        // Si después de la 'E' y el '+'/'-' no hay un número, retrocedo tres caracteres ya que el número había terminado antes de la 'E', y reconozco dicho número.
                        else{
                            devolver_Caracter();
                            devolver_Caracter();
                            devolver_Caracter(); // Devuelvo los tres últimos caracteres por lo antes expuesto.
                            lex->lexema[strlen(lex->lexema)-3] = '\0'; // Borro los tres últimos caracteres.
                            aceptar_Lexema();
                            lex->tipo = _REAL;
                            encontrado++;
                        }
                        continue;
                        
                        /* Acabo de reconocer un real del tipo .xxxx */
                        
                    }
                    // Si después de la 'e' me encuentro directamente números en vez de un '+' o '-'.
                    else if(isdigit(new)){
                        while(isdigit(new)){
                            new = siguiente_Caracter();
                            guardar(lex, new);
                        }
                        // Si me encuentro una 'i'.
                        if(new == 'i'){
                            aceptar_Lexema();
                            lex->tipo = _IMAGINARIO;
                            encontrado++;
                        }
                        
                        /* Acabo de aceptar un imaginario del tipo .xxxxExxxi */
                        
                        else{ // Si no me encuentro una 'i', sino cualquier otra cosa, entonces es un número real.
                            devolver_Caracter();
                            lex->lexema[strlen(lex->lexema)-1] = '\0';
                            aceptar_Lexema();
                            lex->tipo = _REAL;
                            encontrado++;
                        }
                        continue;
                        
                        /* Acabo de aceptar un real del tipo .xxxxExxx*/
                    }
                    // Si después de la 'E' no tengo ni un '+'/'-' ni un número, retrocedo dos caracteres ya que el número había acabado antes de la 'E'.
                    else{
                        devolver_Caracter();
                        devolver_Caracter(); // Devuelvo los dos últimos caracteres por lo antes expuesto.
                        lex->lexema[strlen(lex->lexema)-2] = '\0'; // Borro los dos últimos caracteres.
                        aceptar_Lexema();
                        lex->tipo = _REAL;
                        encontrado++;
                    }
                    continue;
                    
                    /* Acepto un real del tipo .xxxx */
                    
                }
                // Si después de los dígitos posteriores al '.' me encuentro una 'i', es imaginario.
                else if(new == 'i'){
                    aceptar_Lexema();
                    lex->tipo = _IMAGINARIO;
                    encontrado++;
                }
                
                /* Acabo de aceptar un imaginario del tipo .xxxxi */
                
                // Si después de los dígitos posteriores al '.' no me encuentro ni 'e' ni 'i'.
                else{
                    devolver_Caracter();
                    lex->lexema[strlen(lex->lexema)-1] = '\0';
                    aceptar_Lexema();
                    lex->tipo = _REAL;
                    encontrado++;
                }
                continue;
                
                /* Acabo de aceptar un real del tipo .xxxx */
                
            }
            
            /* Hasta aquí he reconocido todos los números que empiezan por punto y tienen cosas detrás. */
            
            // En este caso si tras el punto (que va solo) no hay al menos un número, no se trata de un número sino del punto en sí mismo.
            else{
                devolver_Caracter();
                lex->lexema[strlen(lex->lexema)-1] = '\0';
                aceptar_Lexema();
                lex->tipo = lex->lexema[strlen(lex->lexema)-1];
                encontrado++;
            }
            continue;
            
            /* Acabo de aceptar el carácter '.' */
            
        }
        
        /* Hasta aquí he reconocido TODOS los componentes léxicos numéricos (o al menos todos los necesarios para la práctica). 
          Ahora empezaré a reconocer el resto de elementos del lenguaje. */
        
        // Si empieza por una letra o '_', se trata de un identificador o una palabra reservada.
        else if(isalpha(new) || new == '_'){
            new = siguiente_Caracter();
            guardar(lex, new);
            // Mientras lea letras, dígitos o '_'.
            if(isalpha(new) || isdigit(new) || new == '_'){
                while(isalpha(new) || isdigit(new) || new == '_'){
                    new = siguiente_Caracter();
                    guardar(lex, new);
                }
                // En cuanto recibo cualquier otra cosa, ha terminado la cadena alfanumérica + '_'.
                devolver_Caracter();
                lex->lexema[strlen(lex->lexema)-1] = '\0';
                aceptar_Lexema();
                encontrado++;
                // Lo intento insertar en la tabla de símbolos. Esta función me devuelve _IDENTIFICADOR si es un identificador nuevo, o el componente léxico que sea aquello con lo que ha chocado (si chocan, son lo mismo).
                lex->tipo = insertarEnTabla(lex->lexema, 0);
            }
            
            /* Acabo de encontrar un identificador o una palabra reservada. */
            
            // Si tras la '_' no hay nada de eso, se identifica por sí misma (es el blank identifier), pero me aseguro de que lo primero que he leído sea una '_' y no un carácter alfabético solitario cualquiera.
            else if(lex->lexema[strlen(lex->lexema)-2] == '_'){
                devolver_Caracter();
                lex->lexema[strlen(lex->lexema)-1] = '\0';
                aceptar_Lexema(); 
                encontrado++;
                // Lo intento insertar en la tabla.
                lex->tipo = insertarEnTabla(lex->lexema, 0);
            }
            
            /* Acabo de encontrar una '_' */
            
            // Lo mismo que antes, pero para cualquier carácter alfabético.
            else if(isalpha(lex->lexema[strlen(lex->lexema)-2])){
                devolver_Caracter();
                lex->lexema[strlen(lex->lexema)-1] = '\0';
                aceptar_Lexema();
                encontrado++;
                // Lo intento insertar en la tabla.
                lex->tipo = insertarEnTabla(lex->lexema, 0);
            }
            
            /* Acabo de encontrar un identificador o una palabra reservada. */
            
        }
        // Si empieza por '"' se trata de un String.
        else if(new == '"'){
            new = siguiente_Caracter(); // Extraigo el siguiente carácter.
            guardar(lex, new);
            while(new != '"' || (lex->lexema[strlen(lex->lexema)-2] == '\\' && lex->lexema[strlen(lex->lexema)-3] != '\\')){ // Mientras no lea otro '"' QUE NO ESTÉ PRECEDIDO de UN Y SOLO UN '\'.
                // Si me encuentro un '"' y antes había un '\' (Y NO DOS), debo guardar solo el '"' en el lexema.
                if(new == '"' && lex->lexema[strlen(lex->lexema)-2] == '\\' && lex->lexema[strlen(lex->lexema)-3] != '\\'){
                    lex->lexema[strlen(lex->lexema)-2] = '\0';
                    guardar(lex, new);
                }
                new = siguiente_Caracter();
                guardar(lex, new);
            }
            // En cuanto encuentro ese '"' final, se termina el string.
            aceptar_Lexema();
            lex->tipo = _STRING;
            encontrado++;
        }
        
        /* Acabo de aceptar un String. 
         Ahora voy a empezar a reconocer otros operadores, rune literals y símbolos no contemplados. También comentarios. */
        
        // : o :=
        else if(new == ':'){
            new = siguiente_Caracter();
            guardar(lex, new);
            if(new == '='){
                aceptar_Lexema();
                lex->tipo = _OP_ASIG;
                encontrado++;
            }
            else{
                devolver_Caracter();
                lex->lexema[strlen(lex->lexema)-1] = '\0';
                aceptar_Lexema();
                lex->tipo = lex->lexema[strlen(lex->lexema)-1];
                encontrado++;
            }
        }
        // <-
        else if(new == '<'){
            new = siguiente_Caracter();
            guardar(lex, new);
            if(new == '-'){
                aceptar_Lexema();
                lex->tipo = _OP_FLECHA;
                encontrado++;
            }
            else{
                errorLexico(linea); // Aviso de un error léxico.
                devolver_Caracter();
                continue;
            }
        }
        // +=
        else if(new == '+'){
            new = siguiente_Caracter();
            guardar(lex, new);
            if(new == '='){
                aceptar_Lexema();
                lex->tipo = _OP_SUMA_CMPX;
                encontrado++;
            }
            else{
                devolver_Caracter();
                lex->lexema[strlen(lex->lexema)-1] = '\0';
                aceptar_Lexema();
                lex->tipo = lex->lexema[strlen(lex->lexema)-1];
                encontrado++;
            }
        }
        // / y comentarios (no me molesto en guardar los caracteres extraídos).
        else if(new == '/'){
            new = siguiente_Caracter();
            // Si tras el '/' hay otro '/' se trata de un comentario inline.
            if(new == '/'){
                new = siguiente_Caracter();
                while(new != '\n' && new != EOF){
                    new = siguiente_Caracter();
                }
                devolver_Caracter(); // Devuelvo el último carácter al sistema de entrada (el que rompió el bucle, es decir: '\n' o EOF.
            }
            // Si tras el '/' hay un '*', se trata de un comentario en bloque.
            else if(new == '*'){
                while(1){
                    new = siguiente_Caracter();
                    if(new == '\n') linea++;
                    if(new == '*'){
                        new = siguiente_Caracter();
                        if(new == '\n') linea++;
                        if(new == '/'){
                            aceptar_Lexema();
                            break;
                        }
                        else if(new == EOF){
                            devolver_Caracter();
                            aceptar_Lexema();
                            break;
                        }
                    }
                    else if(new == EOF){
                        devolver_Caracter();
                        aceptar_Lexema();
                        break;
                    }
                }
            }
            else{ // Encuentro '/'.
                devolver_Caracter();
                aceptar_Lexema();
                lex->tipo = lex->lexema[strlen(lex->lexema)-1];
                encontrado++;
            }
        }
        else if(new == ';' || new == ',' || new == '=' || new == '-' || new == '*' || new == '(' || new == ')' || new == '[' || new == ']' || new == '{' || new == '}'){
            aceptar_Lexema();
            lex->tipo = new;
            encontrado++;
        }
        // EOF
        else if(new == EOF){ // Si encuentro el EOF del fichero acepto y aviso del fin del análisis.
            encontrado++;
            estado = 0;
        }
        // Caso especial porque incremento el número de línea. Además debo devolverlo porque el sintáctico necesita saber si hay saltos de línea separando componentes.
        else if(new == '\n'){
            linea++;
            aceptar_Lexema();
            strcpy(lex->lexema, "\\n");
            lex->tipo = new;
            encontrado++;
        }
        // Elementos que debo ignorar pero no reportar como error.
        else if(new == '\t' || new == '\r' || new == ' ') {}
        // Si encuentro otros elementos no definidos los consumo y aviso del error léxico.
        else{
            aceptar_Lexema();
            errorLexico(linea);
        }
    }
    return (estado);
}
