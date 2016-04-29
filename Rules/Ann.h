#ifndef ANN_H_
#define ANN_H_

class Ann {
public:
    Ann(int dimension);
    ~Ann();
    void training(int** x, int** y, int nSamples);
    void print();
private:
    int dimension;
    int*** m_w;
};

#endif /* ANN_H_ */
