// See LICENSE_CELLO file for license and copyright information

/// @file     parallel_GroupProcess.cpp
/// @author   James Bordner (jobordner@ucsd.edu)
/// @date     Wed Aug 11 15:28:49 PDT 2010
/// @brief    Implementation of the GroupProcess class


//----------------------------------------------------------------------

#include "cello.hpp"
#include "parallel.hpp"

//----------------------------------------------------------------------

GroupProcess * GroupProcess::create
(
 int process_first,
 int process_last_plus
 ) throw()
{
  GroupProcess * group = 0;

  group = new GroupProcessCharm;

  ASSERT("GroupProcess","process group creation failed",group != NULL);

  return group;
}

//======================================================================