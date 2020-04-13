#ifndef __MAIN_H__
#define __MAIN_H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#ifndef NOCRYPT
#define NOCRYPT
#endif // NOCRYPT

#ifndef NOUSER
#define NOUSER
#endif // NOUSER

#ifndef NOGDI
#define NOGDI
#endif // NOGDI

#ifndef NOSERVICE
#define NOSERVICE
#endif // NOSERVICE

#ifndef NMCX
#define NMCX
#endif // NMCX

#ifndef NOIME
#define NOIME
#endif // NOIME

#include <windows.h>

/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

/////////////////
// Example Export

#ifdef __cplusplus
extern "C"
{
#endif

// void DLL_EXPORT SomeFunction(const LPCSTR sometext);

#ifdef __cplusplus
}
#endif

//
/////////////////

#endif // __MAIN_H__
