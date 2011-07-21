#include "StringFunctions.h"

void removeTrailingWhiteSpace(TCHAR *string) {

	//===============
	int stringLength;
	//===============

	//
	// get string length
	//

	stringLength = _tcslen(string);

	if(stringLength==0) {
		return;
	}

	while(stringLength>0) {

		if(string[stringLength-1] == ' ') {
			string[stringLength-1] = '\0';
		}
		else {
			break;
		}

		stringLength--;
	}
}