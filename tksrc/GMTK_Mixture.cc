/*-
 * GMTK_MixGaussians.cc
 *        Code for mixtures of Gaussians of a variety of types.
 *
 * Written by Jeff Bilmes <bilmes@ee.washington.edu>
 *
 * Copyright (c) 2001, < fill in later >
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any non-commercial purpose
 * and without fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The University of Washington,
 * Seattle, and Jeff Bilmes make no representations about
 * the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 */


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ieeefp.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

#include <string>

#include "general.h"
VCID("$Header$");
#include "error.h"
#include "rand.h"

#include "GMTK_MixGaussians.h"
#include "GMTK_GMParms.h"



void
MixGaussians::read(iDataStreamFile& is)
{
  // read name of self
  NamedObject::read(is);
  
  // read number of mixtures
  is.read(numComponents,"MixGaussians::read numComponents");
  if (numComponents <= 0) 
    error("Error: MixGaussians '%s' has number of components = %d\n",
	  _name.c_str(),numComponents);

  // read name of dense 1d PMF to use for mixtures
  string str;
  is.read(str);
  if (GM_Parms.dPmfsMap.find(str) == GM_Parms.dPmfsMap.end()) {
    error("Error: MixGaussians '%s', can't find PMF named '%s'\n",
	  _name.c_str(),str.c_str());
  }

  dense1DPMF = GM_Parms.dPmfs[
			      GM_Parms.dPmfsMap[str]
  ];

  // now make sure that this one matches the number of components.
  if (numComponents != dense1DPMF->length()) {
    error("Error: MixGaussians '%s', PMF named '%s' has %d elements but we need %d\n",
	  _name.c_str(),
	  str.c_str(),dense1DPMF->length(),numComponents);
  }

  // now read 'name' pointers to all the Gaussians.
  components.resize(numComponents);
  for (unsigned i=0;i<numComponents;i++) {
    is.read(str);
    if (GM_Parms.gaussianComponentsMap.find(str) == GM_Parms.gaussianComponentsMap.end()) {
      error("Error: MixGaussians '%s', can't find Gaussian Component named '%s'\n",_name.c_str(),str.c_str());
    }
    GaussianComponent*gc = GM_Parms.gaussianComponents [
	GM_Parms.gaussianComponentsMap[str]
    ];
    components[i] = gc;
  }
  // make ready for probability evaluation.
}

void
MixGaussians::write(oDataStreamFile& os)
{
  error("MixGaussians::write not implemented");
  NamedObject::write(os);
}



void
MixGaussians::makeUniform()
{
  dense1DPMF->makeUniform();
  for (unsigned i=0;i<numComponents;i++) {
    components[i]->makeUniform();
  }
}


void
MixGaussians::makeRandom()
{
  dense1DPMF->makeRandom();
  for (unsigned i=0;i<numComponents;i++) {
    components[i]->makeRandom();
  }
}


//
// compute the log probability of x with stride 'stride'
// 
logpr
MixGaussians::log_p(const float *const x,
		    const Data32* const base,
		    const int stride)
{
  logpr rc;
  rc.set_to_zero();
  for (unsigned i=0;i<numComponents;i++) {
    rc += dense1DPMF->p(i)* components[i]->log_p(x,base,stride);
  }
  return rc;
}



/////////////////
// EM routines //
/////////////////


void
MixGaussians::emStartIteration()
{
  if (!GM_Parms.amTrainingMixGaussians())
    return;
  if(emOnGoingBitIsSet())
    return; // already done

  if (!emEmAllocatedBitIsSet()) {
    // this is presumably the first time
    postDistribution.resize(components.size());
    emSetEmAllocatedBit();
  }

  // EM iteration is now going.
  emSetOnGoingBit();
  emSetSwappableBit();

  accumulatedProbability = 0.0;
  dense1DPMF->emStartIteration();
  for (unsigned i=0;i<numComponents;i++) {
    components[i]->emStartIteration();
  }
}


void
MixGaussians::emIncrement(logpr prob,
			  const float *f,
			  const Data32* const base,
			  const int stride)
{
  if (!GM_Parms.amTrainingMixGaussians())
    return;  

  emStartIteration();

  if (prob.val() < log_FLT_MIN) {
    return;
  } 
  accumulatedProbability+= prob;

  // first compute the local Gaussian mixture posterior distribution.
  logpr sum;
  sum.set_to_zero();
  for (unsigned i=0;i<numComponents;i++) {
    postDistribution[i] = 
      dense1DPMF->p(i)* components[i]->log_p(f,base,stride);
    sum += postDistribution[i];
  }

  logpr tmp = prob/sum;
  for (unsigned i=0;i<numComponents;i++) {
    postDistribution[i] =
      postDistribution[i]*tmp;
  }

  // increment the mixture weights
  dense1DPMF->emIncrement(prob,postDistribution);

  // and the components themselves.
  for (unsigned i=0;i<numComponents;i++) {
    components[i]->emIncrement(postDistribution[i],
			       f,base,stride);
  }

}


void
MixGaussians::emEndIteration()
{

  if (!GM_Parms.amTrainingMixGaussians())
    return;

  if ( !emOnGoingBitIsSet() )
    return; // done already

  if (accumulatedProbability.zero()) {
    warning("WARNING: Diagonal covariance vector named '%s' did not receive any accumulated probability in EM iteration",name().c_str());
  }

  dense1DPMF->emEndIteration();

  for (unsigned i=0;i<numComponents;i++)
    components[i]->emEndIteration();

  // stop EM
  emClearOnGoingBit();
}



void
MixGaussians::emSwapCurAndNew()
{

  if (!GM_Parms.amTrainingMixGaussians())
    return;

  if (!emSwappableBitIsSet())
      return;

  dense1DPMF->emSwapCurAndNew();
  for (unsigned i=0;i<numComponents;i++) {
    components[i]->emSwapCurAndNew();
  }
  emClearSwappableBit();
}




void
MixGaussians::emStoreAccumulators(oDataStreamFile& ofile)
{
  error("not implemented");
}

void
MixGaussians::emLoadAccumulators(iDataStreamFile& ifile)
{
  error("not implemented");
}


void
MixGaussians::emAccumulateAccumulators(iDataStreamFile& ifile)
{
  error("not implemented");
}


////////////////////////////////////////////////////////////
// Sample generation
////////////////////////////////////////////////////////////



void MixGaussians::sampleGenerate(float *const sample,
				  const Data32* const base,
				  const int stride)
{
  error("not implemented");
}









