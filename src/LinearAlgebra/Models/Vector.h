//*****************************************************************************
//  Title       :src/LinearAlgebra/Vector.h
//  Author      :Tanabe Yuta
//  Date        :2019/11/10
//  Copyright   :(C)2019 TanabeYuta
//*****************************************************************************


#pragma once
#include <cmath>
#include <iostream>
#include <cassert>
#include <vector>


#include "Matrix.h"


namespace PANSFEM2{
    template<class T>
    class Matrix;


    template<class T>
    class Vector{
public:
        Vector();
        virtual ~Vector();
        Vector(int _size);
        Vector(const Vector<T>& _vec);
        Vector(const std::initializer_list<T>& _vec);
        Vector(const std::vector<T>& _vec);
        Vector(const Matrix<T>& _mat);


        int SIZE() const;
        T& operator()(int _i);


        Vector<T>& operator=(const Vector<T>& _vec);
        Vector<T>& operator+=(const Vector<T>& _vec);
        Vector<T>& operator-=(const Vector<T>& _vec);
        Vector<T>& operator*=(T _a);
        Vector<T>& operator/=(T _a);


        Vector<T> operator+(const Vector<T>& _vec);
        Vector<T> operator-(const Vector<T>& _vec);
        Vector<T> operator-();
        T operator*(const Vector<T>& _vec);
        Matrix<T> operator*(const Matrix<T>& _mat);
        Vector<T> operator*(T _a);
        Vector<T> operator/(T _a);


        template<class U>
	    friend std::ostream& operator << (std::ostream& _out, const Vector<U>& _vec);
        template<class U>
        friend Vector<U> operator*(U _a, const Vector<U>& _vec);
        template<class U>
        friend Matrix<U> Diagonal(const Vector<U>& _vec);
        template<class U>
        friend Vector<U> VectorProduct(Vector<U> _vec0, Vector<U> _vec1);


        T Norm();
        Matrix<T> Transpose();
        Vector<T> Vstack(const Vector<T>& _vec);
        Vector<T> Segment(int _head, int _tail);
        Vector<T> Normal();


        template<class F>
	    friend class Matrix;


protected:
        int size;       //Degree of vector
        T* values;      //Values of vector
    };


    template<class T>
    Vector<T>::Vector(){
        this->size = 0;
    }


    template<class T>
    Vector<T>::~Vector(){
        delete[] this->values;
    }


    template<class T>
    Vector<T>::Vector(int _size){
        this->size = _size;
        this->values = new T[this->size]();
    }


    template<class T>
    Vector<T>::Vector(const Vector<T>& _vec){
        this->size = _vec.size;
        this->values = new T[this->size];
        for(int i = 0; i < this->size; i++){
            this->values[i] = _vec.values[i];
        }
    }


    template<class T>
    Vector<T>::Vector(const std::initializer_list<T>& _vec){
        this->size = _vec.size();
        this->values = new T[this->size];
        for(int i = 0; i < this->size; i++){
            this->values[i] = *(_vec.begin() + i);
        }
    }


    template<class T>
    Vector<T>::Vector(const std::vector<T>& _vec){
        this->size = _vec.size();
        this->values = new T[this->size];
        for(int i = 0; i < this->size; i++){
            this->values[i] = _vec[i];
        }
    }


    template<class T>
    Vector<T>::Vector(const Matrix<T>& _mat){
        assert(_mat.col == 1);
        this->size = _mat.row;
        this->values = new T[this->size];
        for(int i = 0; i < this->size; i++){
            this->values[i] = _mat.values[i];
        }
    }


    template<class T>
    int Vector<T>::SIZE() const {
        return this->size;
    }


    template<class T>
    T& Vector<T>::operator()(int _i){
        assert(0 <= _i && _i < this->size);
        return this->values[_i];
    }


    template<class T>
    Vector<T>& Vector<T>::operator=(const Vector<T>& _vec){
        if(this != &_vec){
            if(this->size != 0){
                delete[] this->values;
            }

            this->size = _vec.size;
            this->values = new T[this->size];
            for(int i = 0; i < this->size; i++){
                this->values[i] = _vec.values[i];
            }
        }
        return *this;
    }


    template<class T>
    Vector<T>& Vector<T>::operator+=(const Vector<T>& _vec){
        assert(this->size == _vec.size);
        for(int i = 0; i < this->size; i++){
            this->values[i] += _vec.values[i];
        }
        return *this;
    }


    template<class T>
    Vector<T>& Vector<T>::operator-=(const Vector<T>& _vec){
        assert(this->size == _vec.size);
        for(int i = 0; i < this->size; i++){
            this->values[i] -= _vec.values[i];
        }
        return *this;
    }


