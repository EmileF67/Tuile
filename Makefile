all:
	g++ -Wall -Wextra -std=c++17 -Iinclude src/Engine/*.cpp src/main.cpp -lncurses -o build/main
