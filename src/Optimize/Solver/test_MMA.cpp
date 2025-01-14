#include <iostream>
#include <vector>
#include <cmath>


#include "MMA.h"


using namespace PANSFEM2;


int main() {
    //----------Set constant----------
    double C1 = 0.0624;
    double C2 = 1.0;

	//----------Initialize optimization solver----------
	std::vector<double> s = std::vector<double>(5, 5.0);
	MMA<double> optimizer = MMA<double>(5, 1, 
		1.0, 
		std::vector<double>(1, 0.0), 
		std::vector<double>(1, 1000.0), 
		std::vector<double>(1, 1.0), 
		std::vector<double>(5, 1.0), 
		std::vector<double>(5, 10.0)
	);
	optimizer.SetParameters(1.0e-5, 0.1, 0.5, 0.5, 0.7, 1.2, 1.0e-5);
	
	//----------Optimize loop----------
	for(int k = 0; k < 10; k++){
		std::cout << std::endl << "k = " << k << "\t";

		//**************************************************
		//	Update design variables
		//**************************************************
		double objective;											                                        //Function value of compliance
		std::vector<double> dobjective = std::vector<double>(5);               								//Sensitivities of compliance
		std::vector<double> constraints = std::vector<double>(1);											//Function values of weight
		std::vector<std::vector<double> > dconstraints = std::vector<std::vector<double> >(1, std::vector<double>(5));	    //Sensitivities of weight

		//----------Get function values and sensitivities----------
		//  Objective
        objective = C1*(s[0] + s[1] + s[2] + s[3] + s[4]); 
        dobjective[0] = C1;
        dobjective[1] = C1;
        dobjective[2] = C1;
        dobjective[3] = C1;
        dobjective[4] = C1;

        //  Constraint
        constraints[0] = 61.0*pow(s[0], -3.0) + 37.0*pow(s[1], -3.0) + 19.0*pow(s[2], -3.0) + 7.0*pow(s[3], -3.0) + 1.0*pow(s[4], -3.0) - C2;
        dconstraints[0][0] = -3.0*61.0*pow(s[0], -4.0);
        dconstraints[0][1] = -3.0*37.0*pow(s[1], -4.0);
        dconstraints[0][2] = -3.0*19.0*pow(s[2], -4.0);
        dconstraints[0][3] = -3.0*7.0*pow(s[3], -4.0);
        dconstraints[0][4] = -3.0*1.0*pow(s[4], -4.0); 

		std::cout << "Objective:\t" << objective << "\t";
		std::cout << "Constraints:\t" << constraints[0] << "\t";

        for(auto si : s){
            std::cout << "\t" << si; 
        }	

		//----------Check convergence----------
		if(optimizer.IsConvergence(objective)){
			std::cout << std::endl << "--------------------Optimized--------------------" << std::endl;
			break;
		}

		//----------Update s----------
		optimizer.UpdateVariables(s, objective, dobjective, constraints, dconstraints);
	}

	return 0;
}