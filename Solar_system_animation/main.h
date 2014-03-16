#ifndef MAIN_H
#define MAIN_H

class Camera
{
public:
	float distance;
	float cx, cy, cz;
	float z_angle, xy_angle;

	void init()
	{
		cx = cy = cz = 0.0f;
		distance = 320.0f;
		z_angle = xy_angle = 0.0f;
	}
};

class Light
{
public:
	VECTOR3D position;
	COLOR color;
};

class Planet
{
public:
	VECTOR3D position;
	float rotationAngle, orbitAngle, increment, distance, radius;
	COLOR color;
	VECTOR3D objectSpaceLightPosition;
	bool texturized;
	GLuint texture;
};

void init();

void Render();
void Resize(int width, int height);
void Menu( int value);
void Mouse( int button, int state, int x, int y);
void Motion( int x, int y);
void Special( int key, int x, int y);
void Keyboard( unsigned char key, int x, int y);
void updateCamera();
void updateFrame();
GLuint LoadTextureRAW( const char * filename, int width, int height );
void FreeTexture( GLuint texture);
void initPlanets();
void initTextures();
void drawScene();

#endif