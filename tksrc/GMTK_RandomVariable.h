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

#include "sArray.h"
#include "logp.h"

/* The random variable class defines the basic functions that random
   variables must implement. 

   Switching parents are guaranteed to be discrete.
   A nodes conditioning parents must be lower indexed than it.
*/


//////////////////////////////////////////////////////////////////////////////
// This is the integer type of the values that a discrete random variable
// may take on. Possibilities include unsigned char, char, short, int, 
// unsigned long, and so on.
#define DISCRETE_VARIABLE_TYPE Int16


class RandomVariable
{
public:

    int nodeNum;  
    // A unique node index. nodes in a GM are numbered consecutively from 0.

    int timeIndex;
    // What time frame does the node belong to?
    // Counting starts from 0.

    bool hidden;
    // Is the node hidden, or is it an observation.

    bool discrete;
    // Is the variable discrete?
    // Inference conditions on this; cliques keep track of the values of 
    // their discrete members.

    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////
    // SUPPORT ONLY FOR DISCRETE RANDOM VARIABLES ////////////
    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////

// Q: Ideally this stuff should be moved to the child class.
// After that is finished, see if we can move it there.

    DISCRETE_VARIABLE_TYPE val;
    // in the discrete case, the actual value of the variable.
    // Cliques keep track of the values of their discrete members.

    const int cardinality;
    // Again in the discrete case.
    // The maximum number of possible values this RV may take in, irrespective
    // of the values of the parents. So, we must
    // have that 0 <= val < cardinality
    sArray<RandomVariable *> switchingParents, *curConditionalParents;

    // The probability of a variable taking a particular value depends on
    // the values of its parents. There come in two varieties: switching 
    // parents and conditional parents. The values of the switching parents
    // determines an additional set of conditional parents. The variable
    // is conditioned on these conditional parents. Note that a switching 
    // parent may also be a conditional parent.
    // A node's switching and conditional parents must all be lower numbered 
    // than the node itself, thus allowing a topological ordering of 

    sArray<RandomVariable *> allPossibleParents;
    // allPossibleParents is the union of the switchingParents and the
    // all possible conditionalParents.
    // Used to determine topological orderings

    sArray< sArray < RandomVariable* > > conditionalParentsList;
    // For each possible different list of
    // conditional parents that might exist for all possible
    // values of the switching parents, this array gives that list of
    // appropriate conditional parents. For example, 
    // suppose that S is the set of conditional parents, and that
    // 0 <= S <= 5 corresponds to one set of conditional parents,
    // and 6 <= S < = 10 corresponds to another set of conditional
    // parents, and those are the only two set of conditional parents
    // that exist for all values of the switching parents, this
    // list is of size two. 

    void setParents(sArray<RandomVariable *> &sparents, 
         sArray<sArray<RandomVariable *> > &cpl);
    // A method to make it easy to set up the graph topology
    

    sArray<RandomVariable *> allPossibleChildren;
    // The set of variables that use this variable either as a switching
    // parent or as a (possible) conditional parent.

    int intFromSwitchingState();
    // Looks at the currently (presumably clamped) values of the
    // switching parents and returns an 1D integer index. 
    // This indexes the set of conditional parents to use.

    virutal void findConditionalParents() = 0;
    // Looks at the values of the switching parents, and sets the 
    // conditionalParents array pointer appropriately.

    void findAllPossibleParents();
    // Iterates through all possible instantiations of the conditional
    // parents and unions together the parents. Stores the result in the
    // allPossibleParents array (in what order??)

    int address(sArray<RandomVariable *>);
    // Looks up the values of a vector of parents and computes an
    // integer index into a 1 dimensional array. 
    //  (not sure what this is for??)

    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////
    // END OF SUPPORT ONLY FOR DISCRETE RANDOM VARIABLES /////
    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////

    virtual logpr probGivenParents() = 0;
    // The inference algorithm guarantees that when this is called, the
    // variable and its parents will be clamped to appropriate values.
    // This also assumes that findConditionalParents has been called.

    virtual void clampFirstValue() = 0;
    // Sets a variable to the first value that is possible.
    // Values with 0 probability can be ignored, as long as some value is set.
    // This function must always do a clamping.
    // Observation variables (already clamped do nothing).
    // It is assumed that the parent values are clamped at this point.

    virtual bool clampNextValue() = 0;
    // Sets a variable to the next possible value.
    // Returns false when there are no more values or the variable is an
    // observation.
    // It is assumed that the parent values are clamped at this point.

    virtual void makeRandom() = 0;
    // Sets the parameters determining probGivenParents() to random values.

    virtual void makeUniform() = 0;
    // Sets the parameters determining probGivenParents() to uniform values.

    virtual void instantiate() = 0;
    // Sets the variable according to the probability distribution determined
    // by its parent's values. (i.e., sample from this distribution)
    // Used for simulation.

    virtual void tieWith(RandomVariable *rv) = 0;
    // In a DGM, the analogous occurrences of a variable in different time
    // slices all share the same distributions and accumulators. This function
    // tells a variable to use the data structures associated with just one
    // member of its equivalence class.

    virtual void cacheValue() = 0;
    // It can be useful to tell a variable to cache the value it's currently
    // set to. For example, wheen keeping track of the best values seen
    // so far in enumerativeViterbi(). Calling this function tells a variable
    // to store its current value (wherever it wants).
    // Not expected to be called many times, so just leave it virtual.
    // This like a push operation, onto a one element size stack.

    virtual void restoreCachedValue() = 0;
    // Sets the variable's value to its cached value.
    // This like a pop operation, off of the stack.

    /////////////////////////////////////////
    /////////////////////////////////////////
    // SUPPORT FOR EM  VARIABLES ////////////
    /////////////////////////////////////////
    /////////////////////////////////////////

    virtual void zeroAccumulators() = 0;
    // Called at the beginning of an EM iteration.

    virtual void increment(logpr posterior) = 0;
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

    virtual void update() = 0;
    // At the end of each EM iteration, this is called to convert the 
    // accumulated statistics into probability distributions.

    /////////////////////////////////////////
    /////////////////////////////////////////
    // END OF SUPPORT FOR EM  VARIABLES /////
    /////////////////////////////////////////
    /////////////////////////////////////////
};

#endif
