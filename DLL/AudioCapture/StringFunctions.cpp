#include "StringFunctions.h"
#include <cstring>

void removeTrailingWhiteSpace(wchar_t* string) {

	size_t stringLength = wcslen(string);
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
