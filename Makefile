COMPILER = g++
FLAGS = -Wall

INCLUDE_DIR = include
LIB_DIR = lib
OBJ_DIR = obj
BIN_DIR = bin


TARGET = $(BIN_DIR)/main
MAIN = main.cpp


LIB = $(wildcard $(LIB_DIR)/*.cpp)
OBJ = $(patsubst $(LIB_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(LIB))

# Compilation
all: $(TARGET)

# Compilation des objets
$(OBJ_DIR)/%.o: $(LIB_DIR)/%.cpp $(OBJ_DIR)
	$(COMPILER) $(FLAGS) -I $(INCLUDE_DIR) -c $< -o $@

# Linking du programme
$(TARGET): $(OBJ) $(MAIN) $(BIN_DIR)
	$(COMPILER) $(FLAGS) $(OBJ) $(MAIN) -o $@

# Instanciation des dossiers
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Supression des dossiers
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(TARGET)

# Execution du programe
run: $(TARGET)
	./$(TARGET)