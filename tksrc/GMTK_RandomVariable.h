/* 
 * GMTK_RandomVariable.h
 * Defines the functions that all random variables must satisfy in order
 * for the inference, sampling, and other generic routines to work.
 *
 * Written by Geoffrey Zweig <gzweig@us.ibm.com>
 * 
 * Copyright (c) 2001, < fill in later >
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any non-commercial purpose
 * and without fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The University of Washington,
 * Seattle make no representations about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 *
 */ 


#ifndef GMTK_RANDOMVARIABLE
#define GMTK_RANDOMVARIABLE

#include "error.h"
#include "sArray.h"
#include "logp.h"

/* The random variable class defines the basic functions that random
   variables must implement. For speed, there are some special non-virtual
   functions for discrete and deterministic variables, which can be handled
   with table lookups.
*/

struct RandomVariable
{
    // Each variable can have some predecessors and successors.
    // The values stored in the Parent and Child arrays define the 
    // graph topology.
    // When switching parents are present, the graph topology is taken to
    // contain the union of all parent relations.
    // Induced cycles are prevented by requiring that for some given node
    // ordering, all the switching parents of a variable are lower-numbered
    // that the variable itself.
    // This means that a static graph with a predetermined topological order
    // can be compiled, even when there is switching parentage.
    sArray<RandomVariable *> Parent, Child;

    /* These next members are used to control the inference loops in a
       clique tree. (The constituents of each clique are random variables.)
    */

    bool hidden;
    // If a variable is not hidden, inference has no choices to make about
    // its value.

    bool discrete;
    // If not hidden then inference has no choices to make.
    // If hidden and discrete, inference will loop over all possible values.
    // If hidden and continuous, something else is done.

    bool deterministic;
    // A special case of discrete variables, where the parents' values 
    // uniquely determine the variable's value. No looping required.

    int num_vals;
    // in the discrete case, how many possible values are there?
    // values range from 0 to num_vals-1

    virtual logp probGivenParents() {error("probGivenParents() Undefined\n");}
    // For continuous variables.
    // The inference algorithm guarantees that when this is called, the
    // variable and its parents will be clamped to appropriate values.
    // The virtualness of this function is expected to be negligible comared
    // to the other work done for continuous variables.

    inline logp discreteProbGivenParents();
    // For discrete variables.
    // The inference algorithm guarantees that when this is called, the
    // variable and its parents will be clamped to appropriate values.
    // This does a table lookup in the discrete case.  

/* Add a pointer to the appropriate GMTK_DiscretePDF table here */
/* Then discreteProbGivenParents() will follow it. */

    virtual void makeRandom() = 0;
    // Sets the parameters determining probGivenParents() to random values.

    virtual void makeUniform() = 0;
    // Sets the parameters determining probGivenParents() to uniform values.

    virtual void instantiate() = 0;
    // Sets the variable according to the probability distribution determined
    // by its parent's values.
    // Used for simulation.

    virtual void tieWith(randomVariable *rv) = 0;
    // In a DGM, the analogous occurrences of a variable in different time
    // slices all share the same distributions and accumulators. This function
    // tells a variable to use the data structures associated with just one
    // member of its equivalence class.

    virtual void zeroAccumulators() = 0;
    // Called at the beginning of an EM iteration.

    virtual void increment(logp posterior) = 0;
    // On the backward pass, this will be called with a posterior probability
    // for the variable and its parents with all possible sets of values.
    // In inference, the GM is used as a global memory, with variable values
    // dynamically changed as inference proceeds. The inference algorithm
    // guarantees that the variable and its parents will have been set to
    // the the appropriate values when increment() is called, so that the
    // variable can just "look" at its parents to see what their values are.
    // Then the count of seeing the variable and its parents with those values
    // is incremented by the posterior amount.
    // For continuous variables, statistics are accumulated.

    inline void discreteIncrement();
    // For discrete variables, incrementing can be done with a table lookup.
    // Avoid a virtual function call.

/* Add a pointer to the appropriate GMTK_DiscretePDF table here */
/* Then discreteIncrement() will follow it to do the increment. */

    virtual void update() = 0;
    // At the end of each EM iteration, this is called to convert the 
    // accumulated statistics into probability distributions.
};

#endif
