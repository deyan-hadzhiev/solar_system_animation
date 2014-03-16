#include <Windows.h>
#include <gl\GL.h>
#include "Maths\Maths.h"
#include "ShadowModel.h"
#include <iostream>

bool ShadowModel::GenerateSphere(float radius)
{
	int spherePrecision= 50;

	numVertices = spherePrecision * (spherePrecision + 1) + 2;
	numIndices = 6*spherePrecision * spherePrecision;
	numTriangles=numIndices/3;

	vertices=new ShadowModelVertex[numVertices];
	if(!vertices)
	{
		std::cout << "Not enough space for sphere vertices";
		return false;
	}

	indices=new unsigned int[numIndices];
	if(!indices)
	{
		std::cout << "Not enough space for sphere indices";
		return false;
	}
	
	//set up the upmost and downmost points
	vertices[0].position = VECTOR3D( 0.0f, 0.0f, radius);
	vertices[0].normal = VECTOR3D(0.0f, 0.0f, 1.0f);
	vertices[0].texture = VECTOR2D( 0.5f, 0.0f);
	vertices[numVertices - 1].position = VECTOR3D( 0.0f, 0.0f, -radius);
	vertices[numVertices - 1].normal = VECTOR3D( 0.0f, 0.0f, -1.0f);
	vertices[numVertices - 1].texture = VECTOR2D( 0.5f, 1.0f);
	
	//generate the initial half circle
	for( int i = 1; i < spherePrecision + 1; i++)
	{
		vertices[i].position = VECTOR3D(0.0f, 0.0f, radius ).GetRotatedY(i * 180.0f / (spherePrecision + 1));
		vertices[i].normal = VECTOR3D(0.0f, 1.0f, 0.0f).
							CrossProduct(VECTOR3D(-1.0f, 0.0f, 0.0f).GetRotatedY(i* 180.0f/ (spherePrecision + 1)));
		vertices[i].texture = VECTOR2D( 0.0f,(float) i / (float) spherePrecision);
	}
	
	//rotate it to get a sphere
	for( int rot = 1; rot < spherePrecision + 1; rot++)
	{
		for( int i = 1; i < spherePrecision + 1; i++)
		{
			vertices[rot * (spherePrecision ) + i].position = vertices[i].position.GetRotatedZ( rot * 360.0f/ spherePrecision);
			vertices[rot * (spherePrecision ) + i].normal = vertices[i].normal.GetRotatedZ( rot * 360.0f / spherePrecision);
			vertices[rot * (spherePrecision ) + i].texture = vertices[i].texture + VECTOR2D((float) rot/ (float) spherePrecision, 0.0f);
		}
	}

	//calculate the indices
	for(int ring=0; ring<spherePrecision; ring++)
	{
		indices[ring * spherePrecision*6] = 0;
		indices[ring * spherePrecision*6 + 1] = ring*spherePrecision + 1;
		indices[ring * spherePrecision*6 + 2] = (ring + 1)*spherePrecision + 1;
		for(int i=0; i< spherePrecision - 1; i++)
		{
			indices[((ring*spherePrecision+i)*2)*3+3]=ring*spherePrecision + i + 1;
			indices[((ring*spherePrecision+i)*2)*3+4]=(ring+1)*spherePrecision + i + 2;
			indices[((ring*spherePrecision+i)*2)*3+5]=(ring + 1)*spherePrecision + i + 1;
			indices[((ring*spherePrecision+i)*2+1)*3+3]=ring * spherePrecision + i + 1;
			indices[((ring*spherePrecision+i)*2+1)*3+4]=ring * spherePrecision + i + 2;
			indices[((ring*spherePrecision+i)*2+1)*3+5]=(ring+1)*spherePrecision + i + 2;
		}
		indices[(ring + 1) * spherePrecision*6 - 3] = (ring + 1)*spherePrecision;
		indices[(ring + 1) * spherePrecision*6 - 2] = numVertices - 1;
		indices[(ring + 1) * spherePrecision*6 - 1] = (ring + 2)*spherePrecision;
	}
	

	//Calculate the plane equation for each face
	planeEquations=new PLANE[numTriangles];
	if(!planeEquations)
	{
		std::cout << "Unable to allocate memory for planes";
		return false;
	}

	for(unsigned int j=0; j<numTriangles; ++j)
	{
		planeEquations[j].SetFromPoints(vertices[indices[j*3+0]].position,
										vertices[indices[j*3+1]].position,
										vertices[indices[j*3+2]].position);
	}

	//Create space for the "is facing light" booleans
	isFacingLight=new bool[numTriangles];
	if(!isFacingLight)
	{
		std::cout << "Unable to allocate memory for " << numTriangles << " booleans.";
		return false;
	}

	//Create space for connectivity data
	neighbourIndices=new GLint[numTriangles*3];
	if(!neighbourIndices)
	{
		std:: cout << "Unable to allocate memory for"<< numTriangles << " neighbour indices";
		return false;
	}

	//Create space for "is silhouette edge" booleans
	isSilhouetteEdge=new bool[numTriangles*3];
	if(!isSilhouetteEdge)
	{
		std::cout << "Unable to allocate memory for "<< numTriangles <<  " booleans";
		return false;
	}

	//Calculate the neighbours
	SetConnectivity();

	return true;
}


