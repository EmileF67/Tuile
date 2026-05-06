all:
	g++ -Wall -Wextra -std=c++17 -Iinclude src/Engine/*.cpp src/Apps/*/*.cpp src/Apps/Bar/BarComponents/*.cpp src/Engine/Components/*.cpp src/Engine/Utils/*.cpp src/main.cpp -lncursesw -o build/main