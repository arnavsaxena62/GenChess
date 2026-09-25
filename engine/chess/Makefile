CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

TARGET = test

SRC = test.cpp movegen.cpp utils.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)