//Calculate neighbour faces for each edge
void ShadowModel::SetConnectivity()
{
	//set the neighbour indices to be -1
	for(unsigned int i=0; i<numTriangles*3; ++i)
		neighbourIndices[i]=-1;

	//loop through triangles
	for(unsigned int i=0; i<numTriangles-1; ++i)
	{
		//loop through edges on the first triangle
		for(int edgeI=0; edgeI<3; ++edgeI)
		{
			//continue if this edge already has a neighbour set
			if(neighbourIndices[i*3+edgeI]!=-1)
				continue;

			//loop through triangles with greater indices than this one
			for(unsigned int j=i+1; j<numTriangles; ++j)
			{
				//loop through edges on triangle j
				for(int edgeJ=0; edgeJ<3; ++edgeJ)
				{
					//get the vertex indices on each edge
					int edgeI1=indices[i*3+edgeI];
					int edgeI2=indices[i*3+(edgeI+1)%3];
					int edgeJ1=indices[j*3+edgeJ];
					int edgeJ2=indices[j*3+(edgeJ+1)%3];

					//if these are the same (possibly reversed order), these faces are neighbours
					if(		(edgeI1==edgeJ1 && edgeI2==edgeJ2)
						||	(edgeI1==edgeJ2 && edgeI2==edgeJ1))
					{
						neighbourIndices[i*3+edgeI]=j;
						neighbourIndices[j*3+edgeJ]=i;
					}
				}
			}
		}
	}
}

//calculate silhouette edges
void ShadowModel::CalculateSilhouetteEdges(VECTOR3D lightPosition)
{
	//Calculate which faces face the light
	for(unsigned int i=0; i<numTriangles; ++i)
	{
		if(planeEquations[i].ClassifyPoint(lightPosition)==POINT_IN_FRONT_OF_PLANE)
			isFacingLight[i]=true;
		else
			isFacingLight[i]=false;
	}

	//loop through edges
	for(unsigned int i=0; i<numTriangles*3; ++i)
	{
		//if this face is not facing the light, not a silhouette edge
		if(!isFacingLight[i/3])
		{
			isSilhouetteEdge[i]=0;
			continue;
		}

		//this face is facing the light
		//if the neighbouring face is not facing the light, or there is no neighbouring face,
		//then this is a silhouette edge
		if(neighbourIndices[i]==-1 || !isFacingLight[neighbourIndices[i]])
		{
			isSilhouetteEdge[i]=1;
			continue;
		}

		isSilhouetteEdge[i]=0;
	}
}

void ShadowModel::DrawInfiniteShadowVolume(VECTOR3D lightPosition)
{
	glBegin(GL_QUADS);
	{
		for(unsigned int i=0; i<numTriangles; ++i)
		{
			//if this face does not face the light, continue
			if(!isFacingLight[i])
				continue;

			//Loop through edges on this face
			for(int j=0; j<3; ++j)
			{
				//Draw the shadow volume "edge" if this is a silhouette edge
				if(isSilhouetteEdge[i*3+j])
				{
					VECTOR3D vertex1=vertices[indices[i*3+j]].position;
					VECTOR3D vertex2=vertices[indices[i*3+(j+1)%3]].position;

					glVertex3fv(vertex2);
					glVertex3fv(vertex1);
					glVertex4f(	vertex1.x-lightPosition.x,
								vertex1.y-lightPosition.y,
								vertex1.z-lightPosition.z, 0.0f);
					glVertex4f(	vertex2.x-lightPosition.x,
								vertex2.y-lightPosition.y,
								vertex2.z-lightPosition.z, 0.0f);
				}
			}
		}
	}
	glEnd();
}
