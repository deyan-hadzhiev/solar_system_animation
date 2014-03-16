#ifndef SHADOWMODEL_H
#define SHADOWMODEL_H

class ShadowModelVertex
{
public:
	VECTOR3D position;
	VECTOR3D normal;
	VECTOR2D texture;
};

class ShadowModel
{
public:
	GLuint numIndices;
	GLuint * indices;

	GLuint numTriangles;

	GLuint numVertices;
	ShadowModelVertex * vertices;

	//Store the plane equation for each face
	PLANE * planeEquations;

	//For each face, does it face the light?
	bool * isFacingLight;

	//For each edge, which is the neighbouring face?
	GLint * neighbourIndices;

	//For each edge, is it a silhouette edge?
	bool * isSilhouetteEdge;

	bool GenerateSphere(float radius);

	void SetConnectivity();
	void CalculateSilhouetteEdges(VECTOR3D lightPosition);
	void DrawInfiniteShadowVolume(VECTOR3D lightPosition);
};

#endif 