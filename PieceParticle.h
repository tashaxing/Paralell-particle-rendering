#pragma once
#include "DXUT.h"
#include <time.h>
#include <math.h>
//the vertex struct 
struct PIECEVERTEX
{
	float x,y,z; //vertex position
	float u,v;
};
#define D3DFVF_PIECEVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

//Piece particle definition
struct Piece
{
	float x,y,z;         //position
	float vx,vy,vz;    //speed
	float xrand;     //disturb
	float yrand;
	float zrand;
};

//-------------------------------------------------------------------------------------------------
// Desc:Piece particle definition
//-------------------------------------------------------------------------------------------------
class PieceParticle
{
private:
	LPDIRECT3DDEVICE9    m_pd3dDevice;
	Piece                 *m_pieceParticle;
	LPDIRECT3DVERTEXBUFFER9   m_pVertexBuffer;     //piece particle vertex buffer
	LPDIRECT3DTEXTURE9             m_pTexture;             //piece particle texture
	long m_particleNumber;  //number of particle
public:
	PieceParticle(IDirect3DDevice9 *pd3dDevice,long particleNumber);
	~PieceParticle(void);
	void InitPieceParticle(float size,WCHAR *file);
	void UpdatePieceParticle(float speedLevel);
	void RenderPieceParticle(D3DXMATRIX &matView);   //transfer the view matrix for billboard

	bool isReset;                    //indicate device reset
	void IsResetDevice();
};

