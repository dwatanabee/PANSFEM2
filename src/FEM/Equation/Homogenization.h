//*****************************************************************************
//  Title		:   src/FEM/Equation/Homogenization.h
//  Author	    :   Tanabe Yuta
//  Date		:   2020/04/11
//  Copyright	:   (C)2020 TanabeYuta
//*****************************************************************************


#pragma once
#include <vector>
#include <cassert>


#include "../../LinearAlgebra/Models/Matrix.h"
#include "../../LinearAlgebra/Models/Vector.h"


namespace PANSFEM2 {
	//********************Make equevalent nodal force for Homogenize********************
	template<class T, template<class>class SF, template<class>class IC>
	void HomogenizePlaneStrainBodyForce(Matrix<T>& _Fes, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, T _E, T _V, T _t) {
		assert(_doulist.size() == 2);

		_Fes = Matrix<T>(2*_element.size(), 3);
		_nodetoelement = std::vector<std::vector<std::pair<int, int> > >(_element.size(), std::vector<std::pair<int, int> >(2));
		for(int i = 0; i < _element.size(); i++) {
			_nodetoelement[i][0] = std::make_pair(_doulist[0], 2*i);
			_nodetoelement[i][1] = std::make_pair(_doulist[1], 2*i + 1);
		}

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);	X(i, 1) = _x[_element[i]](1);
		}

		Matrix<T> D = Matrix<T>(3, 3);
		D(0, 0) = 1.0 - _V;	D(0, 1) = _V;		D(0, 2) = T();
		D(1, 0) = D(0, 1);	D(1, 1) = 1.0 - _V;	D(1, 2) = T();
		D(2, 0) = D(0, 2);	D(2, 1) = D(1, 2);	D(2, 2) = 0.5*(1.0 - 2.0*_V);
		D *= _E/((1.0 - 2.0*_V)*(1.0 + _V));

		for (int g = 0; g < IC<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> B = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2*n) = dNdX(0, n);	B(0, 2*n + 1) = T();			
				B(1, 2*n) = T();		B(1, 2*n + 1) = dNdX(1, n);	
				B(2, 2*n) = dNdX(1, n);	B(2, 2*n + 1) = dNdX(0, n);	
			}

			_Fes += B.Transpose()*D*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}
	}


    //********************Make homogenized elemental Constitutive********************
    template<class T, template<class>class SF, template<class>class IC>
	Matrix<T> HomogenizePlaneStrainConstitutive(std::vector<Vector<T> >& _x, std::vector<int>& _element, std::vector<Vector<T> >& _chi0, std::vector<Vector<T> >& _chi1, std::vector<Vector<T> >& _chi2, T _E, T _V, T _t) {
		Matrix<T> C = Matrix<T>(3, 3);

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);	X(i, 1) = _x[_element[i]](1);
		}

        Matrix<T> CHI = Matrix<T>(2*_element.size(), 3);
        for(int i = 0; i < _element.size(); i++){
            CHI(2*i, 0) = _chi0[_element[i]](0); 		CHI(2*i, 1) = _chi1[_element[i]](0);		CHI(2*i, 2) = _chi2[_element[i]](0);
            CHI(2*i + 1, 0) = _chi0[_element[i]](1);	CHI(2*i + 1, 1) = _chi1[_element[i]](1);	CHI(2*i + 1, 2) = _chi2[_element[i]](1);
		}

		Matrix<T> D = Matrix<T>(3, 3);
		D(0, 0) = 1.0 - _V;	D(0, 1) = _V;		D(0, 2) = T();
		D(1, 0) = D(0, 1);	D(1, 1) = 1.0 - _V;	D(1, 2) = T();
		D(2, 0) = D(0, 2);	D(2, 1) = D(1, 2);	D(2, 2) = 0.5*(1.0 - 2.0*_V);
		D *= _E/((1.0 - 2.0*_V)*(1.0 + _V));

		Matrix<T> I = Identity<T>(3);

		for (int g = 0; g < IC<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> B = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2*n) = dNdX(0, n);	B(0, 2*n + 1) = T();			
				B(1, 2*n) = T();		B(1, 2*n + 1) = dNdX(1, n);	
				B(2, 2*n) = dNdX(1, n);	B(2, 2*n + 1) = dNdX(0, n);	
			}

			C += D*(I - B*CHI)*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}

		return C;
	}


	//********************Check homogenization********************
    template<class T, template<class>class SF, template<class>class IC>
	Matrix<T> HomogenizePlaneStrainCheck(std::vector<Vector<T> >& _x, std::vector<int>& _element, std::vector<Vector<T> >& _chi0, std::vector<Vector<T> >& _chi1, std::vector<Vector<T> >& _chi2, T _t) {
		Matrix<T> C = Matrix<T>(3, 3);

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);	X(i, 1) = _x[_element[i]](1);
		}

        Matrix<T> CHI = Matrix<T>(2*_element.size(), 3);
        for(int i = 0; i < _element.size(); i++){
            CHI(2*i, 0) = _chi0[_element[i]](0); 		CHI(2*i, 1) = _chi1[_element[i]](0);		CHI(2*i, 2) = _chi2[_element[i]](0);
            CHI(2*i + 1, 0) = _chi0[_element[i]](1);	CHI(2*i + 1, 1) = _chi1[_element[i]](1);	CHI(2*i + 1, 2) = _chi2[_element[i]](1);
		}

		for (int g = 0; g < IC<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> B = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2*n) = dNdX(0, n);	B(0, 2*n + 1) = T();			
				B(1, 2*n) = T();		B(1, 2*n + 1) = dNdX(1, n);	
				B(2, 2*n) = dNdX(1, n);	B(2, 2*n + 1) = dNdX(0, n);	
			}

			C += -B*CHI*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}

		return C;
	}


	//********************Make element stiffness matrix******************************
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStiffness(Matrix<T>& _Ke, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, Matrix<T> _D, T _t) {
		assert(_doulist.size() == 2 && _D.ROW() == 3 && _D.COL() == 3);

		_Ke = Matrix<T>(2*_element.size(), 2*_element.size());
		_nodetoelement = std::vector<std::vector<std::pair<int, int> > >(_element.size(), std::vector<std::pair<int, int> >(2));
		for(int i = 0; i < _element.size(); i++) {
			_nodetoelement[i][0] = std::make_pair(_doulist[0], 2*i);
			_nodetoelement[i][1] = std::make_pair(_doulist[1], 2*i + 1);
		}

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);
			X(i, 1) = _x[_element[i]](1);
		}

		for (int g = 0; g < IC<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> B = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2 * n) = dNdX(0, n);	B(0, 2 * n + 1) = T();			
				B(1, 2 * n) = T();			B(1, 2 * n + 1) = dNdX(1, n);	
				B(2, 2 * n) = dNdX(1, n);	B(2, 2 * n + 1) = dNdX(0, n);	
			}

			_Ke += B.Transpose()*_D*B*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}
	}


	//******************************Make element stiffness matrix with B-bar******************************
	template<class T, template<class>class SF, template<class>class ICV, template<class>class ICD>
	void PlaneStiffnessBbar(Matrix<T>& _Ke, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, Matrix<T> _D, T _t) {
		assert(_doulist.size() == 2);

		_Ke = Matrix<T>(2*_element.size(), 2*_element.size());
		_nodetoelement = std::vector<std::vector<std::pair<int, int> > >(_element.size(), std::vector<std::pair<int, int> >(2));
		for(int i = 0; i < _element.size(); i++) {
			_nodetoelement[i][0] = std::make_pair(_doulist[0], 2*i);
			_nodetoelement[i][1] = std::make_pair(_doulist[1], 2*i + 1);
		}

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);
			X(i, 1) = _x[_element[i]](1);
		}

		//----------Integraion volume strain term----------
		for (int g = 0; g < ICV<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(ICV<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> Bvol = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				Bvol(0, 2*n) = 0.5*dNdX(0, n);	Bvol(0, 2*n + 1) = 0.5*dNdX(1, n);			
				Bvol(1, 2*n) = 0.5*dNdX(0, n);	Bvol(1, 2*n + 1) = 0.5*dNdX(1, n);	
				Bvol(2, 2*n) = T();				Bvol(2, 2*n + 1) = T();	
			}

			_Ke += Bvol.Transpose()*_D*Bvol*J*_t*ICV<T>::Weights[g][0]*ICV<T>::Weights[g][1];
		}

		//----------Integraion deviation strain term----------
		for (int g = 0; g < ICD<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(ICD<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> Bdev = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				Bdev(0, 2*n) = 0.5*dNdX(0, n);	Bdev(0, 2*n + 1) = -0.5*dNdX(1, n);			
				Bdev(1, 2*n) = -0.5*dNdX(0, n);	Bdev(1, 2*n + 1) = 0.5*dNdX(1, n);	
				Bdev(2, 2*n) = dNdX(1, n);		Bdev(2, 2*n + 1) = dNdX(0, n);	
			}

			_Ke += Bdev.Transpose()*_D*Bdev*J*_t*ICD<T>::Weights[g][0]*ICD<T>::Weights[g][1];
		}
	}


	//******************************Make element stiffness matrix with Wilson-Taylor******************************
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStiffnessWilsonTaylor(Matrix<T>& _Ke, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, Matrix<T> _D, T _t) {
		assert(_doulist.size() == 2);

		_Ke = Matrix<T>(2*_element.size(), 2*_element.size());
		Matrix<T> Keaa = Matrix<T>(4, 4);
		Matrix<T> Kead = Matrix<T>(4, 2*_element.size());
 		_nodetoelement = std::vector<std::vector<std::pair<int, int> > >(_element.size(), std::vector<std::pair<int, int> >(2));
		for(int i = 0; i < _element.size(); i++) {
			_nodetoelement[i][0] = std::make_pair(_doulist[0], 2*i);
			_nodetoelement[i][1] = std::make_pair(_doulist[1], 2*i + 1);
		}

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);
			X(i, 1) = _x[_element[i]](1);
		}

		for (int g = 0; g < IC<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;
			
			Matrix<T> B = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2*n) = dNdX(0, n);	B(0, 2*n + 1) = T();			
				B(1, 2*n) = T();		B(1, 2*n + 1) = dNdX(1, n);	
				B(2, 2*n) = dNdX(1, n);	B(2, 2*n + 1) = dNdX(0, n);	
			}

			Vector<T> r = IC<T>::Points[g];
			Matrix<T> dPdr = Matrix<T>(2, 2);
			dPdr(0, 0) = -2.0*r(0);		dPdr(0, 1) = T();
			dPdr(1, 0) =T();			dPdr(1, 1) = -2.0*r(1);
			Matrix<T> dPdX = dXdr.Inverse()*dPdr;

			Matrix<T> G = Matrix<T>(3, 4);
			G(0, 0) = dPdX(0, 0);	G(0, 1) = T();			G(0, 2) = dPdX(0, 1);	G(0, 3) = T();
			G(1, 0) = T();			G(1, 1) = dPdX(1, 0);	G(1, 2) = T();			G(1, 3) = dPdX(1, 1);
			G(2, 0) = dPdX(1, 0);	G(2, 1) = dPdX(0, 0);	G(2, 2) = dPdX(1, 1);	G(2, 3) = dPdX(0, 1);

			_Ke += B.Transpose()*_D*B*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
			Keaa += G.Transpose()*_D*G*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
			Kead += G.Transpose()*_D*B*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}

		_Ke -= Kead.Transpose()*Keaa.Inverse()*Kead;
	}
}