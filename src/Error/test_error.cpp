/** 
 *********************************************************************
 *
 * @file      
 * @brief     
 * @author    
 * @date      
 * @ingroup
 * @bug       
 * @note      
 *
 *--------------------------------------------------------------------
 *
 * DESCRIPTION:
 *
 *    
 *
 * CLASSES:
 *
 *    
 *
 * FUCTIONS:
 *
 *    
 *
 * USAGE:
 *
 *    
 *
 * REVISION HISTORY:
 *
 *    
 *
 * COPYRIGHT: See the LICENSE_CELLO file in the project directory
 *
 *--------------------------------------------------------------------
 *
 * $Id$
 *
 *********************************************************************
 */

/** 
 *********************************************************************
 *
 * @file      test_error.cpp
 * @brief     Program implementing unit tests for error classes
 * @author    James Bordner
 * @date      Wed Aug 20 11:24:14 PDT 2008
 *
 * $Id$
 *
 *********************************************************************
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "error.hpp"
#include "test.hpp"

int main(int argc, char ** argv)
{

  unit_class ("Error");
  unit_open();

  //----------------------------------------------------------------------
  printf ("Warning message:\n");

  char warning_message[ERROR_MESSAGE_LENGTH];
  sprintf (warning_message,"Warning message test");
  WARNING_MESSAGE("main",warning_message);

  unit_assert (true);

  //----------------------------------------------------------------------
  printf ("Incomplete message:\n");

  char incomplete_message[ERROR_MESSAGE_LENGTH];
  sprintf (incomplete_message,"Incomplete message test");
  INCOMPLETE_MESSAGE("main",incomplete_message);

  unit_assert (true);

  //----------------------------------------------------------------------
//   printf ("Error message:\n");

//   char error_message[ERROR_MESSAGE_LENGTH];
//   sprintf (error_message,"Error message test");
//   ERROR_MESSAGE("main",error_message);
  
//   // Errors should abort, so all following lines should not be executed
//   exit(1);
  //----------------------------------------------------------------------

  unit_close();
}