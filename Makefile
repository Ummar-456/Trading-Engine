CXX := g++
CXXFLAGS := -O3 -march=native -mtune=native -flto -fno-rtti -DNDEBUG -Wall -Wextra -Wpedantic -std=c++17 -pipe
LDFLAGS := -pthread -flto -s

SRC := main.cpp Order.cpp Orderbook.cpp Logger.cpp
OBJ := $(SRC:.cpp=.o)
BIN := engine

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

run: $(BIN)
	./$(BIN)

.PHONY: all clean run