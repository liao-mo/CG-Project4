#pragma once
#include<iostream>
#include<vector>

#include "learnopengl/model.h"
#include "learnopengl/filesystem.h"
#include <learnopengl/shader_m.h>
#include "RenderUtilities/Texture.h"


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>
#include <GL/glut.h>

using namespace std;

#define MAX_WAVE 8
struct Wave
{
	GLfloat waveLength[MAX_WAVE];
	GLfloat amplitude[MAX_WAVE];
	GLfloat speed[MAX_WAVE];
	glm::vec2 direction[MAX_WAVE];
};

class WaterMesh
{
public:
	WaterMesh(glm::vec3 position);

	//shaders
	Shader* sinWave_shader = nullptr;
	Shader* heightMap_shader = nullptr;

	//sine wave
	void initWaves();
	void addSineWave(float waveLength, float amplitude, float speed, glm::vec2 direction);
	void drawSineWave();

	Wave waves;
	Model* grid;
	int waveCounter;

	float amplitude_coefficient;
	float waveLength_coefficient;
	float speed_coefficient;

	//height map
	vector<Texture2D*> heightMap_textures;
	void loadHeightMaps();

	// mode 0: sin wave, mode 1: height map
	void draw(bool mode);
	void setMVP(glm::mat4 m, glm::mat4 v, glm::mat4 p);
	void setEyePos(glm::vec3 eye_pos);
	void addTime(float delta_t);

	float time;
	glm::vec3 position;
	glm::vec3 eyePos;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;


	void drawHeightMap();
private:

};

