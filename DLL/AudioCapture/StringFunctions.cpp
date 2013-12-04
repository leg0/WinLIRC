#include "StringFunctions.h"

void removeTrailingWhiteSpace(TCHAR *string) {

	//==================
	size_t stringLength;
	//==================

	//
	// get string length
	//

	stringLength = _tcslen(string);

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