#include <iostream>
#include <vector>
#include <cmath>


#include "Lanczos.h"


int main() {
	/*CSR<double> A = CSR<double>(10, 10);
	A.set(0, 0, 10.0);	A.set(0, 1, 9.0);	A.set(0, 2, 8.0);	A.set(0, 3, 7.0);	A.set(0, 4, 6.0);	A.set(0, 5, 5.0);	A.set(0, 6, 4.0);	A.set(0, 7, 3.0);	A.set(0, 8, 2.0);	A.set(0, 9, 1.0);	
	A.set(1, 0, 9.0);	A.set(1, 1, 9.0);	A.set(1, 2, 8.0);	A.set(1, 3, 7.0);	A.set(1, 4, 6.0);	A.set(1, 5, 5.0);	A.set(1, 6, 4.0);	A.set(1, 7, 3.0);	A.set(1, 8, 2.0);	A.set(1, 9, 1.0);	
	A.set(2, 0, 8.0);	A.set(2, 1, 8.0);	A.set(2, 2, 8.0);	A.set(2, 3, 7.0);	A.set(2, 4, 6.0);	A.set(2, 5, 5.0);	A.set(2, 6, 4.0);	A.set(2, 7, 3.0);	A.set(2, 8, 2.0);	A.set(2, 9, 1.0);	
	A.set(3, 0, 7.0);	A.set(3, 1, 7.0);	A.set(3, 2, 7.0);	A.set(3, 3, 7.0);	A.set(3, 4, 6.0);	A.set(3, 5, 5.0);	A.set(3, 6, 4.0);	A.set(3, 7, 3.0);	A.set(3, 8, 2.0);	A.set(3, 9, 1.0);	
	A.set(4, 0, 6.0);	A.set(4, 1, 6.0);	A.set(4, 2, 6.0);	A.set(4, 3, 6.0);	A.set(4, 4, 6.0);	A.set(4, 5, 5.0);	A.set(4, 6, 4.0);	A.set(4, 7, 3.0);	A.set(4, 8, 2.0);	A.set(4, 9, 1.0);	
	A.set(5, 0, 5.0);	A.set(5, 1, 5.0);	A.set(5, 2, 5.0);	A.set(5, 3, 5.0);	A.set(5, 4, 5.0);	A.set(5, 5, 5.0);	A.set(5, 6, 4.0);	A.set(5, 7, 3.0);	A.set(5, 8, 2.0);	A.set(5, 9, 1.0);	
	A.set(6, 0, 4.0);	A.set(6, 1, 4.0);	A.set(6, 2, 4.0);	A.set(6, 3, 4.0);	A.set(6, 4, 4.0);	A.set(6, 5, 4.0);	A.set(6, 6, 4.0);	A.set(6, 7, 3.0);	A.set(6, 8, 2.0);	A.set(6, 9, 1.0);	
	A.set(7, 0, 3.0);	A.set(7, 1, 3.0);	A.set(7, 2, 3.0);	A.set(7, 3, 3.0);	A.set(7, 4, 3.0);	A.set(7, 5, 3.0);	A.set(7, 6, 3.0);	A.set(7, 7, 3.0);	A.set(7, 8, 2.0);	A.set(7, 9, 1.0);	
	A.set(8, 0, 2.0);	A.set(8, 1, 2.0);	A.set(8, 2, 2.0);	A.set(8, 3, 2.0);	A.set(8, 4, 2.0);	A.set(8, 5, 2.0);	A.set(8, 6, 2.0);	A.set(8, 7, 2.0);	A.set(8, 8, 2.0);	A.set(8, 9, 1.0);	
	A.set(9, 0, 1.0);	A.set(9, 1, 1.0);	A.set(9, 2, 1.0);	A.set(9, 3, 1.0);	A.set(9, 4, 1.0);	A.set(9, 5, 1.0);	A.set(9, 6, 1.0);	A.set(9, 7, 1.0);	A.set(9, 8, 1.0);	A.set(9, 9, 1.0);	
    */

	/*CSR<double> A = CSR<double>(6, 6);
	A.set(0, 0, 6.0);	A.set(0, 1, 5.0);	A.set(0, 2, 4.0);	A.set(0, 3, 3.0);	A.set(0, 4, 2.0);	A.set(0, 5, 1.0);
	A.set(1, 0, 5.0);	A.set(1, 1, 5.0);	A.set(1, 2, 4.0);	A.set(1, 3, 3.0);	A.set(1, 4, 2.0);	A.set(1, 5, 1.0);
	A.set(2, 0, 4.0);	A.set(2, 1, 4.0);	A.set(2, 2, 4.0);	A.set(2, 3, 3.0);	A.set(2, 4, 2.0);	A.set(2, 5, 1.0);
	A.set(3, 0, 3.0);	A.set(3, 1, 3.0);	A.set(3, 2, 3.0);	A.set(3, 3, 3.0);	A.set(3, 4, 2.0);	A.set(3, 5, 1.0);
	A.set(4, 0, 2.0);	A.set(4, 1, 2.0);	A.set(4, 2, 2.0);	A.set(4, 3, 2.0);	A.set(4, 4, 2.0);	A.set(4, 5, 1.0);
	A.set(5, 0, 1.0);	A.set(5, 1, 1.0);	A.set(5, 2, 1.0);	A.set(5, 3, 1.0);	A.set(5, 4, 1.0);	A.set(5, 5, 1.0);
	*/

	//*************************************************************************
	//	Eigen values are
	//		-3.2474 -2.6633  1.7511  4.1599 14.9997
	//*************************************************************************
	/*CSR<double> A = CSR<double>(5, 5);
	A.set(0, 0, 1.0);	A.set(0, 1, 2.0);	A.set(0, 2, 3.0);	A.set(0, 3, 0.0);	A.set(0, 4, 0.0);
	A.set(1, 0, 2.0);	A.set(1, 1, 2.0);	A.set(1, 2, 3.0);	A.set(1, 3, 4.0);	A.set(1, 4, 0.0);
	A.set(2, 0, 3.0);	A.set(2, 1, 3.0);	A.set(2, 2, 3.0);	A.set(2, 3, 4.0);	A.set(2, 4, 5.0);
	A.set(3, 0, 0.0);	A.set(3, 1, 4.0);	A.set(3, 2, 4.0);	A.set(3, 3, 4.0);	A.set(3, 4, 5.0);
	A.set(4, 0, 0.0);	A.set(4, 1, 0.0);	A.set(4, 2, 5.0);	A.set(4, 3, 5.0);	A.set(4, 4, 5.0);
	*/

	//*************************************************************************
	//	Eigen values are
	//		8.29086 1.00000	0.42602 0.28312
	//*************************************************************************
	/*CSR<double> A = CSR<double>(4, 4);
	A.set(0, 0, 1.0);	A.set(0, 1, 1.0);	A.set(0, 2, 1.0);	A.set(0, 3, 1.0);	
	A.set(1, 0, 1.0);	A.set(1, 1, 2.0);	A.set(1, 2, 2.0);	A.set(1, 3, 2.0);	
	A.set(2, 0, 1.0);	A.set(2, 1, 2.0);	A.set(2, 2, 3.0);	A.set(2, 3, 3.0);	
	A.set(3, 0, 1.0);	A.set(3, 1, 2.0);	A.set(3, 2, 3.0);	A.set(3, 3, 4.0);	
	*/

	CSR<double> A = CSR<double>(4, 4);
	A.set(0, 0, 1.0);	A.set(0, 1, 3.0);	A.set(0, 2, 4.0);	A.set(0, 3, 5.0);	
	A.set(1, 0, 3.0);	A.set(1, 1, 3.0);	A.set(1, 2, 5.0);	A.set(1, 3, 1.0);	
	A.set(2, 0, 4.0);	A.set(2, 1, 5.0);	A.set(2, 2, 4.0);	A.set(2, 3, 2.0);	
	A.set(3, 0, 5.0);	A.set(3, 1, 1.0);	A.set(3, 2, 2.0);	A.set(3, 3, 0.0);	


	/*CSR<double> A = CSR<double>(3, 3);
	A.set(0, 0, 3.0);	A.set(0, 1, 2.0);	A.set(0, 2, 1.0);
	A.set(1, 0, 2.0);	A.set(1, 1, 2.0);	A.set(1, 2, 1.0);
	A.set(2, 0, 1.0);	A.set(2, 1, 1.0);	A.set(2, 2, 1.0);
	*/

	/*std::cout << A << std::endl;

	std::vector<double> alpha, beta;
	LanczosProcess(A, alpha, beta);
	*/
	
	std::vector<double> alpha = { 1.0, 3.0, 2.0, 1.0 };
	std::vector<double> beta = { 2.0, 3.0, 1.0, 0.0 };
	

	double lambda0 = BisectionMethod(alpha, beta, 0);
	std::cout << lambda0 << std::endl;
	
	std::vector<double> y = InversePowerMethod(alpha, beta, lambda0);
	for(auto yi : y){
		std::cout << yi << "\t";
	}
	
	return 0;
}