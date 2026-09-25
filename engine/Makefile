CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

ENGINE_SRC = src/movegen.cpp src/utils.cpp

.PHONY: all bot targets clean

all: bot targets

bot: bot/main.cpp $(ENGINE_SRC)
	$(CXX) $(CXXFLAGS) bot/main.cpp $(ENGINE_SRC) -o bot/bot.out

targets: targets/main.cpp $(ENGINE_SRC)
	$(CXX) $(CXXFLAGS) targets/main.cpp $(ENGINE_SRC) -o targets/targets.out

clean:
	rm -f bot/bot.out targets/targets.out