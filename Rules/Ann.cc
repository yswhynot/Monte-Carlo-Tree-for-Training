#include "Ann.h"

#include <stdlib.h>
#include <time.h>

#include <iostream>
using namespace std;

#define NLAYER 2
#define RANDMAX 10

Ann::Ann(int dimension) {
    // Use time to set random seed
    srand(time(NULL));
    // Set dimension of inputs and outputs
    this->dimension = dimension;
    // Set hidden layers and output layers
    this->m_w = new int**[NLAYER];
    // Set all nodes to be random number
    for (int layer = 0; layer < NLAYER; layer++) {
        this->m_w[layer] = new int*[dimension]; // Number of nodes in one layer
        for (int node = 0; node < dimension; node++) {
            this->m_w[layer][node] = new int[dimension + 1]; // Weights with one bias
            for (int weight = 0; weight < dimension + 1; weight++) {
                this->m_w[layer][node][weight] = rand() % RANDMAX;
            }
        }
    }
}

Ann::~Ann() {
    // Delete w
    for (int layer = 0; layer < NLAYER; layer++) {
        for (int node = 0; node < dimension; node++) {
            delete [] this->m_w[layer][node];
        }
        delete [] this->m_w[layer];
    }
    delete [] this->m_w;
}

void Ann::training(int** x, int** y, int nSamples) {
    bool flag;
    do {
        flag = false;
        for (int sample = 0; sample < nSamples; sample++) {
            /** Forwards **/
            int sum[NLAYER][this->dimension]; // Use to store the result of each layer
            for (int layer = 0; layer < NLAYER; layer++) {
                for (int node = 0; node < this->dimension; node ++) {
                    sum[layer][node] = 0;
                    for (int weight = 0; weight < this->dimension; weight++) {
                        if (layer == 0) {
                            // Use input when first layer
                            sum[layer][node] += this->m_w[layer][node][weight] * x[sample][weight];
                        } else {
                            // Use last layer
                            sum[layer][node] += this->m_w[layer][node][weight] * sum[layer - 1][weight];
                        }
                    }
                    // Handle bias (minus)
                    sum[layer][node] += this->m_w[layer][node][this->dimension] * -1;
                    // Normalization
                    sum[layer][node] = (sum[layer][node] >= 0);
                }
            }
            /** Backwards **/
            // Calculate delta
            int delta[NLAYER][this->dimension]; // Use to store delta of each layer
            for (int layer = NLAYER - 1; layer >= 0; layer--) {
                for (int node = 0; node < this->dimension; node++) {
                    if (layer == NLAYER - 1) {
                        // Use output when last layer
                        delta[layer][node] = sum[layer][node] - y[sample][node];
                        // If delta not equals to 0, then need to update
                        if (delta[layer][node] != 0) {
                            flag = true;
                        }
                    } else {
                        // Use weights and last layer
                        delta[layer][node] = 0;
                        for (int nextLayerNode = 0; nextLayerNode < this->dimension; nextLayerNode++) {
                            delta[layer][node] += this->m_w[layer + 1][nextLayerNode][node] * delta[layer + 1][nextLayerNode];
                        }
                    }
                }
            }
            // Update weights
            if (flag) {
                for (int layer = 0; layer < NLAYER; layer++) {
                    for (int node = 0; node < this->dimension; node++) {
                        for (int weight = 0; weight < this->dimension; weight++) {
                            if (layer == 0) {
                                // Use input when first layer
                                this->m_w[layer][node][weight] = this->m_w[layer][node][weight] - delta[layer][node] * x[sample][weight];
                            } else {
                                // Use last layer
                                this->m_w[layer][node][weight] = this->m_w[layer][node][weight] - delta[layer][node] * sum[layer - 1][weight];
                            }
                        }
                        // Handle bias
                        this->m_w[layer][node][this->dimension] = this->m_w[layer][node][this->dimension] - delta[layer][node] * (-1);
                    }
                }
            }
        }
    } while (flag);
}

void Ann::print() {
    for (int node = 0; node < this->dimension; node++) {
        for (int weight = 0; weight < this->dimension + 1; weight++) {
            for (int layer = 0; layer < NLAYER; layer++) {
                cout << this->m_w[layer][node][weight] << "\t";
            }
            cout << "\n";
        }
        cout << "\n";
    }
}

// For testing
int main() {
    int x[7][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0}};
    int y[7][3] = {{0,0,0},{1,1,1},{0,0,0},{1,1,1},{0,0,0},{1,1,1},{0,0,0}};

    int** xx = new int*[7];
    int** yy = new int*[7];

    for (int i = 0; i < 7; i++) {
        xx[i] = x[i];
        yy[i] = y[i];
    }

    Ann ann(3);
    ann.training(xx, yy, 7);
    ann.print();

    delete [] xx;
    delete [] yy;

    return 0;
}
