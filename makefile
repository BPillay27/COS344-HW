#add all the files that you need to the files makefile variable below as a space seperated list
files = Figure.cpp Shape3D.cpp Object.cpp RenderState.cpp shapes/light.cpp

main: main.cpp shader.cpp $(files)
	g++ -g shader.cpp $(files) main.cpp -lglfw -pthread -lGLEW -ldl -lGL -o main

clean:
	rm -f *.o main

run:
	./main
val:
	valgrind ./main

all:
	make clean
	make
	make run


zip:
	make clean
	make
	zip -r u24594522.zip shapes/ *.hpp *.h *.cpp makefile *.glsl main *.png