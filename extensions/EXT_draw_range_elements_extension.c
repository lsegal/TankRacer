//////////////////////////////////////////////////////////////////////////////////////////
//	EXT_draw_range_elements_extension.cpp
//	EXT_draw_range_elements extension setup
//	Downloaded from: www.paulsprojects.net
//	Created:	20th July 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	
#include <windows.h>
#include <GL\gl.h>
#include "../glext.h"
#include "EXT_draw_range_elements_extension.h"

BOOL EXT_draw_range_elements_supported = FALSE;

BOOL SetUpEXT_draw_range_elements()
{
	//Check for support
	char * extensionString=(char *)glGetString(GL_EXTENSIONS);
	char * extensionName="GL_EXT_draw_range_elements";

	char * endOfString;									//store pointer to end of string
	unsigned int distanceToSpace;						//distance to next space

	endOfString=extensionString+strlen(extensionString);

	//loop through string
	while(extensionString<endOfString)
	{
		//find distance to next space
		distanceToSpace=(unsigned int)strcspn(extensionString, " ");

		//see if we have found extensionName
		if((strlen(extensionName)==distanceToSpace) &&
			(strncmp(extensionName, extensionString, distanceToSpace)==0))
		{
			EXT_draw_range_elements_supported = TRUE;
		}

		//if not, move on
		extensionString+=distanceToSpace+1;
	}

	if(!EXT_draw_range_elements_supported)
	{
		return FALSE;
	}

	//get function pointers
	glDrawRangeElementsEXT			=	(PFNGLDRAWRANGEELEMENTSEXTPROC)
										wglGetProcAddress("glDrawRangeElementsEXT");

	return TRUE;
}

//function pointers
PFNGLDRAWRANGEELEMENTSEXTPROC			glDrawRangeElementsEXT					=NULL;

