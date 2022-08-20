
FLAGS = -pthread -std=c++20 -O3
COMP = g++


all: clean seq_jacobi par_jacobi par_jacobi2 par_jacobi_ff

seq_jacobi:
	$(COMP) seq_jacobi.cpp utils.cpp -o seq_jacobi $(FLAGS)
	
par_jacobi:
	$(COMP) par_jacobi.cpp utils.cpp -o par_jacobi $(FLAGS)
	
par_jacobi2:
	$(COMP) par_jacobi2.cpp utils.cpp -o par_jacobi2 $(FLAGS)
	
par_jacobi_ff:
	$(COMP) par_jacobi_ff.cpp utils.cpp -o par_jacobi_ff $(FLAGS)

	
clean:
	-rm seq_jacobi par_jacobi par_jacobi2 par_jacobi_ff