    template<class T>
    Vector<T>& Vector<T>::operator*=(T _a){
        for(int i = 0; i < this->size; i++){
            this->values[i] *= _a;
        }
        return *this;
    }


    template<class T>
    Vector<T>& Vector<T>::operator/=(T _a){
        for(int i = 0; i < this->size; i++){
            this->values[i] /= _a;
        }
        return *this;
    }


    template<class T>
    Vector<T> Vector<T>::operator+(const Vector<T>& _vec){
        assert(this->size == _vec.size);
        Vector<T> vec = *this;
        for(int i = 0; i < vec.size; i++){
            vec.values[i] += _vec.values[i];
        }
        return vec;
    }


    template<class T>
    Vector<T> Vector<T>::operator-(const Vector<T>& _vec){
        assert(this->size == _vec.size);
        Vector<T> vec = *this;
        for(int i = 0; i < vec.size; i++){
            vec.values[i] -= _vec.values[i];
        }
        return vec;
    }


    template<class T>
    Vector<T> Vector<T>::operator-(){
        Vector<T> vec = *this;
        for(int i = 0; i < vec.size; i++){
            vec.values[i] *= -1;
        }
        return vec;
    }


    template<class T>
    T Vector<T>::operator*(const Vector<T>& _vec){
        assert(this->size == _vec.size);
        T value = T();
        for(int i = 0; i < this->size; i++){
            value += this->values[i] * _vec.values[i];
        }
        return value;
    }


    template<class T>
    Matrix<T> Vector<T>::operator*(const Matrix<T>& _mat){
        assert(_mat.row == 1);
        Matrix<T> mat = Matrix<T>(this->size, _mat.col);
        for(int i = 0; i < mat.row; i++){
            for(int j = 0; j < mat.col; j++){
                mat.values[i * mat.col + j] = this->values[i] * _mat.values[j];
            }
        }
        return mat;
    }


    template<class T>
    Vector<T> Vector<T>::operator*(T _a){
        Vector<T> vec = *this;
        for(int i = 0; i < vec.size; i++){
            vec.values[i] *= _a;
        }
        return vec;
    }


    template<class T>
    Vector<T> Vector<T>::operator/(T _a){
        Vector<T> vec = *this;
        for(int i = 0; i < vec.size; i++){
            vec.values[i] /= _a;
        }
        return vec;
    }


    template<class U>
    std::ostream& operator << (std::ostream& _out, const Vector<U>& _vec){
        for(int i = 0; i < _vec.size; i++){
            _out << _vec.values[i] << std::endl;
        }
        return _out;
    }


    template<class T>
    T Vector<T>::Norm(){
        T value = T();
        for(int i = 0; i < this->size; i++){
            value += pow(this->values[i], 2.0);
        }
        return sqrt(value);
    }


    template<class T>
    Matrix<T> Vector<T>::Transpose(){
        Matrix<T> mat = Matrix<T>(1, this->size);
        for(int i = 0; i < mat.row * mat.col; i++){
            mat.values[i] = this->values[i];
        }
        return mat;
    }


    template<class T>
    Vector<T> Vector<T>::Vstack(const Vector<T>& _vec){
        Vector<T> vec = Vector<T>(this->size + _vec.size);
        for(int i = 0; i < this->size; i++){
            vec.values[i] = this->values[i];
        }
        for(int i = 0; i < _vec.size; i++){
            vec.values[i + this->size] = _vec.values[i];
        }
        return vec;
    }


    template<class T>
    Vector<T> Vector<T>::Segment(int _head, int _tail){
        assert(_head >= 0 && _tail <= this->size && _head < _tail);
        Vector<T> vec = Vector<T>(_tail - _head);
        for(int i = 0; i < vec.size; i++){
            vec.values[i] = this->values[_head + i];
        }
        return vec;
    }


    template<class T>
    Vector<T> Vector<T>::Normal(){
        Vector<T> v = *this;
        return v/v.Norm();
    }


    template<class U>
    Vector<U> operator*(U _a, const Vector<U>& _vec){
        Vector<U> vec = _vec;
        for(int i = 0; i < vec.size; i++){
            vec.values[i] *= _a;
        }
        return vec;
    }


    template<class U>
    Vector<U> VectorProduct(Vector<U> _vec0, Vector<U> _vec1){
        assert(_vec0.size == 3 && _vec1.size == 3);

        Vector<U> v = Vector<U>(3);
        v(0) = _vec0(1)*_vec1(2) - _vec0(2)*_vec1(1);
        v(1) = _vec0(2)*_vec1(0) - _vec0(0)*_vec1(2);
        v(2) = _vec0(0)*_vec1(1) - _vec0(1)*_vec1(0);
        return v; 
    }
}