/************************************************************************
     File:        TrainView.cpp

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>
#include <GL/glut.h>

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"


// Constructor to set up the GL window
TrainView::
TrainView(int x, int y, int w, int h, const char* l) : 
	Fl_Gl_Window(x,y,w,h,l)
//========================================================================
{
	mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );

	resetArcball();
	camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	camera.MovementSpeed = 50.0f;
	old_t = glutGet(GLUT_ELAPSED_TIME);
	k_pressed = false;
}

// * Reset the camera to look at the world
void TrainView::
resetArcball()
{
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

// * FlTk Event handler for the window
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value()) {
		//if (arcball.handle(event))
		//	return 1;
	}


	// remember what button was used
	static int last_push;
	

	switch (event) {
		// Mouse button being pushed event
	case FL_PUSH:
		last_push = Fl::event_button();
		// if the left button be pushed is left mouse button
		if (last_push == FL_LEFT_MOUSE) {
			doPick();
			damage(1);
			return 1;
		}
		else if (last_push == FL_RIGHT_MOUSE) {
			int xpos = Fl::event_x();
			int ypos = Fl::event_y();
			lastX = xpos;
			lastY = ypos;
			damage(1);
			return 1;
		}
		break;

		// Mouse button release event
	case FL_RELEASE: // button release
		damage(1);
		last_push = 0;
		return 1;

		// Mouse button drag event
	case FL_DRAG:

		// Compute the new control point position
		if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
			ControlPoint* cp = &m_pTrack->points[selectedCube];

			double r1x, r1y, r1z, r2x, r2y, r2z;
			getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

			double rx, ry, rz;
			mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z,
				static_cast<double>(cp->pos.x),
				static_cast<double>(cp->pos.y),
				static_cast<double>(cp->pos.z),
				rx, ry, rz,
				(Fl::event_state() & FL_CTRL) != 0);

			cp->pos.x = (float)rx;
			cp->pos.y = (float)ry;
			cp->pos.z = (float)rz;
			damage(1);
		}
		else if (last_push == FL_RIGHT_MOUSE) {
			// where is the mouse?
			int xpos = Fl::event_x();
			int ypos = Fl::event_y();
			if (firstMouse)
			{
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

			lastX = xpos;
			lastY = ypos;

			camera.ProcessMouseMovement(xoffset, yoffset);
			damage(1);
		}
		break;

		// in order to get keyboard events, we need to accept focus
	case FL_FOCUS:
		return 1;

		// every time the mouse enters this window, aggressively take focus
	case FL_ENTER:
		focus(this);
		break;

	case FL_KEYBOARD:
		if (k_pressed == false) {
			k_pressed = true;
			damage(1);
		}

		k = Fl::event_key();
		ks = Fl::event_state();
		if (k == 'p') {
			// Print out the selected control point information
			if (selectedCube >= 0)
				printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
					selectedCube,
					m_pTrack->points[selectedCube].pos.x,
					m_pTrack->points[selectedCube].pos.y,
					m_pTrack->points[selectedCube].pos.z,
					m_pTrack->points[selectedCube].orient.x,
					m_pTrack->points[selectedCube].orient.y,
					m_pTrack->points[selectedCube].orient.z);
			else
				printf("Nothing Selected\n");

			return 1;
		};
		break;

	case 9:
		k_pressed = false;
		cout << "key up" << endl;
		break;
	}


	return Fl_Gl_Window::handle(event);
}

// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
void TrainView::draw()
{
	//calculate delta time
	updateTimer();

	// * Set up basic opengl informaiton
	//initialized glad
	if (gladLoadGL())
	{
		//initiailize VAO, VBO, Shader...
		if (!basic_light_shader) {
			basic_light_shader = new Shader("../src/shaders/basic_lighting.vs", "../src/shaders/basic_lighting.fs");
		}

		if (!directional_light_shader) {
			directional_light_shader = new Shader("../src/shaders/directional_light.vs", "../src/shaders/directional_light.fs");
		}

		if (!point_light_shader) {
			point_light_shader = new Shader("../src/shaders/point_light.vs", "../src/shaders/point_light.fs");
		}

		if (!spot_light_shader) {
			spot_light_shader = new Shader("../src/shaders/spot_light.vs", "../src/shaders/spot_light.fs");
		}

		if (!light_source_shader) {
			light_source_shader = new Shader("../src/shaders/light_cube.vs", "../src/shaders/light_cube.fs");
		}

		if (!this->test_model) {
			test_model = new Model(FileSystem::getPath("resources/objects/Sci_fi_Train/Sci_fi_Train.obj"));
		}

		if (!this->commom_matrices)
			this->commom_matrices = new UBO();
			this->commom_matrices->size = 2 * sizeof(glm::mat4);
			glGenBuffers(1, &this->commom_matrices->ubo);
			glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
			glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

		if (!this->plane) {
			GLfloat  vertices[] = {
				-0.5f ,0.0f , -0.5f,
				-0.5f ,0.0f , 0.5f ,
				0.5f ,0.0f ,0.5f ,
				0.5f ,0.0f ,-0.5f };
			GLfloat  normal[] = {
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f };
			GLfloat  texture_coordinate[] = {
				0.0f, 0.0f,
				1.0f, 0.0f,
				1.0f, 1.0f,
				0.0f, 1.0f };
			GLuint element[] = {
				0, 1, 2,
				0, 2, 3, };

			this->plane = new VAO;
			this->plane->element_amount = sizeof(element) / sizeof(GLuint);
			glGenVertexArrays(1, &this->plane->vao);
			glGenBuffers(3, this->plane->vbo);
			glGenBuffers(1, &this->plane->ebo);

			glBindVertexArray(this->plane->vao);

			// Position attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);

			// Normal attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(1);

			// Texture Coordinate attribute
			glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(2);

			//Element attribute
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->plane->ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

			// Unbind VAO
			glBindVertexArray(0);
		}

		if (!this->texture)
			this->texture = new Texture2D("../Images/church.png");

		if (!this->device){
			//Tutorial: https://ffainelli.github.io/openal-example/
			this->device = alcOpenDevice(NULL);
			if (!this->device) {
				//puts("ERROR::NO_AUDIO_DEVICE");
			}
				

			ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
			if (enumeration == AL_FALSE) {
				//puts("Enumeration not supported");
			}
			else {
				//puts("Enumeration supported");
			}

			this->context = alcCreateContext(this->device, NULL);
			if (!alcMakeContextCurrent(context))
				puts("Failed to make context current");

			this->source_pos = glm::vec3(0.0f, 5.0f, 0.0f);

			ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
			alListener3f(AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alListener3f(AL_VELOCITY, 0, 0, 0);
			alListenerfv(AL_ORIENTATION, listenerOri);

			alGenSources((ALuint)1, &this->source);
			alSourcef(this->source, AL_PITCH, 1);
			alSourcef(this->source, AL_GAIN, 1.0f);
			alSource3f(this->source, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
			alSourcei(this->source, AL_LOOPING, AL_TRUE);

			alGenBuffers((ALuint)1, &this->buffer);

			ALsizei size, freq;
			ALenum format;
			ALvoid* data;
			ALboolean loop = AL_TRUE;

			//Material from: ThinMatrix
			alutLoadWAVFile((ALbyte*)"../resources/audio/YOASOBI0.wav", &format, &data, &size, &freq, &loop);
			alBufferData(this->buffer, format, data, size, freq);
			alSourcei(this->source, AL_BUFFER, this->buffer);

			if (format == AL_FORMAT_STEREO16 || format == AL_FORMAT_STEREO8)
				puts("TYPE::STEREO");
			else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_MONO8)
				puts("TYPE::MONO");

			alSourcef(source, AL_GAIN, 0.1);
			alSourcePlay(this->source);

			// cleanup context
			//alDeleteSources(1, &source);
			//alDeleteBuffers(1, &buffer);
			//device = alcGetContextsDevice(context);
			//alcMakeContextCurrent(NULL);
			//alcDestroyContext(context);
			//alcCloseDevice(device);
		}
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");

	// Set up the view port
	glViewport(0,0,w(),h());
	

	// clear the window, be sure to clear the Z-Buffer too
	glClearColor(0,0,.3f,0);		// background should be blue

	// we need to clear out the stencil buffer since we'll use
	// it for shadows
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	// Blayne prefers GL_DIFFUSE
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// prepare for projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setProjection();		// put the code to set up matrices here
	

	// TODO: 
	// you might want to set the lighting up differently. if you do, 
	// we need to set up the lights AFTER setting up the projection

	// enable the lighting
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// top view only needs one light
	if (tw->topCam->value()) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	} else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	//GLfloat lightPosition1[]	= {0,1,1,0}; // {50, 200.0, 50, 1.0};
	//GLfloat lightPosition2[]	= {1, 0, 0, 0};
	//GLfloat lightPosition3[]	= {0, -1, 0, 0};
	//GLfloat yellowLight[]		= {0.5f, 0.5f, .1f, 1.0};
	//GLfloat whiteLight[]			= {1.0f, 1.0f, 1.0f, 1.0};
	//GLfloat blueLight[]			= {.1f,.1f,.3f,1.0};
	//GLfloat grayLight[]			= {.3f, .3f, .3f, 1.0};

	//glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	//glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	//glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	//glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	// set linstener position 
	if(selectedCube >= 0)
		alListener3f(AL_POSITION, 
			m_pTrack->points[selectedCube].pos.x,
			m_pTrack->points[selectedCube].pos.y,
			m_pTrack->points[selectedCube].pos.z);
	else
		alListener3f(AL_POSITION, 
			this->source_pos.x, 
			this->source_pos.y,
			this->source_pos.z);


	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	glUseProgram(0);

	setupFloor();
	glDisable(GL_LIGHTING);
	drawFloor(200,10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	//glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();

	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}

	setUBO();
	glBindBufferRange(GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);

	//bind shader


	glm::mat4 model_matrix = glm::mat4();
	model_matrix = glm::translate(model_matrix, this->source_pos);
	model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 10.0f, 10.0f));
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, (float)NEAR, (float)FAR);
	glm::mat4 view = camera.GetViewMatrix();
	this->texture->bind(0);

	////basic light
	//basic_light_shader->use();	
	//glm::vec3 lightPos(100.0f, 50.0f, 150.0f);
	//basic_light_shader->setVec3("objectColor", 0.8f, 0.5f, 0.6f);
	//basic_light_shader->setVec3("lightColor", 0.9f, 1.0f, 0.9f);
	//basic_light_shader->setVec3("lightPos", lightPos);
	//basic_light_shader->setVec3("viewPos", camera.Position);
	//// view/projection transformations
	//projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, (float)NEAR, (float)FAR);
	//view = camera.GetViewMatrix();
	//basic_light_shader->setMat4("projection", projection);
	//basic_light_shader->setMat4("view", view);
	//basic_light_shader->setMat4("model", model_matrix);

	glm::mat4 model = glm::mat4(1.0f);

	//directional light
	if (tw->lightBrowser->value() == 1) {
		directional_light_shader->use();
		directional_light_shader->setVec3("light.direction", -1.0f, -0.1f, -0.3f);
		directional_light_shader->setVec3("viewPos", camera.Position);
		// light properties
		directional_light_shader->setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
		directional_light_shader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
		directional_light_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		// material properties
		directional_light_shader->setFloat("material.shininess", 32.0f);
		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)w() / (float)h(), (float)NEAR, (float)FAR);
		view = camera.GetViewMatrix();
		directional_light_shader->setMat4("projection", projection);
		directional_light_shader->setMat4("view", view);
		// world transformation
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5.0, 5.0, 5.0));
		//model = glm::rotate(model, float(now_t/1000.0), glm::vec3(0, 1, 0));
		directional_light_shader->setMat4("model", model);
	}
	
	//point light
	if (tw->lightBrowser->value() == 2) {
		point_light_shader->use();
		glm::vec3 lightPos(50.0f, 30.0f, 2.0f);
		point_light_shader->setVec3("light.position", lightPos);
		point_light_shader->setVec3("viewPos", camera.Position);

		// light properties
		point_light_shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		point_light_shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		point_light_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		point_light_shader->setFloat("light.constant", 0.01f);
		point_light_shader->setFloat("light.linear", 0.001f);
		point_light_shader->setFloat("light.quadratic", 0.001f);

		// material properties
		point_light_shader->setFloat("material.shininess", 32.0f);

		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)w() / (float)h(), (float)NEAR, (float)FAR);
		view = camera.GetViewMatrix();
		point_light_shader->setMat4("projection", projection);
		point_light_shader->setMat4("view", view);

		// world transformation
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5.0, 5.0, 5.0));
		point_light_shader->setMat4("model", model);
	}

		//spot light
	if (tw->lightBrowser->value() == 3) {
		spot_light_shader->use();
		spot_light_shader->setVec3("light.position", camera.Position);
		spot_light_shader->setVec3("light.direction", camera.Front);
		spot_light_shader->setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
		spot_light_shader->setVec3("viewPos", camera.Position);
		// light properties
		spot_light_shader->setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
		// we configure the diffuse intensity slightly higher; the right lighting conditions differ with each lighting method and environment.
		// each environment and lighting type requires some tweaking to get the best out of your environment.
		spot_light_shader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
		spot_light_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		spot_light_shader->setFloat("light.constant", 1.0f);
		spot_light_shader->setFloat("light.linear", 0.09f);
		spot_light_shader->setFloat("light.quadratic", 0.032f);
		// material properties
		spot_light_shader->setFloat("material.shininess", 32.0f);
		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)w() / (float)h(), (float)NEAR, (float)FAR);
		view = camera.GetViewMatrix();
		spot_light_shader->setMat4("projection", projection);
		spot_light_shader->setMat4("view", view);
		// world transformation
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5.0, 5.0, 5.0));
		spot_light_shader->setMat4("model", model);
	}
	






	
	//bind VAO
	glBindVertexArray(this->plane->vao);

	glDrawElements(GL_TRIANGLES, this->plane->element_amount, GL_UNSIGNED_INT, 0);

	//test_model->Draw(*directional_light_shader);
	if (tw->lightBrowser->value() == 1) {
		test_model->Draw(*directional_light_shader);
	}
	else if (tw->lightBrowser->value() == 2) {
		test_model->Draw(*point_light_shader);
	}
	else if (tw->lightBrowser->value() == 3) {
		test_model->Draw(*spot_light_shader);
	}
	
	model_matrix = glm::translate(model_matrix, glm::vec3(30.0f, 30, 30));

	//unbind VAO
	glBindVertexArray(0);

	//unbind shader(switch to fixed pipeline)
	glUseProgram(0);
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value()) {
		//arcball.setProjection(false);
		updata_camera();
	}
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		} 
		else {
			he = 110;
			wi = he * aspect;
		}
		
		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);
	} 
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this,aspect);
#endif
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	if (!tw->trainCam->value()) {
		for(size_t i=0; i<m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if ( ((int) i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();		

	// where is the mouse?
	int mx = Fl::event_x(); 
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
						5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for(size_t i=0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint) (i+1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3]-1;
	} else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n",selectedCube);
}

void TrainView::setUBO()
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	//HMatrix view_matrix; 
	//this->arcball.getMatrix(view_matrix);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	//projection_matrix = glm::perspective(glm::radians(this->arcball.getFoV()), (GLfloat)wdt / (GLfloat)hgt, 0.01f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TrainView::updateTimer() {
	now_t = glutGet(GLUT_ELAPSED_TIME);
	delta_t = (now_t - old_t) / 1000.0;
	old_t = now_t;
	//cout << "delta time: " << delta_t << endl;
	//glutPostRedisplay();
}

void TrainView::updata_camera() {
	if (k_pressed) {
		if (k == 'w') {
			camera.ProcessKeyboard(FORWARD, delta_t);
			damage(1);
		}
		if (k == 's') {
			camera.ProcessKeyboard(BACKWARD, delta_t);
			damage(1);
		}
		if (k == 'a') {
			camera.ProcessKeyboard(LEFT, delta_t);
			damage(1);
		}
		if (k == 'd') {
			camera.ProcessKeyboard(RIGHT, delta_t);
			damage(1);
		}
	}
	
	//cout << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << endl;
}