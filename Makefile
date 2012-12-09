all: main

main: main.o mandel.o
	gcc -m64 -o main main.o mandel.o -lSDL -L/usr/local/cuda-5.0/lib64 -lcudart

mandel.o: mandel.cu mandel.h
	/usr/local/cuda-5.0/bin/nvcc --compiler-options -O3,-Wundef,-std=gnu++98,-Wno-missing-field-initializers,-Wstrict-aliasing=3,-fstrict-aliasing -m64 -I/usr/local/cuda-5.0/include -gencode arch=compute_10,code=sm_10 -gencode arch=compute_20,code=sm_20 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -c -o mandel.o mandel.cu

#	/usr/local/cuda-5.0/bin/nvcc --compiler-options -O3,-Wall,-Wextra,-Wundef,-pedantic-errors,-std=c++98,-Wno-missing-field-initializers,-Wstrict-aliasing=3,-fstrict-aliasing,-Wvariadic-macros -m64 -I/usr/local/cuda-5.0/include -gencode arch=compute_10,code=sm_10 -gencode arch=compute_20,code=sm_20 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -c -o mandel.o mandel.cu

main.o: main.c palette.h mandel.h
	gcc -O3 -c -m64 -Wall -Wextra -Wundef -pedantic-errors -std=c99 -Wstrict-prototypes -Wno-missing-field-initializers -Wstrict-aliasing=3 -fstrict-aliasing -o main.o main.c

clean: FORCE
	rm -f *.o main

FORCE:
