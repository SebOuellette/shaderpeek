CXX = g++

LOPTS = -lsfml-graphics -lsfml-window -lsfml-system 

all: shaderpeek

shaderpeek: main.o
	$(CXX) main.o -o shaderpeek $(LOPTS) -pthread

main.o: main.cpp
	$(CXX) -c main.cpp -o main.o

clean:
	rm shaderpeek *.o