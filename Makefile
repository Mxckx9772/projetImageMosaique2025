COMPILER = g++
FLAGS = -Wall -O3 -mavx2

INCLUDE_DIR = include
LIB_DIR = lib
OBJ_DIR = obj
BIN_DIR = bin
IMG_DIR = img

SOURCES = $(wildcard *.cpp)
EXECUTABLES = $(patsubst %.cpp, $(BIN_DIR)/%, $(SOURCES))

LIB = $(wildcard $(LIB_DIR)/*.cpp)
OBJ = $(patsubst $(LIB_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(LIB))

# Default target
all: $(EXECUTABLES)

# Compilation des objets de la librairie
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.cpp $(OBJ_DIR) $(INCLUDE_DIR)
	$(COMPILER) $(FLAGS) -I $(INCLUDE_DIR) -c $< -o $@

# Linking du programme principal
$(BIN_DIR)/%: %.cpp $(OBJ) $(BIN_DIR)
	$(COMPILER) $(FLAGS) $(OBJ) $< -o $@

# Instanciation des dossiers
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Supression des dossiers
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(PP_DIR) $(EXECUTABLES)

# Lancer le programme
run: clean $(TARGET)
	clear
	@echo "Liste des arguments : $(filter-out run,$(MAKECMDGOALS))"
	./$(TARGET) $(filter-out run,$(MAKECMDGOALS))

bench: clean bin/bench
	clear
	@echo "Execution du benchmark..."
	./bin/bench $(filter-out bench,$(MAKECMDGOALS))
	@echo "Fin du benchmark"