#ifndef OPENGL_H
#define OPENGL_H

#include <windows.h>
#include <gl\gl.h>

class Opengl {

public:
	Opengl();

	bool setupOpengl	(HWND hwnd);
	void pushValue		(int value);
	void drawGraph		();
	void setViewport	(int width, int height);
	void redrawScene	();
	
private:

	//===================
	HWND	windowHandle;
	HDC		hDC;
	HGLRC	hRC;

	int data[256];
	unsigned char startValue;
	unsigned char endValue;
	bool redraw;
	//===================
};

#endif