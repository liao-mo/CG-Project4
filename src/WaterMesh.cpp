#include "WaterMesh.h"
#include <string>

using namespace std;

WaterMesh::WaterMesh() :
	waveCounter(0),
	time(0),
	position(glm::vec3(0, 10, 0))
{
	grid = new Model(FileSystem::getPath("resources/objects/grid/grid.obj"));
	water_shader = new Shader("../src/shaders/water_surface.vert", "../src/shaders/water_surface.frag");
	initWaves();
}

WaterMesh::WaterMesh(glm::vec3 pos) :
	waveCounter(0),
	time(0),
	position(pos)
{
	grid = new Model(FileSystem::getPath("resources/objects/grid/grid.obj"));
	water_shader = new Shader("../src/shaders/water_surface.vert", "../src/shaders/water_surface.frag");
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

	addSineWave(10, 3, 15, glm::vec2(1, 1));
	addSineWave(10, 3, 15, glm::vec2(1, 0));
	//addSineWave(30, 0.04, 20, glm::vec2(0, 1));
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

void WaterMesh::draw() {
	time += 0.01;
	//cout << "time: " << time << endl;
	water_shader->use();
	water_shader->setMat4("model", modelMatrix);
	water_shader->setMat4("view", viewMatrix);
	water_shader->setMat4("projection", projectionMatrix);

	water_shader->setFloat("time", time);
	water_shader->setInt("numWaves", waveCounter);

	glUniform1fv(glGetUniformLocation(water_shader->ID, "amplitude"), MAX_WAVE, waves.amplitude);
	glUniform1fv(glGetUniformLocation(water_shader->ID, "wavelength"), MAX_WAVE, waves.waveLength);
	glUniform1fv(glGetUniformLocation(water_shader->ID, "speed"), MAX_WAVE, waves.speed);
	for (int i = 0; i != MAX_WAVE; i++) {
		string name = "direction[";
		name += to_string(i);
		name += "]";
		GLint originsLoc = glGetUniformLocation(water_shader->ID, name.c_str());
		glUniform2f(originsLoc, waves.direction[i].x, waves.direction[i].y);
	}

	water_shader->setVec3("EyePos", eyePos);


	water_shader->setVec3("light.direction", -1.0f, -1.0f, -0.0f);
	water_shader->setVec3("viewPos", eyePos);
	// light properties
	water_shader->setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
	water_shader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
	water_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	// material properties
	water_shader->setFloat("material.shininess", 32.0f);

	grid->Draw(*water_shader);
	

}

