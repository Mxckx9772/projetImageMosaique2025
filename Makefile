COMPILER = g++
FLAGS = -Wall

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
CIBLE = $(BIN_DIR)/main

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(CIBLE)
	rm -f main.o

$(CIBLE): $(OBJS) main.o $(BIN_DIR)
	$(COMPILER) $(CXXFLAGS) $(OBJS) main.o -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(OBJ_DIR)
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

main.o: main.cpp
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
