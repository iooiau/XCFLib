/******************************************************************************
*                                                                             *
*    xcfl_config.h                          Copyright(c) 2010-2013 itow,y.    *
*                                                                             *
******************************************************************************/

/*
  XCFLib - GIMP XCF format library
  Copyright(c) 2010-2013 itow,y.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301, USA.
*/


#ifndef XCFL_CONFIG_H
#define XCFL_CONFIG_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* C99 */
#if __STDC_VERSION__ >= 199901L
#define XCFL_HAVE_STDINT_H
#endif

#if defined(_WIN32) || defined(WIN32)
#define XCFL_WINDOWS
#endif

#ifdef _DEBUG
#define XCFL_DEBUG
#endif

#if defined(XCFL_DEBUG) && defined(_MSC_VER)
#define XCFL_MEMORY_DEBUG
#endif

#ifndef XCFL_WCHAR_SUPPORT
#if defined(_UNICODE) || defined(UNICODE)
#define XCFL_WCHAR_SUPPORT
#endif
#endif

#if defined(_MSC_VER)
#define XCFL_WFOPEN_SUPPORT
#endif

#ifdef XCFL_HAVE_STDINT_H
#include <stdint.h>
typedef int8_t xcflInt8;
typedef uint8_t xcflUInt8;
typedef int16_t xcflInt16;
typedef uint16_t xcflUInt16;
typedef int32_t xcflInt32;
typedef uint32_t xcflUInt32;
typedef int64_t xcflInt64;
typedef uint64_t xcflUInt64;
typedef int_fast_32_t xcflInt;
typedef uint_fast_32_t xcflUInt;
typedef int_fast_64_t xcflIntLeast64;
typedef uint_fast_64_t xcflUIntLeast64;
#else
typedef signed char xcflInt8;
typedef unsigned char xcflUInt8;
typedef short xcflInt16;
typedef unsigned short xcflUInt16;
typedef long xcflInt32;
typedef unsigned long xcflUInt32;
typedef long long xcflInt64;
typedef unsigned long long xcflUInt64;
typedef int xcflInt;
typedef unsigned int xcflUInt;
typedef xcflInt64 xcflIntLeast64;
typedef xcflUInt64 xcflUIntLeast64;
#endif

typedef float xcflFloat32;
typedef size_t xcflSize;

typedef int xcflBool;
#define XCFL_TRUE	1
#define XCFL_FALSE	0

#ifdef XCFL_WCHAR_SUPPORT
typedef wchar_t xcflWChar;
#define XCFL_WTEXT(text) L##text
#endif

#if defined(XCFL_WINDOWS) && defined(XCFL_DLL)
#ifdef XCFL_INTERNALS
#define XCFL_EXPORT(type) __declspec(dllexport) type __stdcall
#else
#define XCFL_EXPORT(type) __declspec(dllimport) type __stdcall
#endif
#else
#define XCFL_EXPORT(type) type
#endif

#ifdef XCFL_WINDOWS
#define XCFL_CALLBACK_DECL(type,name) type __stdcall name
#define XCFL_CALLBACK_TYPE(type,name) type (__stdcall *name)
#else
#define XCFL_CALLBACK_DECL(type,name) type name
#define XCFL_CALLBACK_TYPE(type,name) type (*name)
#endif

#define XCFL_MAX_IMAGE_WIDTH	32767
#define XCFL_MAX_IMAGE_HEIGHT	32767


#endif	/* ndef XCFL_CONFIG_H */
