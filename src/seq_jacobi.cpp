#include <vector>
#include <iostream>
#include <functional>

#include "utils.h"
#include "my_timer.cpp"

#define MAX_VALUE 32
#define MIN_VALUE -32


// Standard version of the sequential jacobi algorithm, this function is executed iff it is passed the argument
// stats == 0 to the program
void seq_jacobi(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n, int n_iter,
                float tol, int ch_conv) {

    // start the Jacobi method
    int k = 1;
    std::vector<float> xo = x;
    float val;
    while (k <= n_iter) {
        for(int i = 0; i < n; i++) {
            val = 0.0;
            for(int j = 0; j < n; j++) {
                val += a[i][j]*xo[j];
            }
            val -= a[i][i]*xo[i];
            x[i] = (b[i]-val)/(a[i][i]);
        }
        
        //check if the method has reached the convergence, in case stop the iterations
        if (ch_conv != 0)
            if (compute_norm(std::ref(x), std::ref(xo), n) < tol) {
                std::cout << "condition for convergence is satisfied" << std::endl;
                return;
            }
        k++;
        xo = x;
    }
}


// Second version of the sequential Jacobi algorithm, this version prints some stats about the execution time of the
// Jacobi method. This version has been separated from the standard one due to fact that it requires more time to be
// executed. This version will be executed iff it the argument passed to the program is either stats == 1 or stats == 2
void seq_jacobi_stats(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n,
                      int n_iter, float tol, int ch_conv, int stats) {

  // Instantiate a timer to measure the time needed to execute an iteration of the for loops.
  my_timer iter_timer;
  // Instantiate a timer to measure the time needed to execute the operations that cannot be parallelized
  my_timer seq_timer;

  // This function can measure the elapsed time of different components based on the value of the attribute stats. The
  // code below prints what the function is measuring
  if (stats == 1)
    std::cout << "Stats is equal to 1. Printing time required to compute an iteration of the while loop..." << std::endl;
  else if (stats == 2)
    std::cout << "Stats is equal to 2. Printing time required to compute an iteration of the internal for loop..." << std::endl;


  // start the Jacobi method
  int k = 1;
  std::vector<float> xo = x;
  float val;
  while (k <= n_iter) {

    // If stats is equal to 1 the timer measures the elapsed to execute one iteration of the internal for loop
    if (stats == 1) {
      iter_timer.start_timer();
    }

    for(int i = 0; i < n; i++) {

      // If stats is equal to 2 the timer measures the elapsed to execute one iteration of the internal for loop
      if (stats == 2) {
        iter_timer.start_timer();
      }

      val = 0.0;
      for(int j = 0; j < n; j++) {
        val += a[i][j]*xo[j];
      }
      val -= a[i][i]*xo[i];
      x[i] = (b[i]-val)/(a[i][i]);

      // get and print the elapsed time
      if (stats == 2) {
        time_t elapsed = iter_timer.get_time();
        std::cout << elapsed << std::endl;
      }
    }

    // get and print the elapsed time
    if (stats == 1) {
      time_t elapsed = iter_timer.get_time();
      std::cout << elapsed << std::endl;
    }

    // start measure the elapsed time to execute the operations that cannot be parallelized
    seq_timer.start_timer();

    // check if the method has reached the convergene, in case stop the iterations
    if (ch_conv != 0)
      if (compute_norm(std::ref(x), std::ref(xo), n) < tol) {
        std::cout << "condition for convergence is satisfied" << std::endl;
        return;
      }
    k++;
    xo = x;

    // get and print the result
    time_t seq_elapsed = seq_timer.get_time();
    std::cout << "sequential ops " << seq_elapsed << std::endl;


  }
}

int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
    float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv == 1
    int stats = std::stoul(argv[6]); //if it's 1 or 2 the programm will print some stats about the program execution, if it's 0 it will not

    srand(seed);
    
    // Creation of matrix A
    std::vector<std::vector<float>> a(n);
    for (int i = 0; i < n; i++) {
        a[i] = std::vector<float>(n);
    }    
    // Creation of vector b
    std::vector<float> b(n);
    // Creation of vector x
    std::vector<float> x(n, 0);

    // Initialize the matrices A and b
    initialize_problem(n, std::ref(a), std::ref(b), MIN_VALUE, MAX_VALUE);
    
    // OPTIONAL, print the system created
    //print_system(n, std::ref(a), std::ref(b));
    

    // Start to measure the elapsed time
    my_timer timer;
    timer.start_timer();
    
    // Compute Jacobi
    // if stats is equal to 0, the program will execute the standard version of the sequential Jacobi algorithm
    if (stats == 0)
      seq_jacobi(std::ref(a), std::ref(b), std::ref(x), n, n_iter, tol, ch_conv);
    // Otherwise, if stats is equal to 1, the program will execute the version of the sequential Jacobi algorithm that
    // prints some data about its execution time
    else
      seq_jacobi_stats(std::ref(a), std::ref(b), std::ref(x), n, n_iter, tol, ch_conv, stats);


    // Measure the elapsed time and print the result.
    time_t elapsed = timer.get_time();
    std::cout << "Elapsed time: " << elapsed << std::endl;

    
    // OPTIONAL to check the error
    //check_error(n, std::ref(a), std::ref(b), std::ref(x));
    
    return 0;
}
