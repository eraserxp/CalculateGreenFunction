/*
 * formMatrix.h
 *
 *  Created on: Jun 17, 2014
 *      Author: pxiang
 */

#ifndef FORMMATRIX_H_
#define FORMMATRIX_H_

#include "../basis_set/basis.h"
#include "../Utility/misc.h"
#include "../Utility/random_generator.h"
#include "../IO/binaryIO.h"

/* this InteractionData struct contains no information about the size of the lattice*/
// the lattice geometry information is stored in class LatticeShape
/**
 * InteractionData stores the onsite energy, the strengths of hopping
 * and dynamic interactions. It also specifies whether the interactions
 * are random or not, and contains the seed for generating the random
 * interactions and the effective range of hopping and dynamic interactions
 */
typedef struct {
	double onsiteE, hop, dyn;
	bool randomHop, randomDyn, randomOnSite;
	int maxDistance; // beyond which the interaction is set to zero
	unsigned seed;
} InteractionData;



/** form the matrix describing the interaction between sites
 * hop(index1,index2) is the hopping interaction
 * dyn(index1,index2) is the dynamic interaction
 * onsiteE(index) is the on-site energy of site i
 * for 1D case: lattice index (x) = index
 * for 2D case: y*(xmax+1) + x = index
 * maxdis --- max distance beyond which the interaction is zero
 */

class Interaction {
public:
	Interaction(LatticeShape& lattice, InteractionData& interactionData) {
		dim = lattice.getDim();
		seed = interactionData.seed;
		rng.SetSeed(seed);
		if (dim==1) {
			int xsite = lattice.getXmax() + 1;
			maxDistance = interactionData.maxDistance;
			// initialize the hopping matrix
			t = DMatrix::Zero(xsite,xsite);
			if (interactionData.randomHop) {
				setRandomMatrix(t, interactionData.hop, maxDistance);
			} else {
				// set all element to constant
				setConstantMatrix(t, interactionData.hop, maxDistance);
			}

			// initialize the dynamic matrix
			d = DMatrix::Zero(xsite,xsite);
			if (interactionData.randomDyn) {
				setRandomMatrix(d, interactionData.dyn, maxDistance);
			} else {
				// set all element to constant
				setConstantMatrix(d, interactionData.dyn, maxDistance);
			}

			// initialize the dynamic matrix
			e = DVector::Zero(xsite);
			if (interactionData.randomOnSite) {
				setRandomVector(e, interactionData.onsiteE);
			} else {
				// set all element to constant
				setConstantVector(e, interactionData.onsiteE);
			}
		} //end of if for dim=1


	}

	double hop(Basis& basis1, Basis& basis2) {
		double result;
		// for 1D case
		switch (dim) {
		case 1:
			if (basis1[0]==basis2[0]) {
				result = t(basis1[1], basis2[1]);
			} else if (basis1[1]==basis2[1]) {
				result = t(basis1[0], basis2[0]);
			} else {
				result = 0.0;
			}
			break;
		case 2:
		case 3:
		default:
			result = 0.0;
		}
		return result;
	}

	double dyn(Basis& basis) {
		double result;
		switch (dim) {
		case 1:
			result = d(basis[0], basis[1]);
			break;
		case 2:
		case 3:
		default:
			result = 0.0;
		}
		return result;
	}

	double onsiteE(Basis& basis) {
		double result;
		switch (dim) {
		case 1:
			result = e(basis[0]) +  e(basis[1]);
			break;
		case 2:
		case 3:
		default:
			result = 0.0;
		}
		return result;
	}

	// if the seed is not set, use the default one
	void setRandomSeed(unsigned inputSeed) {
		seed = inputSeed;
		rng.SetSeed(seed);
	}

	// obtain the range of interactions
	int getMaxDistance() {
		return maxDistance;
	}

	// destructor
	~Interaction() {
		// release the memory of the matrices
		t.resize(0,0);
		d.resize(0,0);
		e.resize(0);
	}

private:
	DMatrix t;
	DMatrix d;
	DVector e;
	int maxDistance;
	int dim;
	unsigned seed;
	RandomNumberGenerator rng;

	void setRandomMatrix(DMatrix& m, double maxVal, int maxDistance) {
		int xsite = m.rows();
		for (int i=0; i<xsite-1; ++i) {
			for (int incr=1; incr<=maxDistance && i+incr<xsite; ++incr) {
				m(i,i+incr) = maxVal*rng.randomReal()/std::pow(incr,3.0); // range [0, hop)
				m(i+incr,i) = t(i,i+incr);
			}
		}
	}

	void setConstantMatrix(DMatrix& m, double maxVal, int maxDistance) {
		int xsite = m.rows();
		for (int i=0; i<xsite-1; ++i) {
			for (int incr=1; incr<=maxDistance && i+incr<xsite; ++incr) {
				m(i,i+incr) = maxVal/std::pow(incr,3.0); // range [0, hop)
				m(i+incr,i) = t(i,i+incr);
			}
		}
	}

	void setRandomVector(DVector& v, double maxVal) {
		for (int i=0; i<v.size(); ++i) {
			v(i) = maxVal*rng.randomReal();
		}
	}

	void setConstantVector(DVector& v, double maxVal) {
		for (int i=0; i<v.size(); ++i) {
			v(i) = maxVal;
		}
	}

};

// a pointer to an Interaction object
extern Interaction *pInteraction;
extern LatticeShape *pLattice;

void getMSize(int K, int Kp, int& rows, int& cols);

void getZSize(int K, int& rows, int& cols);

void setInteractions(LatticeShape& lattice, InteractionData& interactionData);

void formMatrixZ(int K, dcomplex Energy, CDMatrix& ZK);

void formMatrixM(int K, int Kp, CDMatrix& MKKp);

void formMatrixW(int K, dcomplex energy, CDMatrix& WK);

void formMatrixAlpha(int K, CDMatrix& AlphaK);

void formMatrixBeta(int K, CDMatrix& BetaK);

#endif /* FORMMATRIX_H_ */