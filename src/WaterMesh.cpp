#include "WaterMesh.h"
#include <string>

using namespace std;

WaterMesh::WaterMesh() :
	waveCounter(0),
	time(0),
	position(glm::vec3(0, 0, 0)),
	amplitude_coefficient(1.0)
{
	grid = new Model(FileSystem::getPath("resources/objects/grid/grid.obj"));
	//Debug
	//grid = new Model(FileSystem::getPath("resources/objects/grid/low_grid.obj"));
	sinWave_shader = new Shader("../src/shaders/water_surface.vert", "../src/shaders/water_surface.frag");
	initWaves();
}

WaterMesh::WaterMesh(glm::vec3 pos) :
	waveCounter(0),
	time(0),
	position(pos),
	amplitude_coefficient(1.0)
{
	grid = new Model(FileSystem::getPath("resources/objects/grid/grid.obj"));
	//Debug
	//grid = new Model(FileSystem::getPath("resources/objects/grid/low_grid.obj"));
	sinWave_shader = new Shader("../src/shaders/water_surface.vert", "../src/shaders/water_surface.frag");
	initWaves();
}

void WaterMesh::initWaves()
{
	for (int i = 0; i < MAX_WAVE; i++)
	{
		this->waves.waveLength[i] = 0;
		this->waves.amplitude[i] = 0;
		this->waves.speed[i] = 0;
		this->waves.direction[i] = glm::vec2(1, 0);
	}

	addSineWave(10, 0.3, 5, glm::vec2(1, 1));
	addSineWave(20, 0.4, 3, glm::vec2(1, -1));
	addSineWave(30, 0.5, 4, glm::vec2(2, 1));
	//addSineWave(20, 0.05, 50, glm::vec2(1, -0.5));
	//addSineWave(60, 0.2, 10, glm::vec2(-1.5, 0));
}


void WaterMesh::addSineWave(float waveLength, float amplitude, float speed, glm::vec2 direction) {
	this->waves.waveLength[waveCounter] = waveLength;
	this->waves.amplitude[waveCounter] = amplitude;
	this->waves.speed[waveCounter] = speed;
	this->waves.direction[waveCounter] = direction;
	waveCounter++;
	if (waveCounter == MAX_WAVE)
	{
		waveCounter = 0;
		cerr << "too many wave" << endl;
	}
}

void WaterMesh::setEyePos(glm::vec3 eye_pos) {
	eyePos = eye_pos;
}

void WaterMesh::setMVP(glm::mat4 m, glm::mat4 v, glm::mat4 p) {
	modelMatrix = m;
	viewMatrix = v;
	projectionMatrix = p;
}

void WaterMesh::addTime(float delta_t) {
	time += delta_t;
}

void WaterMesh::draw() {
	sinWave_shader->use();
	sinWave_shader->setMat4("model", modelMatrix);
	sinWave_shader->setMat4("view", viewMatrix);
	sinWave_shader->setMat4("projection", projectionMatrix);

	sinWave_shader->setFloat("time", time);
	sinWave_shader->setInt("numWaves", waveCounter);

	glUniform1fv(glGetUniformLocation(sinWave_shader->ID, "amplitude"), MAX_WAVE, waves.amplitude);
	glUniform1fv(glGetUniformLocation(sinWave_shader->ID, "wavelength"), MAX_WAVE, waves.waveLength);
	glUniform1fv(glGetUniformLocation(sinWave_shader->ID, "speed"), MAX_WAVE, waves.speed);
	for (int i = 0; i != MAX_WAVE; i++) {
		string name = "direction[";
		name += to_string(i);
		name += "]";
		GLint originsLoc = glGetUniformLocation(sinWave_shader->ID, name.c_str());
		glUniform2f(originsLoc, waves.direction[i].x, waves.direction[i].y);
	}

	sinWave_shader->setVec3("EyePos", eyePos);


	sinWave_shader->setVec3("light.direction", -1.0f, -1.0f, -0.0f);
	sinWave_shader->setVec3("viewPos", eyePos);
	// light properties
	sinWave_shader->setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
	sinWave_shader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
	sinWave_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	// material properties
	sinWave_shader->setFloat("material.shininess", 32.0f);

	grid->Draw(*sinWave_shader);
}

