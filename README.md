# analizador_lexico

###### IDE
> NetBeans

###### Conocimientos principales aplicados
- C
- Teoría de compiladores
- Go
- Árboles binarios de búsqueda
- Expresiones regulares

###### Descripción
En este proyecto se desarrolla un analizador léxico para el lenguaje Go que sea capaz de reconocer al menos los componentes léxicos que aparecen en el código [concurrentSum.go](./concurrentSum.go).

Para ello se implementan un sistema de entrada con doble buffer centinela, el analizador léxico per se, un analizador sintáctico falso (el análisis sintáctico está fuera del alcance de la práctica, se usa simplemente para llamar al analizador léxico), una tabla de símbolos y otros elementos menos relevantes pero necesarios para el correcto funcionamiento.

El guion de la práctica se puede ver [aquí](./Guion).

###### Ejecución
[Instrucciones](./README).

Solo se ha probado en **Linux**.

###### Salida
La salida generada puede consultarse [aquí](./salida.txt).

###### Posibles mejoras
- Ampliarlo para que identifique todos los componentes léxicos del lenguaje Go.
- Mejorar la eficiencia de la función que identifica los componentes.
- Mejorar la claridad de la función que identifica los componentes.
