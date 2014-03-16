// Solar_system_animation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include "glut\glut.h"
#include <gl\GL.h>
#include "Maths\Maths.h"
#include "ShadowModel.h"
#include "main.h"
#include "s2ws.h"

COLOR backgroundColor(0.0f, 0.0f, 0.0f, 0.0f);

int click_x, click_y;

int camFocus = 0;

float speed;

int winWidth = 1024, winHeight = 768;

Camera cam;

Light light;

bool texturized;

const int numPlanets = 8;
Planet planets[ numPlanets ];
ShadowModel models[ numPlanets ];
Planet sun;
ShadowModel sunM;

MATRIX4X4 projectionMatrix;

int MenuID;

int main(int argc, char* argv[])
{
	glutInit( &argc, argv);

	init();

	glutMainLoop();
	return 0;
}

void initPlanets()
{
	planets[0].color.Set( 1.0f, 0.5f, 0.0f, 0.0f);
	planets[1].color.Set( 1.0f, 1.0f, 0.0f, 0.0f);
	planets[2].color.Set( 0.0f, 1.0f, 0.5f, 0.0f);
	planets[3].color.Set( 1.0f, 0.3f, 0.0f, 0.0f);
	planets[4].color.Set( 1.0f, 0.62f, 0.2f, 0.0f);
	planets[5].color.Set( 1.0f, 1.0f, 1.0f, 0.0f);
	planets[6].color.Set( 0.6f, 0.8f, 1.0f, 0.0f);
	planets[7].color.Set( 0.2f, 1.0f, 0.6f, 0.0f);
	sun.color.Set(0.8f, 0.8f, 0.8f, 0.0f);
	
	planets[0].distance = 6.0;
	planets[1].distance = 9.2;
	planets[2].distance = 13.5;
	planets[3].distance = 20.0;
	planets[4].distance = 52.0;
	planets[5].distance = 95.0;
	planets[6].distance = 150.0;
	planets[7].distance = 220.0;
	
	planets[0].increment = 0.006;
	planets[1].increment = 0.004;
	planets[2].increment = 0.9;
	planets[3].increment = 0.5;
	planets[4].increment = 0.62;
	planets[5].increment = 0.5;
	planets[6].increment = 0.29;
	planets[7].increment = 0.234;

	planets[0].radius = 0.29;
	planets[1].radius = 0.60;
	planets[2].radius = 0.63;
	planets[3].radius = 0.33;
	planets[4].radius = 5.14;
	planets[5].radius = 4.02;
	planets[6].radius = 1.96;
	planets[7].radius = 1.93;
	sun.radius = 1.7;

	planets[0].orbitAngle = 0.45;
	planets[1].orbitAngle = 0.35;
	planets[2].orbitAngle = 0.298;
	planets[3].orbitAngle = 0.24;
	planets[4].orbitAngle = 0.13;
	planets[5].orbitAngle = 0.0964;
	planets[6].orbitAngle = 0.068;
	planets[7].orbitAngle = 0.0543;

	for(int i = 0; i < numPlanets; i++)
	{
		planets[i].position.Set(planets[i].distance, 0.0f, 0.0f);
		planets[i].texturized = true;
	}
	sun.texturized = true;
}

void initTextures()
{
	planets[0].texture = LoadTextureRAW( "mercury.raw" , 512, 256);
	planets[1].texture = LoadTextureRAW( "venus.raw" , 512, 256);
	planets[2].texture = LoadTextureRAW( "earth.raw" , 1024, 512);
	planets[3].texture = LoadTextureRAW( "mars.raw" , 512, 256);
	planets[4].texture = LoadTextureRAW( "jupiter.raw" , 512, 256);
	planets[5].texture = LoadTextureRAW( "saturn.raw" , 512, 256);
	planets[6].texture = LoadTextureRAW( "uranus.raw" , 512, 256);
	planets[7].texture = LoadTextureRAW( "neptune.raw" , 512, 256);
	sun.texture = LoadTextureRAW( "sun.raw", 512, 256);
}

