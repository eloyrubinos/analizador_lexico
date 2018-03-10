CC= gcc -Wall
HEADER_FILES_DIR = .
INCLUDES = -I $(HEADER_FILES_DIR)
MAIN= anaLex
SRCS = main.c AnalizadorLexico.c AnalizadorSintactico.c ArbolBinario.c SistemaDeEntrada.c TablaSimbolos.c gestionErrores.c
DEPS = $(HEADER_FILES_DIR)/AnalizadorLexico.h AnalizadorSintactico.h ArbolBinario.h Definiciones.h SistemaDeEntrada.h TablaSimbolos.h gestionErrores.h
OBJS = $(SRCS:.c=.o)
$(MAIN): $(OBJS)
	$(CC) -o $(MAIN) $(OBJS)
	rm -f *.o *~
%.o: %.c $(DEPS)
	$(CC) -c $< $(INCLUDES)
cleanall: clean
	rm -f $(MAIN)
clean:
	rm -f *.o *~
