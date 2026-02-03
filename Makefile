all:
	g++ -Wall -Wextra -std=c++17 -Iinclude src/Engine/*.cpp src/Apps/*.cpp src/main.cpp -lncursesw -o build/main
