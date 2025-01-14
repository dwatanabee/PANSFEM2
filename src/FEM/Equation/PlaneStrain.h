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
	void PlaneStrainStiffness(Matrix<T>& _Ke, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, T _E, T _V, T _t) {
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

		Matrix<T> D = Matrix<T>(3, 3);
		D(0, 0) = 1.0 - _V;	D(0, 1) = _V;		D(0, 2) = T();
		D(1, 0) = D(0, 1);	D(1, 1) = 1.0 - _V;	D(1, 2) = T();
		D(2, 0) = D(0, 2);	D(2, 1) = D(1, 2);	D(2, 2) = 0.5*(1.0 - 2.0*_V);
		D *= _E / ((1.0 - 2.0*_V)*(1.0 + _V));

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

			_Ke += B.Transpose()*D*B*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}
	}


	//******************************Make element stiffness matrix with Selective Reduced Integration******************************
	template<class T, template<class>class SF, template<class>class ICV, template<class>class ICD>
	void PlaneStrainStiffnessSRI(Matrix<T>& _Ke, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, T _E, T _V, T _t) {
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
		Matrix<T> Dvol = Matrix<T>(3, 3);
		Dvol(0, 0) = 1.0;	Dvol(0, 1) = 1.0;	Dvol(0, 2) = T();
		Dvol(1, 0) = 1.0;	Dvol(1, 1) = 1.0;	Dvol(1, 2) = T();
		Dvol(2, 0) = T();	Dvol(2, 1) = T();	Dvol(2, 2) = T();
		Dvol *= _E/(3.0*(1.0 - 2.0*_V));

		for (int g = 0; g < ICV<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(ICV<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> B = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2 * n) = dNdX(0, n);	B(0, 2 * n + 1) = T();			
				B(1, 2 * n) = T();			B(1, 2 * n + 1) = dNdX(1, n);	
				B(2, 2 * n) = dNdX(1, n);	B(2, 2 * n + 1) = dNdX(0, n);	
			}

			_Ke += B.Transpose()*Dvol*B*J*_t*ICV<T>::Weights[g][0]*ICV<T>::Weights[g][1];
		}

		//----------Integraion deviation strain term----------
		Matrix<T> Ddev = Matrix<T>(3, 3);
		Ddev(0, 0) = 4.0;	Ddev(0, 1) = -2.0;	Ddev(0, 2) = T();
		Ddev(1, 0) = -2.0;	Ddev(1, 1) = 4.0;	Ddev(1, 2) = T();
		Ddev(2, 0) = T();	Ddev(2, 1) = T();	Ddev(2, 2) = 3.0;
		Ddev *= _E/(6.0*(1.0 + _V));

		for (int g = 0; g < ICD<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(ICD<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();
			Matrix<T> dNdX = dXdr.Inverse()*dNdr;

			Matrix<T> B = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2 * n) = dNdX(0, n);	B(0, 2 * n + 1) = T();			
				B(1, 2 * n) = T();			B(1, 2 * n + 1) = dNdX(1, n);	
				B(2, 2 * n) = dNdX(1, n);	B(2, 2 * n + 1) = dNdX(0, n);	
			}

			_Ke += B.Transpose()*Ddev*B*J*_t*ICD<T>::Weights[g][0]*ICD<T>::Weights[g][1];
		}
	}


	//******************************Make element stiffness matrix with B-bar******************************
	template<class T, template<class>class SF, template<class>class ICV, template<class>class ICD>
	void PlaneStrainStiffnessBbar(Matrix<T>& _Ke, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, T _E, T _V, T _t) {
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

		Matrix<T> D = Matrix<T>(3, 3);
		D(0, 0) = 1.0 - _V;	D(0, 1) = _V;		D(0, 2) = T();
		D(1, 0) = D(0, 1);	D(1, 1) = 1.0 - _V;	D(1, 2) = T();
		D(2, 0) = D(0, 2);	D(2, 1) = D(1, 2);	D(2, 2) = 0.5*(1.0 - 2.0*_V);
		D *= _E/((1.0 - 2.0*_V)*(1.0 + _V));

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

			_Ke += Bvol.Transpose()*D*Bvol*J*_t*ICV<T>::Weights[g][0]*ICV<T>::Weights[g][1];
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

			_Ke += Bdev.Transpose()*D*Bdev*J*_t*ICD<T>::Weights[g][0]*ICD<T>::Weights[g][1];
		}
	}


	//******************************Make element stiffness matrix with Wilson-Taylor******************************
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStrainStiffnessWilsonTaylor(Matrix<T>& _Ke, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, T _E, T _V, T _t) {
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

			Vector<T> r = IC<T>::Points[g];
			Matrix<T> dPdr = Matrix<T>(2, 2);
			dPdr(0, 0) = -2.0*r(0);		dPdr(0, 1) = T();
			dPdr(1, 0) =T();			dPdr(1, 1) = -2.0*r(1);
			Matrix<T> dPdX = dXdr.Inverse()*dPdr;

			Matrix<T> G = Matrix<T>(3, 4);
			G(0, 0) = dPdX(0, 0);	G(0, 1) = T();			G(0, 2) = dPdX(0, 1);	G(0, 3) = T();
			G(1, 0) = T();			G(1, 1) = dPdX(1, 0);	G(1, 2) = T();			G(1, 3) = dPdX(1, 1);
			G(2, 0) = dPdX(1, 0);	G(2, 1) = dPdX(0, 0);	G(2, 2) = dPdX(1, 1);	G(2, 3) = dPdX(0, 1);

			_Ke += B.Transpose()*D*B*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
			Keaa += G.Transpose()*D*G*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
			Kead += G.Transpose()*D*B*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}

		_Ke -= Kead.Transpose()*Keaa.Inverse()*Kead;
	}


	//******************************Make element tangent stiffness matrix and residual vector with TotalLagrange******************************
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStrainStiffnessTotalLagrange(Matrix<T>& _Ke, Vector<T>& _Fe, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, std::vector<Vector<T> >& _u, T _E, T _V, T _t) {
		assert(_doulist.size() == 2);

		_Ke = Matrix<T>(2*_element.size(), 2*_element.size());
		_Fe = Vector<T>(2*_element.size());
		_nodetoelement = std::vector<std::vector<std::pair<int, int> > >(_element.size(), std::vector<std::pair<int, int> >(2));
		for(int i = 0; i < _element.size(); i++) {
			_nodetoelement[i][0] = std::make_pair(_doulist[0], 2*i);
			_nodetoelement[i][1] = std::make_pair(_doulist[1], 2*i + 1);
		}

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);	X(i, 1) = _x[_element[i]](1);
		}

		Matrix<T> U = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			U(i, 0) = _u[_element[i]](0);	U(i, 1) = _u[_element[i]](1);
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

			Matrix<T> Z = (dNdX*U).Transpose();
			Matrix<T> F = Identity<T>(2) + Z;
			Matrix<T> BL = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				BL(0, 2*n) = F(0, 0)*dNdX(0, n);						BL(0, 2*n + 1) = F(1, 0)*dNdX(0, n);
				BL(1, 2*n) = F(0, 1)*dNdX(1, n);						BL(1, 2*n + 1) = F(1, 1)*dNdX(1, n);
				BL(2, 2*n) = F(0, 1)*dNdX(0, n) + F(0, 0)*dNdX(1, n);	BL(2, 2*n + 1) = F(1, 1)*dNdX(0, n) + F(1, 0)*dNdX(1, n);
			}

			_Ke += BL.Transpose()*D*BL*J*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];

			Matrix<T> E = (Z + Z.Transpose() + Z.Transpose()*Z)/2.0;
			Vector<T> Ev = Vector<T>({ E(0, 0), E(1, 1), E(0, 1) + E(1, 0) });
			Vector<T> Sv = D*Ev;

			Matrix<T> BNL = Matrix<T>(4, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				BNL(0, 2*n) = dNdX(0, n);	BNL(0, 2*n + 1) = T();
				BNL(1, 2*n) = T();			BNL(1, 2*n + 1) = dNdX(0, n);
				BNL(2, 2*n) = dNdX(1, n);	BNL(2, 2*n + 1) = T();
				BNL(3, 2*n) = T();			BNL(3, 2*n + 1) = dNdX(1, n);
			}

			Matrix<T> S00 = Sv(0)*Identity<T>(2);	Matrix<T> S01 = Sv(2)*Identity<T>(2);
			Matrix<T> S10 = Sv(2)*Identity<T>(2);	Matrix<T> S11 = Sv(1)*Identity<T>(2);
			Matrix<T> S = (S00.Hstack(S01)).Vstack((S10.Hstack(S11)));

			_Ke += BNL.Transpose()*S*BNL*J*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];

			_Fe -= BL.Transpose()*Sv*J*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}
	}


	//******************************Make element tangent stiffness matrix and residual vector with UpdatedLagrange******************************
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStrainStiffnessUpdatedLagrange(Matrix<T>& _Ke, Vector<T>& _Fe, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, std::vector<Vector<T> >& _u, T _E, T _V, T _t) {
		assert(_doulist.size() == 2);

		_Ke = Matrix<T>(2*_element.size(), 2*_element.size());
		_Fe = Vector<T>(2*_element.size());
		_nodetoelement = std::vector<std::vector<std::pair<int, int> > >(_element.size(), std::vector<std::pair<int, int> >(2));
		for(int i = 0; i < _element.size(); i++) {
			_nodetoelement[i][0] = std::make_pair(_doulist[0], 2*i);
			_nodetoelement[i][1] = std::make_pair(_doulist[1], 2*i + 1);
		}

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);	X(i, 1) = _x[_element[i]](1);
		}

		Matrix<T> U = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			U(i, 0) = _u[_element[i]](0);	U(i, 1) = _u[_element[i]](1);
		}

		Matrix<T> x = X + U;

		T mu0 = 0.5*_E/(1.0 + _V);
		T lambda0 = 2.0*mu0*_V/(1.0 - 2.0*_V);

		for (int g = 0; g < IC<T>::N; g++) {
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dxdr = dNdr*x;
			T J = dxdr.Determinant();
			Matrix<T> dNdx = dxdr.Inverse()*dNdr;
			Matrix<T> F = ((dNdr*X).Inverse()*dNdr*x).Transpose();

			T detF = F.Determinant();
			T mu = (mu0 - lambda0*log(detF))/detF;
			T lambda = lambda0/detF;
			Matrix<T> C = Matrix<T>(3, 3);
			C(0, 0) = lambda + 2.0*mu;	C(0, 1) = lambda;			C(0, 2) = T();
			C(1, 0) = lambda;			C(1, 1) = lambda + 2.0*mu;	C(1, 2) = T();
			C(2, 0) = T();				C(2, 1) = T();				C(2, 2) = mu;
			Matrix<T> BL = Matrix<T>(3, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				BL(0, 2*n) = dNdx(0, n);	BL(0, 2*n + 1) = T();
				BL(1, 2*n) = T();			BL(1, 2*n + 1) = dNdx(1, n);
				BL(2, 2*n) = dNdx(1, n);	BL(2, 2*n + 1) = dNdx(0, n);
			}
			_Ke += BL.Transpose()*C*BL*J*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];

			Matrix<T> S = (mu0/detF)*(F*F.Transpose() - Identity<T>(2)) + (lambda0*log(detF)/detF)*Identity<T>(2);
			Matrix<T> BNL = Matrix<T>(4, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				BNL(0, 2*n) = dNdx(0, n);	BNL(0, 2*n + 1) = T();
				BNL(1, 2*n) = T();			BNL(1, 2*n + 1) = dNdx(0, n);
				BNL(2, 2*n) = dNdx(1, n);	BNL(2, 2*n + 1) = T();
				BNL(3, 2*n) = T();			BNL(3, 2*n + 1) = dNdx(1, n);
			}
			Matrix<T> S00 = S(0, 0)*Identity<T>(2);	Matrix<T> S01 = S(0, 1)*Identity<T>(2);
			Matrix<T> S10 = S(1, 0)*Identity<T>(2);	Matrix<T> S11 = S(1, 1)*Identity<T>(2);
			Matrix<T> Sm = (S00.Hstack(S01)).Vstack((S10.Hstack(S11)));
			_Ke += BNL.Transpose()*Sm*BNL*J*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];

			Vector<T> Sv = { S(0, 0), S(1, 1), S(0, 1) };
			_Fe -= BL.Transpose()*Sv*J*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}
	}


	//******************************Make element mass matrix******************************
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStrainMass(Matrix<T>& _Me, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, T _rho, T _t) {
		assert(_doulist.size() == 2);

		_Me = Matrix<T>(2*_element.size(), 2*_element.size());
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
			Vector<T> N = SF<T>::N(IC<T>::Points[g]);
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();

			Matrix<T> B = Matrix<T>(2, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2*n) = N(n);	B(0, 2*n + 1) = T();			
				B(1, 2*n) = T();	B(1, 2*n + 1) = N(n);	
			}

			_Me += B.Transpose()*B*J*_rho*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}
	}


	//******************************Make surface force vector******************************
	template<class T, template<class>class SF, template<class>class IC, class F>
	void PlaneStrainSurfaceForce(Vector<T>& _Fe, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, F _f, T _t){
		assert(_doulist.size() == 2);

		_Fe = Vector<T>(2*_element.size());
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
			Vector<T> N = SF<T>::N(IC<T>::Points[g]);
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Vector<T> x = X.Transpose()*N;
			Matrix<T> dXdr = dNdr*X;
			T dl = sqrt((dXdr*dXdr.Transpose())(0, 0));

			Matrix<T> B = Matrix<T>(2, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2*n) = N(n);	B(0, 2*n + 1) = T();			
				B(1, 2*n) = T();	B(1, 2*n + 1) = N(n);	
			}

			_Fe += B.Transpose()*_f(x)*dl*_t*IC<T>::Weights[g][0];
		}
	}


	//******************************Make surface pressure tangent stiffness matrix with UpdatedLagrange****************************** 
	template<class T, template<class>class SF, template<class>class IC>
	void PlaneStrainSurfacePressureUpdatedLagrange(Matrix<T>& _Ke, Vector<T>& _Fe, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, std::vector<Vector<T> >& _u, T _P, T _t) {
		assert(_doulist.size() == 2);

		_Ke = Matrix<T>(2*_element.size(), 2*_element.size());
		_Fe = Vector<T>(2*_element.size());
		_nodetoelement = std::vector<std::vector<std::pair<int, int> > >(_element.size(), std::vector<std::pair<int, int> >(2));
		for(int i = 0; i < _element.size(); i++) {
			_nodetoelement[i][0] = std::make_pair(_doulist[0], 2*i);
			_nodetoelement[i][1] = std::make_pair(_doulist[1], 2*i + 1);
		}

		Matrix<T> X = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			X(i, 0) = _x[_element[i]](0);	X(i, 1) = _x[_element[i]](1);
		}

		Matrix<T> U = Matrix<T>(_element.size(), 2);
		for(int i = 0; i < _element.size(); i++){
			U(i, 0) = _u[_element[i]](0);	U(i, 1) = _u[_element[i]](1);
		}

		Matrix<T> x = X + U;

		for (int g = 0; g < IC<T>::N; g++) {
			Vector<T> N = SF<T>::N(IC<T>::Points[g]);
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Matrix<T> dxdr = dNdr*x;
			
			Matrix<T> K = Matrix<T>(2*_element.size(), 2*_element.size());
			Vector<T> F = Vector<T>(2*_element.size());
			for(int i = 0; i < _element.size(); i++) {
				for(int j = 0; j < _element.size(); j++) {
					K(2*i, 2*j) = T();										K(2*i, 2*j + 1) = dNdr(0, i)*N(j) - N(i)*dNdr(0, j);
					K(2*i + 1, 2*j) = -dNdr(0, i)*N(j) + N(i)*dNdr(0, j);	K(2*i + 1, 2*j + 1) = T();
				}
				F(2*i) = -N(i)*dxdr(0, 1);
				F(2*i + 1) = N(i)*dxdr(0, 0);
			}
			_Ke -= 0.5*_P*K*_t*IC<T>::Weights[g][0];
			_Fe += _P*F*_t*IC<T>::Weights[g][0];
		}
	}


	//******************************Make body force vector******************************
	template<class T, template<class>class SF, template<class>class IC, class F>
	void PlaneStrainBodyForce(Vector<T>& _Fe, std::vector<std::vector<std::pair<int, int> > >& _nodetoelement, const std::vector<int>& _element, const std::vector<int>& _doulist, std::vector<Vector<T> >& _x, F _f, T _t){
		assert(_doulist.size() == 2);

		_Fe = Vector<T>(2*_element.size());
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
			Vector<T> N = SF<T>::N(IC<T>::Points[g]);
			Matrix<T> dNdr = SF<T>::dNdr(IC<T>::Points[g]);
			Vector<T> x = X.Transpose()*N;
			Matrix<T> dXdr = dNdr*X;
			T J = dXdr.Determinant();

			Matrix<T> B = Matrix<T>(2, 2*_element.size());
			for (int n = 0; n < _element.size(); n++) {
				B(0, 2 * n) = N(n);	B(0, 2 * n + 1) = T();			
				B(1, 2 * n) = T();	B(1, 2 * n + 1) = N(n);	
			}

			_Fe += B.Transpose()*_f(x)*J*_t*IC<T>::Weights[g][0]*IC<T>::Weights[g][1];
		}
	}
}