void init()
{
	initPlanets();

	//Set up models
	for( int i = 0; i < numPlanets; i++)
		models[i].GenerateSphere(planets[i].radius);
	sunM.GenerateSphere(sun.radius);
	
	//Set up light
	light.position.Set(0.0f, 0.0f, 0.0f);
	light.color = white;

	cam.init();
	speed = 1.0;
	
	//set up the projection matrix
	projectionMatrix.SetPerspective(45.0f, 1.0f, 1.0f, -1.0f);
	
	//Set up GL and glut
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL ); //Render mode

	glutInitWindowSize( winWidth , winHeight ); //Window size

	glutInitWindowPosition( 70, 70); //Window initial position

	glutCreateWindow( "Solar System Animation (by Deyan Hadzhiev)"); //Creating the window

	glutDisplayFunc( Render); //define Render function

	glutReshapeFunc( Resize); //define Reshape function

	glutMouseFunc( Mouse); //define Mouse function

	glutMotionFunc( Motion); //define Motion function

	glutSpecialFunc( Special); //define Special function

	glutKeyboardFunc( Keyboard); //define Keyboard function

	MenuID = glutCreateMenu( Menu); //define Menu function

	glutSetMenu( MenuID); //setting the <MenuID>, as current menu

	glutAddMenuEntry("Toggle textures", 1); //Texture control
	glutAddMenuEntry("Help", 2); // Help about controls
	glutAddMenuEntry("About", 3); // Information about program
	glutAddMenuEntry("Exit", 4); // Exit menu

	glutAttachMenu( GLUT_RIGHT_BUTTON); //setting menu activation button

	//Set up matrixs and etc.
	glMatrixMode (GL_PROJECTION);

	glLoadMatrixf(projectionMatrix);

	glMatrixMode( GL_MODELVIEW);

	glLoadIdentity();
	
	//Other states
	glShadeModel( GL_SMOOTH);
	glClearColor(backgroundColor.r,	backgroundColor.g, backgroundColor.b, backgroundColor.a);
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//depth
	glClearDepth(1.0f);
	glEnable( GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);
	//hints

	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable( GL_CULL_FACE);
	
	//client states
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	initTextures();
}

void Resize( int width, int height)
{
	if(height == 0) height = 1; // avoid dividing by zero

	glViewport( 0, 0, width, height); // we draw on the hole window

	glMatrixMode( GL_PROJECTION); // switch to Projection matrix

	glLoadIdentity(); // loading the initial Projection matrix

	gluPerspective( 45, (GLdouble) width / height, 1, -1); // defining the new 3D view

	glMatrixMode( GL_MODELVIEW); //switching back to Model View matrix

	glLoadIdentity(); //loading initial Model View matrix
}

void Menu( int value)
{
	int result;	//this is for the result of a pop-up windows
	std::wstring title; //for conversint of strings
	std::wstring body;
	std::string str;

	switch( value)
	{
	case 1:
		for(int i = 0; i < numPlanets; i++)
			planets[i].texturized = !planets[i].texturized;
		sun.texturized = !sun.texturized;
		break;
	case 2:
		title = s2ws("Help");
		str = "This are the main key shortcuts:\n";
		str = str + "  [F1] - Help Menu\n" + "  [Home/MMB] - Reset camera\n" + "  [Hold LMB] + [Mouse direction] - Control camera angle of view\n";
		str = str + "  [t] Toggle textures\n" + "  [Up/Down] - Zoom in/out\n" + "  [+/-] - Speed up/down animation by 50 \% \n";
		str = str + "  [Left/Right] - Change focus of planet\n" + "  [0 .. 8] - Focus planet 'n' (0 for sun)\n" + "  [p] - Pause / unpause animation\n";
		str = str + "  [r] - Reset the animation\n" + "  [Esc] - Quit\n";
		body = s2ws(str);
		MessageBox( NULL, body.c_str(), title.c_str(), MB_OK); // Creating of message box
		break;
	case 3:
		title = s2ws("About");
		body = s2ws("This project was created by Deyan Hadzhiev\n Computer Science, Sofia University\n FN: 80592");
		MessageBox( NULL, body.c_str(), title.c_str(), MB_OK);
		break;
	case 4:
		title = s2ws("Quit");
		body = s2ws("Do you really want to quit?");
		result = MessageBox( NULL, body.c_str(), title.c_str(), MB_YESNO);
		if( result == IDYES) exit(0);
		break;
	}
}

