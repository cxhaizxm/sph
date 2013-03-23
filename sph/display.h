#ifndef _SPH_DISPLAY_H_
#define _SPH_DISPLAY_H_

// c++ libraries
#include <fstream>
#include <iostream>

// glew
#include "glew.h"

// glm
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"

#include "camera.h"
#include "world.h"
#include "fluid.h"

using namespace glm;

class Display
{
public:
	Display();
	char* textFileRead(const char* filename);
	void printLinkInfoLog(int prog);
	void printShaderInfoLog(int shader);
	void setFluids(Fluid *fluid); 

	void draw();
	void updateCamera();
	void updateViewport( int w, int h ) { camera->setViewport( w, h ); updateCamera(); }
	void zoomCamera( int dz ) { camera->zoom( dz ); updateCamera(); }
	void orbitCamera( int dx, int dy ) { camera->orbit( dx, dy ); ; updateCamera(); }

private:
	void initShaders();

	World * world;
	Camera * camera;
	Fluid* theFluid; 

	vec3 lightPos;
	vec3 lightCol;

	//frame buffer / depth texture
	unsigned int fbo;
	unsigned int depthTexture;

	//shader stuff
	unsigned int vertexShader;
	unsigned int fragmentShader;
	unsigned int shaderProgram;

	unsigned int vertexShadowShader;
	unsigned int fragmentShadowShader;
	unsigned int shadowShaderProgram;

	//attributes
	unsigned int shadowPositionLocation;

	unsigned int positionLocation;
	unsigned int colorLocation;
	unsigned int normalLocation;

	//uniforms
	unsigned int u_modelMatrixLocation;
	unsigned int u_projMatrixLocation;
	unsigned int u_lightPositionLocation;
	unsigned int u_lightColorLocation;
	unsigned int u_camPositionLocation;

	unsigned int u_shadowModelMatrixLocation;
	unsigned int u_shadowProjMatrixLocation;
	unsigned int u_shadowMapLocation;
	unsigned int u_shadowBiasMatrixLocation;
};

#endif