

# Parallel and Distributed Systems: Paradigms and Models #

## Project; Applications Jacobi (AJ) Carlo Tosoni 644824, a.a. 2021/2022 ##

---

The project is composed by five different implementations of the Jacobi Method. Below is written which are the main features of each version, how to compile the code and which are the parameters of each program.

---

### seq_jacobi.cpp ###

Implements the sequential version of the Jacobi method. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution.

__To compile__:&nbsp; &nbsp; _g++ -o3 seq_jacobi.cpp -o seq_jacobi_

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __ch_conv__ : if it's equal to 1 the program will compute the stopping criterion ||x - x_old||/||x|| at each iteration, the program will stop if _tol_ < ||x - x_old||/||x||. If it's equal to 0 the program will not compute any stopping criterion.
5. float __tol__ : tolerance for convergence, if __tol__ = 0.0  the program will compute at each iteration the stopping criterion without ever reaching convergence. This parameter will not be considered by the program if __ch_conv__ = 0.

---

### par_jacobi.cpp 

Implements a parallel version of the Jacobi method. The first internal for loop of the Jacobi algorithm has been parallelised using native c++ threads and barriers. The computation of the stopping criterion is perfomed sequentially. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution.

__To compile__:&nbsp; &nbsp; _g++ -std=c++20 -o3 -pthread par_jacobi.cpp -o par_jacobi_

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __ch_conv__ : if it's equal to 1 the program will compute the stopping criterion ||x - x_old||/||x|| at each iteration, the program will stop if _tol_ < ||x - x_old||/||x||. If it's equal to 0 the program will not compute any stopping criterion.
5. float __tol__ : tolerance for convergence, if __tol__ = 0.0  the program will compute at each iteration the stopping criterion without ever reaching convergence. This parameter will not be considered by the program if __ch_conv__ = 0.
6. int __nw__ : parallel degree of the program. 


---

### par_jacobi2.cpp

Implements a parallel version of the Jacobi method. The first internal for loop of the Jacobi algorithm has been parallelised implementing a thread pool created using native c++ threads. It doesn't compute any stopping criteria. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution.

__To compile__:&nbsp; &nbsp; _g++ -o3 -pthread par_jacobi3.cpp -o par_jacobi3_

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __nw__ : parallel degree of the program. 
5. int __csize__: chunks' dimension.

---

### par_jacobi_ff.cpp

Implements a parallel version of the Jacobi method. The first internal for loop of the Jacobi algorithm has been parallelised using the class __ParallelFor__ from the programming library __FastFlow__. It doesn't compute any stopping criteria. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution.

__To compile__:&nbsp; &nbsp; _g++ -o3 -pthread par_jacobi_ff.cpp -o par_jacobi_ff_&nbsp; &nbsp; &nbsp; &nbsp; (Requires __FastFlow__ configured)

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __nw__ : parallel degree of the program. 
5. int __chunk_size__: chunks' dimension (required by the method __parallel_for()__ of the class __ParallelFor__).



