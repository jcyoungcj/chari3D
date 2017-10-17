/*
 * MATLAB Compiler: 5.1 (R2014a)
 * Date: Fri May 26 10:29:55 2017
 * Arguments: "-B" "macro_default" "-B" "csharedlib:fit" "-W" "lib:fit" "-T"
 * "link:lib" "fit.m" 
 */

#ifndef __fit_h
#define __fit_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_fit
#define PUBLIC_fit_C_API __global
#else
#define PUBLIC_fit_C_API /* No import statement needed. */
#endif

#define LIB_fit_C_API PUBLIC_fit_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_fit
#define PUBLIC_fit_C_API __declspec(dllexport)
#else
#define PUBLIC_fit_C_API __declspec(dllimport)
#endif

#define LIB_fit_C_API PUBLIC_fit_C_API


#else

#define LIB_fit_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_fit_C_API 
#define LIB_fit_C_API /* No special import/export declaration */
#endif

extern LIB_fit_C_API 
bool MW_CALL_CONV fitInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_fit_C_API 
bool MW_CALL_CONV fitInitialize(void);

extern LIB_fit_C_API 
void MW_CALL_CONV fitTerminate(void);



extern LIB_fit_C_API 
void MW_CALL_CONV fitPrintStackTrace(void);

extern LIB_fit_C_API 
bool MW_CALL_CONV mlxFit(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);



extern LIB_fit_C_API bool MW_CALL_CONV mlfFit(mxArray* result);

#ifdef __cplusplus
}
#endif
#endif