void Mouse( int button, int state, int x, int y)
{
	click_x = x;
	click_y = y;

	if(button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN){
		cam.init();
		speed = 1.0;
		camFocus = 0;
	}
}

void Motion( int x, int y)
{
	cam.xy_angle += (x - click_x)/2;
	int dy = (y - click_y);
	if(cam.z_angle > -90 && cam.z_angle < 90)
		cam.z_angle += dy/2;
	else if((cam.z_angle <= -90 && dy > 0) || (cam.z_angle >= 90 && dy < 0))
		cam.z_angle += dy/2;

	click_x = x;
	click_y = y;
}

void Special( int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		Menu(2);
		break;
	case GLUT_KEY_HOME:
		cam.init();
		speed = 1.0;
		camFocus = 0.0;
		break;
	case GLUT_KEY_LEFT:
		camFocus--;
		if(camFocus <= 0) 
		{
			camFocus = 0;
			cam.init();
		}else
		{
			cam.distance = planets[camFocus - 1].radius * 5;
		}
		break;
	case GLUT_KEY_RIGHT:
		camFocus++;
		if(camFocus > numPlanets) camFocus = numPlanets;
		cam.distance = planets[camFocus - 1].radius * 5;
		break;
	case GLUT_KEY_DOWN:
		if(cam.distance < 700.0f) cam.distance *= 1.15;
		break;
	case GLUT_KEY_UP:
		if(cam.distance > 7.5f) cam.distance *= 0.85;
		break;
	}
}

void Keyboard( unsigned char key, int x, int y)
{
	static float tspeed = speed;
	switch (key)
	{
	case 27:
		Menu(4);
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		camFocus = (int) key - '0';
		if(camFocus == 0) cam.init();
		else cam.distance = planets[camFocus - 1].radius * 5;
		break;
	case '+':
		speed *= 1.25;
		break;
	case '-':
		speed *= 0.75;
		break;
	case 'p':
		if(speed < 0.00001)
		{
			speed = tspeed;
		}
		else
		{
			tspeed = speed;
			speed = 0.0;
		}
		break;
	case 't':
		Menu(1);
		break;
	case 'r':
		initPlanets();
		break;
	}
}

GLuint LoadTextureRAW( const char * filename, int width, int height)
{    
  GLuint texture;
    
  unsigned char * data;
  FILE * file;
  
  file = fopen( filename, "rb" ); 
  if ( file == NULL ) return 0;
    
  data = (unsigned char *)malloc( width * height * 3 );  
  fread( data, width * height * 3, 1, file );
    
  fclose( file ); 
  glGenTextures( 1, &texture );
  
  glBindTexture( GL_TEXTURE_2D, texture );  
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST ); 
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );
  
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );  
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );    
  free( data );  

  return texture;
} 

void FreeTexture( GLuint texture)
{
	glDeleteTextures( 1, &texture);
}

void updateCamera()
{

	if(camFocus != 0)
	{
		cam.cx = -planets[camFocus - 1].position.x;
		cam.cy = -planets[camFocus - 1].position.y;
		cam.cz = -planets[camFocus - 1].position.z;
	}
	gluLookAt(cam.distance, 0, 0, 0, 0, 0, 0, 0, 1);

	glRotatef(cam.z_angle, 0, 1, 0);
	glRotatef(cam.xy_angle, 0, 0 ,1);
	glTranslatef(cam.cx, cam.cy, cam.cz);
}

void updateFrame()
{
	//Update planet positions and angles
	for(int i = 0; i < numPlanets; i++)
	{
		planets[i].position.RotateZ(planets[i].orbitAngle * speed);
		planets[i].rotationAngle += planets[i].increment * speed;
		if(planets[i].rotationAngle > 360.0) planets[i].rotationAngle -= 360.0;

		MATRIX4X4 inverseModelMatrix;

		glPushMatrix();
		glLoadIdentity();
		glRotatef(-planets[i].rotationAngle, 0.0f, 0.0f, 1.0f);
		glTranslatef(-planets[i].position.x, -planets[i].position.y, -planets[i].position.z);
		glGetFloatv(GL_MODELVIEW_MATRIX, inverseModelMatrix);
		glPopMatrix();

		//Set the object space light positions
		planets[i].objectSpaceLightPosition = inverseModelMatrix * light.position;
	}

}

