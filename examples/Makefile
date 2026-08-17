# Compilador
CXX = g++

# Flags de compilación: advertencias y estándar C++11
CXXFLAGS = -Wall -Wextra -std=c++11

# Nombre del ejecutable final
TARGET = aht21b

# Archivo fuente
SOURCES = aht21b.cpp

# Regla por defecto (la primera)
all: $(TARGET)

# Cómo construir el ejecutable
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Limpiar archivos generados
clean:
	rm -f $(TARGET)

# Regla para ejecutar el programa después de compilar
run: $(TARGET)
	sudo ./$(TARGET)

# Declarar que 'clean' y 'run' no son nombres de archivos
.PHONY: all clean run

