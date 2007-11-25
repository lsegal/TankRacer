//////////////////////////////////////////////////////////////////////////////////////////
//	EXT_multi_draw_arrays_extension.h
//	Extension setup header
//	Downloaded from: www.paulsprojects.net
//	Created:	8th August 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#ifndef EXT_MULTI_DRAW_ARRAYS_EXTENSION_H
#define EXT_MULTI_DRAW_ARRAYS_EXTENSION_H

BOOL SetUpEXT_multi_draw_arrays();
extern BOOL EXT_multi_draw_arrays_supported;

extern PFNGLMULTIDRAWARRAYSEXTPROC				glMultiDrawArraysEXT;
extern PFNGLMULTIDRAWELEMENTSEXTPROC			glMultiDrawElementsEXT;

#endif	//EXT_MULTI_DRAW_ARRAYS_EXTENSION_H
