/*
 * GMTK_RVInfo.h
 * A class that keeps all the information about a random
 * variable that is unique to its location in the structure
 * file (i.e., this is a random variable "class", and the information 
 * kept here is common to *all* instantiations of a random variable).
 *
 *
 * Written by Jeff Bilmes <bilmes@ee.washington.edu>
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
 *
 * $Header$
 *
 */

#ifndef GMTK_RVINFO_H
#define GMTK_RVINFO_H

#include <vector>
#include <string>
#include <map>

#include <stdio.h>
#include <stdlib.h>

class RV;
class DiscRV;
class ContRV;
class FileParser;
class GMTemplate;
class Partition;
class BoundaryTriangulate;

class RV;
class DiscRV;

#include "GMTK_CPT.h"
#include "GMTK_MixtureCommon.h"

class RVInfo {
  friend class FileParser;
  friend class GMTemplate;
  friend class Partition;
  friend class BoundaryTriangulate;
  friend class JunctionTree;

  friend class RV;
  friend class DiscRV;
  friend class ObsDiscRV;
  friend class Sw_ObsDiscRV;
  friend class ScPnSh_ObsDiscRV;
  friend class ScPnSh_Sw_ObsDiscRV;
  friend class HidDiscRV;
  friend class Sw_HidDiscRV;
  friend class ScPnSh_HidDiscRV;
  friend class ScPnSh_Sw_HidDiscRV;
  friend class ObsContRV;
  friend class Sw_ObsContRV;
  friend class ScPnSh_ObsContRV;
  friend class ScPnSh_Sw_ObsContRV;
  friend class ScPnShRV;
  friend class ContRV;

public:
  ///////////////////////////////////////////////////
  // TODO: change the name of this from rvParent to something like rvID (since
  // this isn't nec. a parent, but a general ID of a random variable).
  typedef pair<string,int> rvParent;
private:

  ////////////////////////////////////////////////////////////
  // define a bunch of types that are used in RVs
  enum Type { t_discrete, t_continuous, t_unknown };

  enum Disposition { d_hidden, d_observed, d_unknown };

  struct WeightInfo {

    // Given a probability p, the weight info modifies the
    // probability according to the following formula:
    //
    //        penalty*p^scale+shift
    // 
    // each of penalty, scale, and shift might be an immediate value
    // or might come from the observation file at a frame given by the
    // child class. 
    
    struct WeightItem { 
      enum wtEnum { wt_NoWeight, wt_Constant, wt_Observation };
      wtEnum wt_Status;
      // the values of scale,penalty, or shift depending on the
      // particular case. Note that the value stored is the
      // natural_log(value), where value is scale, penalty, or shift
      // above.
      double weight_value;
      unsigned firstFeatureElement;
      unsigned lastFeatureElement;
      void clear() { wt_Status = wt_NoWeight; }
    };
    
    WeightItem penalty;
    WeightItem scale;
    WeightItem shift;
    WeightInfo() { clear(); }
    void clear() { penalty.clear(); scale.clear(); shift.clear(); }

  };

  struct FeatureRange {
    enum frEnum { fr_UnFilled, fr_Range, fr_FirstIsValue, 
		  fr_FrameNumIsValue, fr_NumFramesIsValue,
		  fr_SegmentNumIsValue, fr_NumSegmentsIsValue 
    };
    frEnum filled;
    unsigned firstFeatureElement;
    unsigned lastFeatureElement;
    FeatureRange() { filled = fr_UnFilled; }
    void clear() { filled = fr_UnFilled; }
  };

  struct ListIndex {
    enum ListIndexType { li_String, li_Index, li_Unknown } liType;
    // if this is an integer index, this is used.
    unsigned intIndex;
    // Otherwise, if this is an string index, this is used.
    string nameIndex;
    // in cases where this is a reference to a DT that
    // is supposed to reference via a collection, here
    // is the name of the collection.
    string collectionName;
    void clear() { liType = li_Unknown; }
  };


  // A hint given to the triangulation algorithm.
  // this is something that can be specified in the structure
  // file and can be used as a selector during triangulation.
  float eliminationOrderHint;

  // The position of this variable in the structure file 
  // (i.e., this variable indicates the order in which
  // this variable is encountered when parsing). It is
  // used as a possible triangulation hint. Value -1
  // means uninitialized.
  int variablePositionInStrFile;

  ///////////////////////////////////////////////////////////
  // data associated with a RV

  // TODO: remove some of the friends above and make public interfaces to 
  //        these as read only.
  // the frame where it was defined (so is part of name really)
  unsigned frame;
  // line number of file where this RV was first declared
  unsigned fileLineNumber;
  // file name where this r.v. is first defined.
  string rvFileName;
  // rv's name
  string name;    

  // fields from type
  Type rvType;
  Disposition rvDisp;
  unsigned rvCard;
  // if it is an observed variable, it must have a feature range.
  FeatureRange rvFeatureRange;

  // switching parents and weight stuff
  vector< rvParent > switchingParents;

  ListIndex switchMapping;

  // conditional parents stuff
  vector<vector< rvParent > > conditionalParents;
  // if discrete, then the list if discrete implementations
  vector< CPT::DiscreteImplementaton > discImplementations;
  // if continuous, the list of continuous implementations
  vector< MixtureCommon::ContinuousImplementation > contImplementations;
  // in either case, a low-level parameter index
  vector< ListIndex > listIndices;

  // (possibly switching) weight information.  Note that either
  // (rvWeightInfo.size() == 1), or (rvWeightInfo.size() ==
  // conditionalParents.size()).
  vector <WeightInfo> rvWeightInfo;

  /////////////////////////////////////////////////////////
  // An actual pointer to the RV once we instantiate it.  Note that
  // in general, there is a many-to-one mapping from random variable
  // instances to RVInfo instances.  This pointer here is used
  // only by the parser to create the very first set of random
  // variables that correspond exactly to the structure file. If you
  // will, you can consider rv here to be the Adam&Eve random
  // variable since all instances of this are going to be clones of
  // 'rv' (or clones of clones of 'rv', etc.).
  RV* rv;

public:

  /////////////////////////////////////////////////////////
  // constructor
  RVInfo() { clear(); }


  // copy constructor
  RVInfo(const RVInfo&v) {
    frame = v.frame;
    fileLineNumber = v.fileLineNumber;
    rvFileName = v.rvFileName;
    name = v.name;
    rvType = v.rvType;
    rvDisp = v.rvDisp;
    rvCard = v.rvCard;
    eliminationOrderHint = v.eliminationOrderHint;
    variablePositionInStrFile = v.variablePositionInStrFile;
    rvFeatureRange = v.rvFeatureRange;
    rvWeightInfo = v.rvWeightInfo;
    switchingParents = v.switchingParents;
    switchMapping = v.switchMapping;
    conditionalParents = v.conditionalParents;
    discImplementations = v.discImplementations;
    contImplementations = v.contImplementations;
    listIndices = v.listIndices;
    rv = v.rv;
  }

  // clear out the current RV structure when we 
  // are parsing and encounter a new RV.
  void clear() { 
    name.erase();

    rvType = t_unknown;
    rvDisp = d_unknown;
    rvFeatureRange.clear();
    rvWeightInfo.clear();
    eliminationOrderHint = 0.0;
    variablePositionInStrFile = -1;

    switchingParents.clear();
    switchMapping.clear();

    conditionalParents.clear();
    discImplementations.clear();
    contImplementations.clear();
    listIndices.clear();

  }

};

#endif
