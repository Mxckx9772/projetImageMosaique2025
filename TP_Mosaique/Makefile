# Variables pour les chemins et les fichiers
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
LDFLAGS = $(shell pkg-config --cflags --libs opencv4)  # Utilise pkg-config pour obtenir les flags de OpenCV
SRC = mosaique.cpp
OBJ = mosaique.o
OUT = mosaique
DIMENSION_IMAGETTE = 128

# Cibles par défaut
all: $(OUT)

# Cible pour compiler l'exécutable
$(OUT): $(OBJ)
	$(CXX) $(OBJ) -o $(OUT) $(LDFLAGS)

# Cible pour compiler le fichier .cpp en .o
$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC) $(LDFLAGS)

# Cible pour nettoyer les fichiers de compilation
clean:
	rm -f $(OBJ) $(OUT)

# Cible pour convertir les images (si nécessaire)
convertir_pgm:
	./$(OUT) convertir_pgm images images_pgm $(DIMENSION_IMAGETTE)

# Cible pour effectuer le traitement et générer l'image mosaïque
process:
	./$(OUT)

# Cible pour effectuer la conversion, puis la découpe, et la génération de mosaïque
full_process: convertir_pgm process

.PHONY: clean convertir_pgm process full_process
