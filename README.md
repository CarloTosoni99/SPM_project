

# Parallel and Distributed Systems: Paradigms and Models #

## Project; Applications Jacobi (AJ) Carlo Tosoni 644824, a.a. 2021/2022 ##

---

The project is composed by four different implementations of the Jacobi Method. Below is written which are the main features of each version, how to compile the code and which are the parameters of each program.

It is possible to compile all the files running the command&nbsp; &nbsp; ```make```

---

### seq_jacobi.cpp ###

Implements the sequential version of the Jacobi method. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution. Requires the files __utils.cpp__ and __utils.h__ to initialize the linear system and to compute the stopping criterion.

__To compile__:&nbsp; &nbsp; ```g++ -std=c++20 -O3 seq_jacobi.cpp utils.cpp -o seq_jacobi```

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __ch_conv__ : if it's equal to 1 the program will compute the stopping criterion ||x - x_old||/||x|| at each iteration, the program will stop if _tol_ < ||x - x_old||/||x||. If it's equal to 0 the program will not compute any stopping criterion.
5. float __tol__ : tolerance for convergence, if __tol__ = 0.0  the program will compute at each iteration the stopping criterion without ever reaching convergence. This parameter will not be considered by the program if __ch_conv__ = 0.
6. int __stats__ : if it's equal to 1 or 2 the programm will print some stats about the program execution (i.e. if __stats__ == 1, the program measures the time required to compute one interation of the while loop of the Jacobi method, if __stats__ == 2, the program measures the time required to compute one iteration of the internal for loop of the Jacobi method), if it's equal to 0 it will not.


---

### par_jacobi.cpp 

Implements a parallel version of the Jacobi method. The first internal for loop of the Jacobi algorithm has been parallelised using native c++ threads and barriers. The computation of the stopping criterion is perfomed sequentially. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution. Requires the files __utils.cpp__ and __utils.h__ to initialize the linear system and to compute the stopping criterion.

__To compile__:&nbsp; &nbsp; ```g++ -std=c++20 -O3 -pthread par_jacobi.cpp utils.cpp -o par_jacobi```

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __ch_conv__ : if it's equal to 1 the program will compute the stopping criterion ||x - x_old||/||x|| at each iteration, the program will stop if _tol_ < ||x - x_old||/||x||. If it's equal to 0 the program will not compute any stopping criterion.
5. float __tol__ : tolerance for convergence, if __tol__ = 0.0  the program will compute at each iteration the stopping criterion without ever reaching convergence. This parameter will not be considered by the program if __ch_conv__ = 0.
6. int __nw__ : parallel degree of the program.
7. int __stats__ : if it's equal to 1 the programm will print some stats about the program execution (i.e. elapsed execution time of the fastest
 thread, elapsed execution time of the slowest thread, average elapsed execution time of all the threads, maximum
 waiting time among the threads, minimum waiting time among the threads, average waiting time of all the threads, percentage of total active time), if it's equal to 0 it will not.


---

### par_jacobi2.cpp

Implements a parallel version of the Jacobi method. The first internal for loop of the Jacobi algorithm has been parallelised implementing a thread pool created using native c++ threads. It doesn't compute any stopping criteria. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution. Requires the files __utils.cpp__ and __utils.h__ to initialize the linear system and to compute the stopping criterion.

__To compile__:&nbsp; &nbsp; ```g++ -std=c++20 -O3 -pthread par_jacobi2.cpp utils.cpp -o par_jacobi2```

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __ch_conv__ : if it's equal to 1 the program will compute the stopping criterion ||x - x_old||/||x|| at each iteration, the program will stop if _tol_ < ||x - x_old||/||x||. If it's equal to 0 the program will not compute any stopping criterion.
5. float __tol__ : tolerance for convergence, if __tol__ = 0.0  the program will compute at each iteration the stopping criterion without ever reaching convergence. This parameter will not be considered by the program if __ch_conv__ = 0.
6. int __nw__ : parallel degree of the program. 
7. int __csize__: chunks' dimension, it indicates how many iterations a subtask consists of.
8. int __stats__ : if it's equal to 1 the programm will print some stats about the program execution (i.e. lowest total execution time among all threads, highest total execution time among all the threads, average execution time of the threads, lowest total
waiting time among all the threads, highest total waiting time among all the threads, average waiting time of the threads,
average ratio execution time/(execution time + waiting time) of the threads, total time needed to refill the queue by
the main thread), if it's equal to 0 it will not.

---

### par_jacobi_ff.cpp

Implements a parallel version of the Jacobi method. The first internal for loop of the Jacobi algorithm has been parallelised using the class __ParallelFor__ from the programming library __FastFlow__. It doesn't compute any stopping criteria. Requires the file __my_timer.cpp__ to measure the elapsed time during its execution. Requires the files __utils.cpp__ and __utils.h__ to initialize the linear system and to compute the stopping criterion.

__To compile__:&nbsp; &nbsp; ```g++ -std=c++20 -O3 -pthread par_jacobi_ff.cpp utils.cpp -o par_jacobi_ff```&nbsp; &nbsp; &nbsp; &nbsp; (Requires __FastFlow__ configured)

__Parameters__:

1. int __seed__ : seed to generate random numbers.
2. int __n__ : linear system's dimension.
3. int __n_iter__ : number of Jacobi iterations to execute.
4. int __ch_conv__ : if it's equal to 1 the program will compute the stopping criterion ||x - x_old||/||x|| at each iteration, the program will stop if _tol_ < ||x - x_old||/||x||. If it's equal to 0 the program will not compute any stopping criterion.
5. float __tol__ : tolerance for convergence, if __tol__ = 0.0  the program will compute at each iteration the stopping criterion without ever reaching convergence. This parameter will not be considered by the program if __ch_conv__ = 0.
6. int __nw__ : parallel degree of the program. 
7. int __chunk_size__: chunks' dimension (required by the method __parallel_for()__ of the class __ParallelFor__).

---

### test.cpp 

This file will not be compiled by the command ```make```. This file has been implemented to study the time required to fork-join threads and to notify waiting threads.

__To compile__:&nbsp; &nbsp; ```g++ -std=c++20 -O3 -pthread test.cpp -o test```

__Parameters__:

1. int __nw__ : parallel degree of the program.
2. int __dummy_mode__ : if it's equal to 1, the program instantiates __nw__ threads that compute a dummy function, the program prints the elapsed time to fork-join the __nw__ threads. If it's equal to 0, the program instantiates __nw__ threads that wait the main thread on a mutex. When the main thread is ready it uses a __condition_variable__ to execute __notify_all()__ and to wake up all the waiting threads. In this case, the program prints the elapsed time to execute __notify_all()__ and to fork-join the __nw__ threads. 

---

__NB__; to carry out my experiments I always passed the arguments __ch_conv__ = 0 and __tol__ = 0 to the programs.
