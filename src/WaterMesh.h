#pragma once
#include<iostream>
#include<vector>

#include "learnopengl/model.h"
#include "learnopengl/filesystem.h"
#include <learnopengl/shader_m.h>


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>
#include <GL/glut.h>

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
	WaterMesh();
	WaterMesh(glm::vec3 position);

	Shader* water_shader = nullptr;

	Wave waves;
	Model* grid;
	void initWaves();
	void addSineWave(float waveLength, float amplitude, float speed, glm::vec2 direction);
	void draw();

	int waveCounter;
	float time;
	glm::vec3 position;
	glm::vec3 eyePos;

	void setMVP(glm::mat4 m, glm::mat4 v, glm::mat4 p);
	void setEyePos(glm::vec3 eye_pos);

	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;


private:

};

