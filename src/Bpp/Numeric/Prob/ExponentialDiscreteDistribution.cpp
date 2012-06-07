//
// File: ExponentialDiscreteDistribution.cpp
// Created by: Julien Dutheil
// Created on: Tue Nov 13 12:37 2007
//

/*
   Copyright or © or Copr. Bio++ Development Team, (November 17, 2004)

   This software is a computer program whose purpose is to provide classes
   for numerical calculus.

   This software is governed by the CeCILL  license under French law and
   abiding by the rules of distribution of free software.  You can  use,
   modify and/ or redistribute the software under the terms of the CeCILL
   license as circulated by CEA, CNRS and INRIA at the following URL
   "http://www.cecill.info".

   As a counterpart to the access to the source code and  rights to copy,
   modify and redistribute granted by the license, users are provided only
   with a limited warranty  and the software's author,  the holder of the
   economic rights,  and the successive licensors  have only  limited
   liability.

   In this respect, the user's attention is drawn to the risks associated
   with loading,  using,  modifying and/or developing or reproducing the
   software by the user in light of its specific status of free software,
   that may mean  that it is complicated to manipulate,  and  that  also
   therefore means  that it is reserved for developers  and  experienced
   professionals having in-depth computer knowledge. Users are therefore
   encouraged to load and test the software's suitability as regards their
   requirements in conditions enabling the security of their systems and/or
   data to be ensured and,  more generally, to use and operate it in the
   same conditions as regards security.

   The fact that you are presently reading this means that you have had
   knowledge of the CeCILL license and that you accept its terms.
 */

#include "ExponentialDiscreteDistribution.h"
#include "../Random/RandomTools.h"
#include "../NumConstants.h"
#include "../../Utils/MapTools.h"

using namespace bpp;

// From the STL:
#include <cmath>
using namespace std;

/****************************************************************/

ExponentialDiscreteDistribution::ExponentialDiscreteDistribution(unsigned int n, double lambda) :
  AbstractParameterAliasable("Exponential."),
  AbstractDiscreteDistribution(n, "Exponential."),
  lambdaConstraint_(0),
  lambda_(lambda)
{
  lambdaConstraint_ = new IntervalConstraint(1, 0.0, true);
  Parameter p("Exponential.lambda", lambda, dynamic_cast<Constraint*>(lambdaConstraint_->clone()), true);
  addParameter_(p);

  intMinMax_.setLowerBound(0, true);
  discretize();
}

ExponentialDiscreteDistribution::~ExponentialDiscreteDistribution()
{
  delete lambdaConstraint_;
}

/******************************************************************************/

void ExponentialDiscreteDistribution::fireParameterChanged(const ParameterList& parameters)
{
  AbstractDiscreteDistribution::fireParameterChanged(parameters);
  lambda_ = getParameterValue("lambda");
  discretize();
}