void Render() {

	updateFrame();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glLoadIdentity();

	updateCamera();

	//Draw ambient
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT1, GL_POSITION, VECTOR4D(light.position)); 
	glLightfv(GL_LIGHT1, GL_AMBIENT, light.color/10);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light.color/9);
	glLightfv(GL_LIGHT1, GL_SPECULAR, black);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	//Draw models
	drawScene();

	glPopAttrib();

	//Draw shadow volumes
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glColorMask(0, 0, 0, 0);
	glShadeModel(GL_FLAT);
	glDepthMask(0);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);

	for(int i = 0; i < numPlanets; ++i)
	{
		models[i].CalculateSilhouetteEdges(planets[i].objectSpaceLightPosition);

		glPushMatrix();
		glTranslatef(planets[i].position.x, planets[i].position.y, planets[i].position.z);
		glRotatef(planets[i].rotationAngle, 0.0f, 0.0f, 1.0f);

		glStencilFunc(GL_ALWAYS, 0, ~0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glCullFace(GL_BACK);
		//Draw the shadow volume
		models[i].DrawInfiniteShadowVolume(	planets[i].objectSpaceLightPosition);

		//Decrement stencil buffer for back face depth pass
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		glCullFace(GL_FRONT);
		//Draw the shadow volume
		models[i].DrawInfiniteShadowVolume(	planets[i].objectSpaceLightPosition);
				
		glPopMatrix();

	}

	glPopAttrib();

	//Now draw lit where stencil==0
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT1, GL_POSITION, VECTOR4D(light.position)); 
	glLightfv(GL_LIGHT1, GL_AMBIENT, light.color/10);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light.color);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_STENCIL_TEST);

	//Draw models
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialf(GL_FRONT, GL_SHININESS, 32.0f);

	drawScene();

	glPopAttrib();

	//Draw sphere at light's position
	glPushMatrix();
	glColor4fv(sun.color);
	glTranslatef(light.position.x, light.position.y, light.position.z);

	glVertexPointer(3, GL_FLOAT, sizeof(ShadowModelVertex), &sunM.vertices[0].position);
	glNormalPointer(GL_FLOAT, sizeof(ShadowModelVertex), &sunM.vertices[0].normal);

	if(sun.texturized)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glColor4fv(white);
		glBindTexture(GL_TEXTURE_2D, sun.texture);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexCoordPointer(2, GL_FLOAT, sizeof( ShadowModelVertex), &sunM.vertices[0].texture);
	}
		
	glDrawElements(GL_TRIANGLES, sunM.numIndices, GL_UNSIGNED_INT, sunM.indices);

	if(sun.texturized) 
	{
		glDisableClientState( GL_TEXTURE_COORD_ARRAY);
	}
	glPopMatrix();

	glutSwapBuffers();

	glutPostRedisplay();
}

void drawScene()
{
	for(int i=0; i < numPlanets; ++i)
	{
		glColor4fv(planets[i].color);
		glPushMatrix();
		glTranslatef(planets[i].position.x, planets[i].position.y, planets[i].position.z);
		glRotatef(planets[i].rotationAngle, 0.0f, 0.0f, 1.0f);		

		glVertexPointer(3, GL_FLOAT, sizeof(ShadowModelVertex), &models[i].vertices[0].position);
		glNormalPointer(GL_FLOAT, sizeof(ShadowModelVertex), &models[i].vertices[0].normal);
		
		if(planets[i].texturized)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glColor4fv(white);
			glBindTexture(GL_TEXTURE_2D, planets[i].texture);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexCoordPointer(2, GL_FLOAT, sizeof( ShadowModelVertex), &models[i].vertices[0].texture);
		}

		glDrawElements(GL_TRIANGLES, models[i].numIndices, GL_UNSIGNED_INT, models[i].indices);

		if(planets[i].texturized) 
		{
			glDisableClientState( GL_TEXTURE_COORD_ARRAY);
		}

		glPopMatrix();
	}
}