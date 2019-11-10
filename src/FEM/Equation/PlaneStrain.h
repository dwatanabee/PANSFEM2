//*****************************************************************************
//Title		:src/FEM/Equation/PlaneStrain.h
//Author	:Tanabe Yuta
//Date		:2019/10/02
//Copyright	:(C)2019 TanabeYuta
//*****************************************************************************


#pragma once
#include <vector>
#include <cassert>


#include "../../LinearAlgebra/Models/Matrix.h"
#include "../../LinearAlgebra/Models/Vector.h"


namespace PANSFEM2 {
	//******************************Make element stiffness matrix******************************
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStrain(Matrix<T>& _Ke, std::vector<Vector<T> >& _x, std::vector<int>& _element, T _E, T _V, T _t) {
		//----------Initialize element stiffness matrix----------
		_Ke = Matrix<T>(2 * _element.size(), 2 * _element.size());

		//----------Generate cordinate matrix X----------
		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			for(int j = 0; j < 2; j++){
				X(i, j) = _x[_element[i]](j);
			}
		}

		//----------Generate D matrix----------
		Matrix<T> D = Matrix<T>(3, 3);
		D(0, 0) = 1.0 - _V;	D(0, 1) = _V;		D(0, 2) = 0.0;
		D(1, 0) = D(0, 1);	D(1, 1) = 1.0 - _V;	D(1, 2) = 0.0;
		D(2, 0) = D(0, 2);	D(2, 1) = D(1, 2);	D(2, 2) = 0.5*(1.0 - 2.0*_V);
		D *= (_E / ((1.0 - 2.0*_V)*(1.0 + _V)));

		//----------Loop of Gauss Integration----------
		for (int g = 0; g < IC<T>::N; g++) {
			//----------Get difference of shape function----------
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);

			//----------Get difference of shape function----------
			Matrix<T> dXdr = dNdr * X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse() * dNdr;

			//----------Generate B matrix----------
			Matrix<T> B = Matrix<T>(3, 2 * _element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2 * n) = dNdX(0, n);	B(0, 2 * n + 1) = 0.0;			
				B(1, 2 * n) = 0.0;			B(1, 2 * n + 1) = dNdX(1, n);	
				B(2, 2 * n) = dNdX(1, n);	B(2, 2 * n + 1) = dNdX(0, n);	
			}

			//----------Update element stiffness matrix----------
			_Ke += B.Transpose()*D*B*J*_t*IC<T>::Weights[g][0] * IC<T>::Weights[g][1];
		}
	}
}