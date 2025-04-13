COMPILER = g++
FLAGS = -Wall -O3 -mavx2

INCLUDE_DIR = include
LIB_DIR = lib
OBJ_DIR = obj
BIN_DIR = bin
IMG_DIR = img
PP_DIR = pp_img


SOURCES = $(wildcard *.cpp)
EXECUTABLES = $(patsubst %.cpp, $(BIN_DIR)/%, $(SOURCES))

LIB = $(wildcard $(LIB_DIR)/*.cpp)
OBJ = $(patsubst $(LIB_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(LIB))

# Compilation
all: $(EXECUTABLES)

# Compilation des objets
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.cpp $(OBJ_DIR)
	$(COMPILER) $(FLAGS) -I $(INCLUDE_DIR) -c $< -o $@

# Linking du programme
$(BIN_DIR)/%:%.cpp $(OBJ) $(MAIN) $(BIN_DIR) $(IMG_DIR) $(PP_DIR)
	$(COMPILER) $(FLAGS) $(OBJ) $< -o $@ 

# Instanciation des dossiers
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(PP_DIR):
	mkdir -p $(PP_DIR)

# Supression des dossiers
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(PP_DIR) $(EXECUTABLES)

# Lancer le programme
run: clean $(TARGET)
	clear
	@echo "Liste des arguments : $(filter-out run,$(MAKECMDGOALS))"
	./$(TARGET) $(filter-out run,$(MAKECMDGOALS))