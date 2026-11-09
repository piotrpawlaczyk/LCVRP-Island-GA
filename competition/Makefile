all:
	mkdir -p build
	g++ -std=c++20 -O2 -pthread main.cpp GA.cpp LcVRPContest.cpp conditions/*.cpp operations/*.cpp optimizer/*.cpp population/*.cpp templete/*.cpp utility/*.cpp -o build/ga

run: all
	./build/ga