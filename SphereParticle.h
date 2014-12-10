#pragma once
#include "DXUT.h"
#include <time.h>
#include <math.h>
struct Sphere
{
	float x,y,z;   //position
	float vx,vy,vz;  //speed
	//disturb
	float xrand;
	float yrand;
	float zrand;
};
class SphereParticle
{
private:
	IDirect3DDevice9 *m_pd3dDevice;
    ID3DXMesh *m_pSphere;   //the sphere D3D com port
	Sphere *m_SphereParticle;
	long m_particleNumber;  //number of particle
public:
	SphereParticle(IDirect3DDevice9 *pd3dDevice,long particleNumber);
	~SphereParticle(void);
	void InitSphereParticle(float radius,UINT slices,UINT stacks);     //initial the sphere particle
	void UpdateSphereParticle(float speedLevel);   //update the sphere particle,use speedLevel to control the move speed
	void RenderSphereParticle();   //render the sphere particle

	bool isReset;                   //indicate device reset
	void IsResetDevice();
};

