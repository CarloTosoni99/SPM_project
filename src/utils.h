#include <vector>


// Initialize the matrices A and b of the linear system
void initialize_problem(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, float min_value, float max_value);

// Compute the stopping criterion to understand if Jacobi has achieved convergence.
// Executed iff ch_conv = 1
float compute_norm(std::vector<float>& x, std::vector<float>& xo, int n);


// OPTIONAL, print the matrices A and b of the linear system
void print_system(int n, std::vector<std::vector<float>> &a, std::vector<float> &b);

// OPTIONAL, this function checks the error at the end of Jacobi
void check_error(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x);
