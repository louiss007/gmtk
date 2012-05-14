/*
 * gmtkStats.cc
 * produce a FIR filter file to do mean and variance normalization
 *
 * Written by Richard Rogers <rprogers@ee.washington.edu>
 *
 * Copyright (c) 2012, < fill in later >
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any non-commercial purpose
 * and without fee is hereby granted, provided that the above copyright
 * notice appears in all copies.  The University of Washington,
 * Seattle make no representations about the suitability of this software
 * for any purpose. It is provided "as is" without express or implied warranty.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
static const char * gmtk_version_id = PACKAGE_STRING;
#  ifdef HAVE_HG_H
#    include "hgstamp.h"
#  endif

#else 
// TODO: automate the process of updating this string.
static const char * gmtk_version_id = "GMTK Version 0.2b Tue Jan 20 22:59:41 2004";
#endif

#include "general.h"
#include "error.h"
#include "debug.h"
#include "arguments.h"

#include "GMTK_ObservationSource.h"
#include "GMTK_FileSource.h"
#include "GMTK_CreateFileSource.h"
#include "GMTK_FilterFile.h"
#include "GMTK_MergeFile.h"
#include "GMTK_ASCIIFile.h"
#include "GMTK_FlatASCIIFile.h"
#include "GMTK_PFileFile.h"
#include "GMTK_HTKFile.h"
#include "GMTK_HDF5File.h"
#include "GMTK_BinaryFile.h"
#include "GMTK_Stream.h"

#define GMTK_ARG_OBS_FILES
#define GMTK_ARG_CPP_CMD_OPTS
#define GMTK_ARG_OBS_MATRIX_XFORMATION
#define GMTK_ARG_DCDRNG
#define GMTK_ARG_FILE_RANGE_OPTIONS
#define GMTK_ARG_START_END_SKIP
#define GMTK_ARG_GENERAL_OPTIONS
#define GMTK_ARG_HELP
#define GMTK_ARG_VERSION

#define GMTK_ARGUMENTS_DEFINITION
#include "ObsArguments.h"
#undef GMTK_ARGUMENTS_DEFINITION

bool meanSubOnly=false;

Arg Arg::Args[] = {
#define GMTK_ARGUMENTS_DOCUMENTATION
#include "ObsArguments.h"
#undef GMTK_ARGUMENTS_DOCUMENTATION

  Arg("\n*** Statistical options ***\n"),
  Arg("meanSubOnly", Arg::Opt,meanSubOnly,"Only normalize for 0 mean rather 0 mean and unit variance"),
  // final one to signal the end of the list
  Arg()
};


#define TOO_SMALL (1e-10)


int 
main(int argc, char *argv[]) {

  bool parse_was_ok = Arg::parse(argc,(char**)argv);

  if(help) {
    Arg::usage();
    exit(0);
  }
  if(!parse_was_ok) {
    Arg::usage(); exit(-1);
  }
  if (print_version_and_exit) {
#ifdef HAVE_CONFIG_H
    printf("%s (Mercurial id: %s)\n",gmtk_version_id,HGID);
#else
    printf("%s\n", gmtk_version_id);
#endif
    exit(0);
  }
  infoMsg(IM::Max,"Finished parsing arguments\n");
#define GMTK_ARGUMENTS_CHECK_ARGS
#include "ObsArguments.h"
#undef GMTK_ARGUMENTS_CHECK_ARGS

  infoMsg(IM::Max,"Opening Files ...\n");

#if 0
  FileSource globalObservationMatrix;
  ObservationFile *obsFile[MAX_NUM_OBS_FILES];
  unsigned nFiles=0;
  unsigned nCont = 0;
  for (unsigned i=0; i < MAX_NUM_OBS_FILES && ofs[i] != NULL; i+=1, nFiles+=1) {
    
    obsFile[i] = instantiateFile(ifmts[i], ofs[i], nfs[i], nis[i], i, iswp[i],
                                 Cpp_If_Ascii, cppCommandOptions, prefrs[i], preirs[i],
                                 prepr[i], sr[i]);
    assert(obsFile[i]);
    Filter *fileFilter = instantiateFilters(Per_Stream_Transforms[i],
                                            obsFile[i]->numContinuous());
    if (fileFilter) {
      obsFile[i] = new FilterFile(fileFilter, obsFile[i], frs[i], irs[i], postpr[i]);
      nCont += obsFile[i]->numContinuous();
    } else
      error("current implementation requires filter\n");
  }
  MergeFile *mf  = new  MergeFile(nFiles, obsFile,
				  Action_If_Diff_Num_Sents,
				  Action_If_Diff_Num_Frames,
				  Ftr_Combo);
  FilterFile *ff = new FilterFile(instantiateFilters(Post_Transforms, nCont), 
				  mf, NULL, NULL, gpr_str);
  globalObservationMatrix.initialize(ff, 1024*1024, /* FIXME - argument */
				     startSkip, endSkip, justification);

  FileSource *f = &globalObservationMatrix;
#else
  FileSource *f = instantiateFileSource();
#endif

  unsigned nCont = f->numContinuous();

  double *mean     = new double[nCont]; assert(mean);
  double *xSqrd    = new double[nCont]; assert(xSqrd);
  double *std      = new double[nCont]; assert(std);
  double *diff     = new double[nCont]; assert(diff);
  double *diffSqrd = new double[nCont]; assert(diffSqrd);

  for (unsigned i=0; i < nCont; i+=1) {
    mean[i] = 0.0; xSqrd[i] = 0.0; std[i] = 0.0; diff[i] = 0.0; diffSqrd[i] = 0.0;
  }
  
  float N = 0.0f;
  for (unsigned j=0; j < f->numSegments(); j+=1) {
    assert(f->openSegment(j));
    for (unsigned k=0; k < f->numFrames(); k+=1) {
      Data32 const *buf = f->loadFrames(k,1);
      for (unsigned ff=0; ff < nCont; ff+=1) {
	float x = ((float *)buf)[ff];
	mean[ff] += x;
	xSqrd[ff] += x * x;
      }
      N += 1.0f;
    }
  }

  if (meanSubOnly) {
    for (unsigned i=0; i < nCont; i+=1) {
      mean[i] /= N;
    }
  } else {
    for (unsigned i=0; i < nCont; i+=1) {
      mean[i] /= N;
      std[i] = sqrt( N / (N-1.0f) * fabs( xSqrd[i]/N - mean[i] * mean[i] ) );
    }
  }
  printf("%u %u\n",  nCont, nCont);
  for (unsigned i=0; i < nCont; i+=1) {
    unsigned j;
    for (j=0; j < i; j+=1)
      printf("0.0 ");
    if (meanSubOnly || std[i] <= TOO_SMALL) {
      if (!meanSubOnly) 
	warning("WARNING: Value of variance in mean/variance normalization is too small (<=%f).  Will only perform mean substraction.\n", TOO_SMALL);
      printf("1.0");
    } else {
      printf("%f", 1.0/std[i]);
    }
    for (j=i+1; j < nCont; j+=1)
      printf(" 0.0");
    printf("\n");
  }
  for (unsigned i=0; i < nCont; i+=1) {
    if (meanSubOnly || std[i] <= TOO_SMALL) {
      printf(" %f", -mean[i]);
    } else {
      printf(" %f", -mean[i] / std[i]);
    }
  }
  printf("\n");
  exit(0);
}