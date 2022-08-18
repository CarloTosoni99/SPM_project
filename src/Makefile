all: seq_jacobi par_jacobi par_jacobi2 par_jacobi_ff

seq_jacobi:
	g++ -o3 seq_jacobi.cpp -o seq_jacobi
	
par_jacobi:
	g++ -std=c++20 -o3 -pthread par_jacobi.cpp -o par_jacobi
	
par_jacobi2:
	g++ -o3 -pthread par_jacobi2.cpp -o par_jacobi2
	
par_jacobi_ff:
	g++ -o3 -pthread par_jacobi_ff.cpp -o par_jacobi_ff
