#include <iostream>
#include <vector>


#include "../../src/LinearAlgebra/Models/Vector.h"
#include "../../src/PrePost/Import/ImportFromCSV.h"
#include "../../src/FEM/Equation/Stokes.h"
#include "../../src/FEM/Equation/NavierStokes.h"
#include "../../src/FEM/Controller/ShapeFunction.h"
#include "../../src/FEM/Controller/GaussIntegration.h"
#include "../../src/FEM/Controller/BoundaryCondition.h"
#include "../../src/FEM/Controller/Assembling.h"
#include "../../src/LinearAlgebra/Solvers/CG.h"
#include "../../src/PrePost/Export/ExportToVTK.h"



using namespace PANSFEM2;


int main() {
    std::string model_path = "sample/navierstokes/model3/";
	std::vector<Vector<double> > x;
	ImportNodesFromCSV(x, model_path + "Node.csv");
	std::vector<std::vector<int> > elementsu;
	ImportElementsFromCSV(elementsu, model_path + "ElementU.csv");
	std::vector<std::vector<int> > elementsp;
	ImportElementsFromCSV(elementsp, model_path + "ElementP.csv");
    std::vector<std::pair<std::pair<int, int>, double> > ufixed;
	ImportDirichletFromCSV(ufixed, model_path + "Dirichlet.csv");

    std::vector<Vector<double> > up = std::vector<Vector<double> >(x.size(), Vector<double>(3));
	std::vector<std::vector<int> > nodetoglobal = std::vector<std::vector<int> >(x.size(), std::vector<int>(3, 0));
	
    double rho = 1.0;
    double mu = 1.0/100.0;
    int kmax = 10;
	std::vector<std::pair<std::pair<int, int>, double> > ufixed0 = ufixed;
    for(auto& ufixedi : ufixed0) {
        ufixedi.second *= 1.0/(double)kmax;
    }
    SetDirichlet(up, nodetoglobal, ufixed0);
	int KDEGREE = Renumbering(nodetoglobal);

    //----------Get initial result with Stokes equation----------
    LILCSR<double> K = LILCSR<double>(KDEGREE, KDEGREE); 
    std::vector<double> F = std::vector<double>(KDEGREE, 0.0);

    for (int i = 0; i < elementsu.size(); i++) {
        std::vector<std::vector<std::pair<int, int> > > nodetoelementu, nodetoelementp;
        Matrix<double> Ke;
        Stokes<double, ShapeFunction8Square, ShapeFunction4Square, Gauss9Square>(Ke, nodetoelementu, elementsu[i], nodetoelementp, elementsp[i], { 0, 1, 2 }, x, mu);
        Assembling(K, F, up, Ke, nodetoglobal, { nodetoelementu, nodetoelementp }, { elementsu[i], elementsp[i] });
    }

    CSR<double> Kmod = CSR<double>(K);
    std::vector<double> result = CG(Kmod, F, 100000, 1.0e-10);
    Disassembling(up, result, nodetoglobal);
    
    //----------Incremental step loop----------
    for(int k = 1; k < kmax; k++) {
        std::cout << "k=" << k;

        std::vector<std::pair<std::pair<int, int>, double> > ufixedk = ufixed;
        for(auto& ufixedi : ufixedk) {
            ufixedi.second *= (k + 1)/(double)kmax;
        }
        SetDirichlet(up, nodetoglobal, ufixedk);

        //----------Newton-Raphson loop----------
        for(int l = 0; l < 100; l++) {
            LILCSR<double> K = LILCSR<double>(KDEGREE, KDEGREE);			//System stiffness matrix
            std::vector<double> R = std::vector<double>(KDEGREE, 0.0);		//Residual load vector
            std::vector<Vector<double> > dup = std::vector<Vector<double> >(x.size(), Vector<double>(3));

            for (int i = 0; i < elementsu.size(); i++) {
                std::vector<std::vector<std::pair<int, int> > > nodetoelementu, nodetoelementp;
                Matrix<double> Ke;
                Vector<double> Qe;
                NavierStokesTangent<double, ShapeFunction8Square, ShapeFunction4Square, Gauss9Square>(Ke, Qe, nodetoelementu, elementsu[i], nodetoelementp, elementsp[i], { 0, 1, 2 }, x, up, rho, mu);
                Assembling(K, R, dup, Ke, nodetoglobal, { nodetoelementu, nodetoelementp }, { elementsu[i], elementsp[i] });
                Assembling(R, Qe, nodetoglobal, nodetoelementu, elementsu[i]);
                Assembling(R, Qe, nodetoglobal, nodetoelementp, elementsp[i]);  
            }
  
            double normR = std::inner_product(R.begin(), R.end(), R.begin(), 0.0);
            std::cout << "\t\tl = " << l << "\tR Norm = " << normR << std::endl;
            if (normR < 1.0e-10) {
                std::cout << "\tConvergence at l = " << l << "\tR Norm = " << normR << std::endl;
                break;
            }
            
            CSR<double> Kmod = CSR<double>(K);
            std::vector<double> result = BiCGSTAB2(Kmod, R, 100000, 1.0e-10);
            Disassembling(dup, result, nodetoglobal);
            for(int i = 0; i < x.size(); i++){
                up[i] += dup[i];
            }
        }
    }

    std::vector<Vector<double> > u = std::vector<Vector<double> >(x.size());
    std::vector<double> p = std::vector<double>(x.size(), 0.0);
    for(int i = 0; i < x.size(); i++){
        u[i] = up[i].Segment(0, 2);
        p[i] = up[i](2);
    }

    std::ofstream fout(model_path + "result.vtk");
    MakeHeadderToVTK(fout);
    AddPointsToVTK(x, fout);
    AddElementToVTK(elementsp, fout);
    AddElementTypes(std::vector<int>(elementsp.size(), 5), fout);
    AddPointVectors(u, "u", fout, true);
    AddPointScalers(p, "p", fout, false);
    fout.close();
    
	return 0;
}