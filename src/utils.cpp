#include <iostream>
#include <cmath>
#include <random>
#include <vector>

#include "utils.h"


// This function initiliazes the elements of the matrix A and the vector b
void initialize_problem(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, float min_value, float max_value) {
    
    
    // Random initialization of matrix A
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            a[i][j] = min_value + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(max_value-min_value)));
        }
    }
    
    
    
    // Transform matrix A into a strictly diagonally matrix
    float sum;
    for(int i = 0; i <n; i++) {
        // Store the absolute sum of the i-th row
        sum = 0.0;
        
        for(int j = 0; j < n; j++) {
            sum += std::abs(a[i][j]);
        }
        sum -= std::abs(a[i][i]);
        
        //check if the elements on the diagonal has to be modified
        if (std::abs(a[i][i]) <= sum){
            if (a[i][i] < 0) 
                a[i][i] = -sum - 10;
            else
                a[i][i] = sum + 10;
        }
    }
    
    // Random initialization of vector b
    for (int i = 0; i < n; i++){
        b[i] = min_value + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(max_value-min_value)));;
    }
    
}


// Compute the stopping criterion to understand if Jacobi has achieved convergence.
// Executed iff ch_conv = 1
float compute_norm(std::vector<float>& x, std::vector<float>& xo, int n) {
    
    // This function computes the stopping criterion ||x - x_old||/||x|| if ch_conv == 1
    float num = 0.0;
    for(int i = 0; i < n; i++) {
        num = num + ((x[i] - xo[i])*(x[i] - xo[i]));
    }
    num = std::sqrt(num);

    float den = 0.0;
    for(int i = 0; i < n; i++) {
        den = den + (x[i]*x[i]);
    }
    den = std::sqrt(den);

    return num / den;
}


//OPTIONAL you can use this function to print the system created
void print_system(int n, std::vector<std::vector<float>> &a, std::vector<float> &b) {
    
    std::cout << std::endl;
    std::cout << "printing the matrix A" << std::endl;
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "printing the diagonal of A" << std::endl;
    for(int i = 0; i < n; i++) {
        std::cout << a[i][i] << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    
    std::cout << "printing the vector b" << std::endl;
    for(int i = 0; i < n; i++) {
        std::cout << b[i] << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;

}

// OPTIONAL, this function checks the error at the end of Jacobi
void check_error(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x) {
    
    float err;
    for(int i = 0; i < n; i++) {
        err = 0.0;
        for(int j = 0; j < n; j++){
            err = a[i][j]*x[j] + err;
        }
        err = err - b[i];
        std::cout << "Error at row i " << err << std::endl;
    }
}
