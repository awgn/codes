/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef MATRIX_HH
#define MATRIX_HH

#include <iostream>
#include <stdexcept>

namespace more {

    template <typename T>
        class Matrix {

            int row;
            int col;

            T *matrix;

            public:
            Matrix(int r, int c) : 
            row(r), 
            col(c),  
            matrix(new T[r*c])
            {}

            ~Matrix() {
                delete [] matrix;
            }

            Matrix(Matrix &aMatrix) :
            row(aMatrix.row),
            col(aMatrix.col),
            matrix(new T[aMatrix.row*aMatrix.col])
            {
                for(int i=0;i < aMatrix.row ; i++)
                    for(int j=0; j < aMatrix.col; j++) {
                        matrix[i*col+j] = aMatrix(i,j);
                    }               
            }

            Matrix &operator=(const Matrix &mat) {
                if (this == &mat)
                    return *this;
                if (row != mat.row || col != mat.col )
                    throw std::runtime_error("matrix size mismatch!");
                for (int i=0; i < row; i++)
                    for (int j=0; j < col; j++)
                        (*this)(i,j) = mat(i,j);

                return *this;
            }

            T &operator()(int i, int j) {
                if ( i < 0 || i >= row || j < 0 || j >= col )
                    throw std::runtime_error("bad i,j index!");
                return matrix[i*col+j];        
            }


            T &operator()(int i, int j) const {
                if ( i < 0 || i >= row || j < 0 || j >= col )
                    throw std::runtime_error("bad i,j index!");
                return matrix[i*col+j];        
            }

            friend std::ostream & operator<<(std::ostream &o, Matrix<T> &mat) {
                for (int i=0; i< mat.row ; i++) {
                    o << "[ ";
                    for(int j=0;j<mat.col; j++)
                        o << mat(i,j) << " ";
                    o << "]\n";
                }
                return o;
            }

        };

}

#endif /* MATRIX_HH */
