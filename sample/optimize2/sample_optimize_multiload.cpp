#include <iostream>
#include <vector>
#include <cmath>


#include "../../src/LinearAlgebra/Models/Vector.h"
#include "../../src/LinearAlgebra/Models/Matrix.h"
#include "../../src/LinearAlgebra/Models/LILCSR.h"
#include "../../src/PrePost/Import/ImportFromCSV.h"
#include "../../src/FEM/Controller/Assembling.h"
#include "../../src/FEM/Equation/PlaneStrain.h"
#include "../../src/FEM/Controller/BoundaryCondition.h"
#include "../../src/LinearAlgebra/Solvers/CG.h"
#include "../../src/PrePost/Export/ExportToVTK.h"
#include "../../src/FEM/Controller/ShapeFunction.h"
#include "../../src/FEM/Controller/IntegrationConstant.h"


using namespace PANSFEM2;


int main() {
	//----------Model Path----------
	std::string model_path = "sample/optimize2/";
	
	//----------Add Nodes----------
	std::vector<Vector<double> > nodes;
	ImportNodesFromCSV(nodes, model_path + "Node.csv");
	
	//----------Add Elements----------
	std::vector<std::vector<int> > elements;
	ImportElementsFromCSV(elements, model_path + "Element.csv");
	
	//----------Add Field----------
	std::vector<int> field;
	int KDEGREE = 0;
	ImportFieldFromCSV(field, KDEGREE, nodes.size(), model_path + "Field.csv");

	//----------Add Dirichlet Condition----------
	std::vector<int> isufixed;
	std::vector<double> ufixed;
	ImportDirichletFromCSV(isufixed, ufixed, field, model_path + "Dirichlet.csv");

	//----------Add Neumann Condition----------
	int n = 5;
	std::vector<std::vector<int> > isqfixed = std::vector<std::vector<int> >(n);
	std::vector<std::vector<double> > qfixed = std::vector<std::vector<double> >(n);
	for(int l = 0; l < n; l++){
		ImportNeumannFromCSV(isqfixed[l], qfixed[l], field, model_path + "Neumann" + std::to_string(l) + ".csv");
	}
	
	//----------Initialize design variables----------
	std::vector<double> s = std::vector<double>(elements.size(), 0.5);

	//----------Define design parameters----------
	double E0 = 0.001;
	double E1 = 210000.0;
	double Poisson = 0.3;
	double p = 3.0;

	double iota = 0.75;
	double lambdamin = 1.0e-15;
	double lambdamax = 1.0e15;
	double lambdaeps = 1.0e-10;
	double movelimit = 0.15;

	double weightlimit = 0.25;
	double objectivebefore = 0.0;
	double objectiveeps = 1.0e-5;
	
	//----------Optimize loop----------
	for(int k = 0; k < 200; k++){
		std::cout << "k = " << k << "\t";

		//**************************************************
		//	Excute direct analysis
		//**************************************************

		//----------Assembling----------
		LILCSR<double> K = LILCSR<double>(KDEGREE, KDEGREE);
		for (int i = 0; i < elements.size(); i++) {
			double E = E1 * pow(s[i], p) + E0 * (1.0 - pow(s[i], p));
			Matrix<double> Ke;
			PlaneStrain<double, ShapeFunction8Square, Gauss9Square >(Ke, nodes, elements[i], E, Poisson, 1.0);
			Assembling(K, Ke, elements[i], field);
		}

		//----------Set Dirichlet Boundary Condition----------
		SetDirichlet(K, isufixed, ufixed, 1.0e10);

		//----------Solve for each Neumann conditions----------
		std::vector<std::vector<Vector<double> > > u = std::vector<std::vector<Vector<double> > >(n);
		for(int l = 0; l < n; l++){
			std::vector<double> F = std::vector<double>(KDEGREE, 0.0);

			//----------Set Neumann Boundary Condition----------
			SetNeumann(F, isqfixed[l], qfixed[l]);

			//----------Solve System Equation----------
			CSR<double> Kmod = CSR<double>(K);
			std::vector<double> result = ScalingCG(Kmod, F, 100000, 1.0e-10);

			//----------Post Process----------
			FieldResultToNodeValue(result, u[l], field);	
		}	

		//----------Save file----------
		std::ofstream fout(model_path + "result" + std::to_string(k) + ".vtk");
		MakeHeadderToVTK(fout);
		AddPointsToVTK(nodes, fout);
		AddElementToVTK(elements, fout);
		std::vector<int> et = std::vector<int>(elements.size(), 23);
		AddElementTypes(et, fout);
		AddPointVectors(u[0], "u0", fout, true);
		for(int l = 1; l < n; l++){
			AddPointVectors(u[l], "u" + std::to_string(l), fout, false);
		}
		AddElementScalers(s, "s", fout, true);
		fout.close();


		//**************************************************
		//	Get sensitivity and update design variables
		//**************************************************
		double objective = 0.0;															//Function value of compliance
		std::vector<double> dobjectives = std::vector<double>(elements.size(), 0.0);	//Sensitivities of compliance
		double weight = 0.0;															//Function values of weight
		std::vector<double> dweights = std::vector<double>(elements.size());			//Sensitivities of weight

		//----------Get function values and sensitivities----------
		std::vector<double> compliances = std::vector<double>(n, 0.0);
		std::vector<std::vector<double> > dcompliances = std::vector<std::vector<double> >(n, std::vector<double>(elements.size()));
		for (int i = 0; i < elements.size(); i++) {
			Matrix<double> Ke;
		    PlaneStrain<double, ShapeFunction8Square, Gauss9Square >(Ke, nodes, elements[i], 1.0, Poisson, 1.0);

			for(int l = 0; l < n; l++){
				Vector<double> ue = Vector<double>();
				for(int j = 0; j < elements[i].size(); j++){
					ue = ue.Vstack(u[l][elements[i][j]]);
				}
				double ueKeue = (ue.Transpose()*Ke*ue)(0);

				compliances[l] += (E1 * pow(s[i], p) + E0 * (1.0 - pow(s[i], p))) * ueKeue;
				dcompliances[l][i] = p * (E1 * pow(s[i], p - 1.0) - E0 * pow(s[i], p - 1.0)) * ueKeue;
			}
			
			weight += s[i] - weightlimit;
			dweights[i] = 1.0;
		}

		//----------Get objective function values----------
		double tmp = 0.0;
		for(int l = 0; l < n; l++){
			tmp += 1.0/compliances[l];
		}
		objective = (double)n/tmp;
		for(int i = 0; i < elements.size(); i++){
			for(int l = 0; l < n; l++){
				dobjectives[i] += dcompliances[l][i]/pow(compliances[l], 2.0);
			}
			dobjectives[i] *= (double)n/pow(tmp, 2.0);
		}

		//----------Check convergence----------
		if(fabs((objective - objectivebefore) / (objective + objectivebefore)) < objectiveeps) {
			std::cout << std::endl << "----------Convergence----------" << std::endl;
			break;
		}

		std::cout << "Objective:\t" << objective << "(";
		for(int l = 0; l < n; l++){
			std::cout << compliances[l] << "\t";
		}
		std::cout << ")\tWeight:\t" << weight << "\t";

		//----------Get updated design variables with OC method----------
		double lambda0 = lambdamin, lambda1 = lambdamax, lambda;
		std::vector<double> snext = std::vector<double>(elements.size());
		while((lambda1 - lambda0) / (lambda1 + lambda0) > lambdaeps){
			lambda = 0.5 * (lambda1 + lambda0);

			for (int i = 0; i < elements.size(); i++) {
				snext[i] = pow(dobjectives[i] / (dweights[i] * lambda), iota) * s[i];
				if(snext[i] < std::max(0.0, (1.0 - movelimit)*s[i])) {
					snext[i] = std::max(0.0, (1.0 - movelimit)*s[i]);
				} else if(snext[i] > std::min(1.0, (1.0 + movelimit)*s[i])) {
					snext[i] = std::min(1.0, (1.0 + movelimit)*s[i]);
				}
			}

			double weightnext = 0.0;
			for (int i = 0; i < elements.size(); i++) {
				weightnext += snext[i] - weightlimit;
			}

			if (weightnext > 0.0) {
				lambda0 = lambda;
			}
			else {
				lambda1 = lambda;
			}
		}

		std::cout << "Lagrange value:\t" << lambda << std::endl;

		//----------Update design variables and objective----------
		for (int i = 0; i < elements.size(); i++) {
			s[i] = snext[i];
		}
		objectivebefore = objective;
	}
	
	return 0;
}