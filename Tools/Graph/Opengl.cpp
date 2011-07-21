#include "Opengl.h"
#include <stdio.h>
#include "LIRCDefines.h"

Opengl::Opengl() {

	windowHandle = NULL;
	hDC	= NULL;
	hRC	= NULL;

	endValue	= 0;
	redraw		= false;

	for(int i=0; i<256; i++) {
		data[i] = 0;
	}
}

bool Opengl::setupOpengl(HWND hwnd) {

	//=================
	GLuint pixelFormat;
	//=================

	PIXELFORMATDESCRIPTOR pfd=						// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		24,											// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	
	if ((hDC=GetDC(hwnd))) {
		if ((pixelFormat=ChoosePixelFormat(hDC,&pfd))) {
			if(SetPixelFormat(hDC,pixelFormat,&pfd)) {
				if ((hRC=wglCreateContext(hDC))) {
					if(wglMakeCurrent(hDC,hRC)) {
						goto success;
					}
				}
			}
		}
	}

	printf("fail");
	return false;

success:

	//
	//setup a few states
	//

	glClearColor	(51/255.0f,153/255.0f,255/255.0f,1);
	glColor3ub		(0,0,0);
	glTranslatef	(-1,0,0);

	//
	//prepare screen
	//

	glClear			(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	SwapBuffers		(hDC);

	return true;
}

void Opengl::pushValue(int value) {

	if(!(value&0x01000000)) {
		if(value>10000) {
			endValue = 0;		//found gap so data will start now
			//printf("GAP VALUE %i\n",value);
			return;				//don't add the gap !
		}
	}

	data[endValue] = value;
	endValue++;
	redraw = true;
}

void Opengl::drawGraph() {

	//============
	int timeValue;
	int totalLength;
	//============

	if(!redraw) return;

	//
	// clear back buffer
	//

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	totalLength = 0;

	glBegin(GL_LINE_STRIP);

	for(int i=0; i<endValue; i++) {

		timeValue = data[i];

		//printf("index %i time value %i\n",i,timeValue&PULSE_MASK);

		if(timeValue&PULSE_BIT) {
			glVertex3f(totalLength * 0.000025f,0.7f,0);
		}
		else {
			glVertex3f(totalLength * 0.000025f,-0.7f,0);
		}

		totalLength += (timeValue & PULSE_MASK);

		if(timeValue&PULSE_BIT) {
			glVertex3f(totalLength * 0.000025f,0.7f,0);
		}
		else {
			glVertex3f(totalLength * 0.000025f,-0.7f,0);
		}
	}

	if(endValue) {
		glVertex3f(totalLength * 0.000025f,-0.7f,0);
		glVertex3f(2,-0.7f,0);
	}

	glEnd();

	SwapBuffers(hDC);

	redraw = false;
}

void Opengl::setViewport(int width, int height) {

	if(width<=0)	width	= 1;
	if(height<=0)	height	= 1;

	glViewport(0,0,width,height);
}

void Opengl::redrawScene() {

	redraw = true;
	drawGraph();
}