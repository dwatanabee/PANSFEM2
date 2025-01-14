#include <iostream>
#include <vector>


#include "../../src/LinearAlgebra/Models/Vector.h"
#include "../../src/PrePost/Import/ImportFromCSV.h"
#include "../../src/PrePost/Mesher/SquareAnnulusMesh.h"
#include "../../src/FEM/Equation/PlaneStrain.h"
#include "../../src/FEM/Equation/Homogenization.h"
#include "../../src/FEM/Controller/ShapeFunction.h"
#include "../../src/FEM/Controller/GaussIntegration.h"
#include "../../src/FEM/Controller/BoundaryCondition.h"
#include "../../src/FEM/Controller/Assembling.h"
#include "../../src/LinearAlgebra/Solvers/CG.h"
#include "../../src/PrePost/Export/ExportToVTK.h"
#include "../../src/FEM/Equation/General.h"


using namespace PANSFEM2;


int main() {
	//----------Set parameter----------
	double E = 100.0;
	double Poisson = 0.3;
	double Volume = 1.0;	//	Volume of an unit cell
		

	//----------Make model----------
	std::string model_path = "sample/homogenization/";
	SquareAnnulusMesh2<double> mesh = SquareAnnulusMesh2<double>(1.0, 1.0, 20, 20, 10, 10);
    std::vector<Vector<double> > x = mesh.GenerateNodes();
    std::vector<std::vector<int> > elements = mesh.GenerateElements();
	std::vector<std::pair<int, int> > ufixed;
	ImportPeriodicFromCSV(ufixed, model_path + "Periodic.csv");

    std::vector<Vector<double> > chi0 = std::vector<Vector<double> >(x.size(), Vector<double>(2));
    std::vector<Vector<double> > chi1 = std::vector<Vector<double> >(x.size(), Vector<double>(2));
    std::vector<Vector<double> > chi2 = std::vector<Vector<double> >(x.size(), Vector<double>(2));
	std::vector<std::vector<int> > nodetoglobal = std::vector<std::vector<int> >(x.size(), std::vector<int>(2, 0));
	int KDEGREE = SetPeriodic(nodetoglobal, ufixed);


	//----------Get characteristics displacement----------
	LILCSR<double> K = LILCSR<double>(KDEGREE, KDEGREE);
	std::vector<double> F0 = std::vector<double>(KDEGREE, 0.0);
	std::vector<double> F1 = std::vector<double>(KDEGREE, 0.0);
	std::vector<double> F2 = std::vector<double>(KDEGREE, 0.0);
	
    for (auto element : elements) {
        std::vector<std::vector<std::pair<int, int> > > nodetoelement;
		Matrix<double> Ke;
		PlaneStrainStiffness<double, ShapeFunction4Square, Gauss4Square>(Ke, nodetoelement, element, { 0, 1 }, x, E, Poisson, 1.0);
		Assembling(K, Ke, nodetoglobal, nodetoelement, element);
		Assembling(F0, chi0, Ke, nodetoglobal, nodetoelement, element);
		Assembling(F1, chi1, Ke, nodetoglobal, nodetoelement, element);
		Assembling(F2, chi2, Ke, nodetoglobal, nodetoelement, element);

		Matrix<double> Fes;
		HomogenizePlaneStrainBodyForce<double, ShapeFunction4Square, Gauss4Square>(Fes, nodetoelement, element, { 0, 1 }, x, E, Poisson, 1.0);
		Vector<double> Fe0 = Fes.Block(0, 0, Ke.ROW(), 1);
		Assembling(F0, Fe0, nodetoglobal, nodetoelement, element);
		Vector<double> Fe1 = Fes.Block(0, 1, Ke.ROW(), 1);
		Assembling(F1, Fe1, nodetoglobal, nodetoelement, element);
		Vector<double> Fe2 = Fes.Block(0, 2, Ke.ROW(), 1);
		Assembling(F2, Fe2, nodetoglobal, nodetoelement, element);
	}

	for (auto element : elements) {
        std::vector<std::vector<std::pair<int, int> > > nodetoelement;
		Matrix<double> Ke;
		WeakSpring<double>(Ke, nodetoelement, element, { 0, 1 }, x, 1.0e-9);
		Assembling(K, Ke, nodetoglobal, nodetoelement, element);
	}

	CSR<double> Kmod = CSR<double>(K);
	std::vector<double> result0 = ScalingCG(Kmod, F0, 100000, 1.0e-10);
    Disassembling(chi0, result0, nodetoglobal);
	std::vector<double> result1 = ScalingCG(Kmod, F1, 100000, 1.0e-10);
    Disassembling(chi1, result1, nodetoglobal);
	std::vector<double> result2 = ScalingCG(Kmod, F2, 100000, 1.0e-10);
    Disassembling(chi2, result2, nodetoglobal);


	//----------Get homogenized value----------
	Matrix<double> CH = Matrix<double>(3, 3);
	Matrix<double> I = Identity<double>(3);
	for (auto element : elements) {
		CH += HomogenizePlaneStrainConstitutive<double, ShapeFunction4Square, Gauss4Square>(x, element, chi0, chi1, chi2, E, Poisson, 1.0);
		I += HomogenizePlaneStrainCheck<double, ShapeFunction4Square, Gauss4Square>(x, element, chi0, chi1, chi2, 1.0);
	}
	std::cout << I/Volume << std::endl << CH/Volume << std::endl;;


	//----------Export microscopic result----------
	std::ofstream fout(model_path + "result_microscopic.vtk");
	MakeHeadderToVTK(fout);
	AddPointsToVTK(x, fout);
	AddElementToVTK(elements, fout);
	AddElementTypes(std::vector<int>(elements.size(), 9), fout);
	AddPointVectors(chi0, "chi0", fout, true);
	AddPointVectors(chi1, "chi1", fout, false);
	AddPointVectors(chi2, "chi2", fout, false);
	fout.close();

	return 0;
}