#include "PieceParticle.h"


PieceParticle::PieceParticle(IDirect3DDevice9 *pd3dDevice,long particleNumber)
{
	m_pd3dDevice=pd3dDevice;
	m_pVertexBuffer=NULL;
	m_particleNumber=particleNumber;
	isReset=false;
}

void PieceParticle::InitPieceParticle(float size,WCHAR *file)
{
	srand(time(0));
	m_pieceParticle=new Piece[m_particleNumber];
	for(int i=0;i<m_particleNumber;i++)
	{
		m_pieceParticle[i].x=0.0f;
		m_pieceParticle[i].y=0.0f;
		m_pieceParticle[i].z=0.0f;
		m_pieceParticle[i].vx=0.04f;
		m_pieceParticle[i].vy=0.02f;
		m_pieceParticle[i].vz=float(-400+rand()%900)/500000;
		m_pieceParticle[i].xrand=0;
		m_pieceParticle[i].yrand=0;
		m_pieceParticle[i].zrand=0;
	}

	//create particle vertexbuffer
	m_pd3dDevice->CreateVertexBuffer(4*sizeof(PIECEVERTEX),0,D3DFVF_PIECEVERTEX,D3DPOOL_MANAGED,&m_pVertexBuffer,NULL);
	//fill the vertexbuffer
	PIECEVERTEX vertices[]=
	{
		{ -size/2, -size/2, 0.0f,   0.0f, 1.0f, },
		{ -size/2, size/2, 0.0f,   0.0f, 0.0f, },
		{  size/2, -size/2, 0.0f,   1.0f, 1.0f, }, 
		{  size/2, size/2, 0.0f,   1.0f, 0.0f, }
	};
	//lock
	VOID *pVertices;
	m_pVertexBuffer->Lock(0,sizeof(vertices),(void **)&pVertices,0);
	//visit
	memcpy(pVertices,vertices,sizeof(vertices));
	//unlock
	m_pVertexBuffer->Unlock();

	//create texture
	D3DXCreateTextureFromFile(m_pd3dDevice,file,&m_pTexture);
}

void PieceParticle::UpdatePieceParticle(float speedLevel)
{
	//update
	//actually the move speed depends on the FPS value sometimes
	static int beginningFlag=0;  //the first render,don't want the particle be generated together
	static int renderCount=1;//count the render sequence number
	srand(time(0));
	if(!beginningFlag)
	{
		for(int i=0;i<renderCount;i++)
		{
			m_pieceParticle[i].x+=m_pieceParticle[i].vx;
			m_pieceParticle[i].y+=m_pieceParticle[i].vy;
			m_pieceParticle[i].z+=m_pieceParticle[i].vz;
			if(fabs(m_pieceParticle[i].vx)<0.3&&fabs(m_pieceParticle[i].vx)>0.0001
				&&fabs(m_pieceParticle[i].vy)<0.3&&fabs(m_pieceParticle[i].vy)>0.0001
				&&fabs(m_pieceParticle[i].vz)<0.3&&fabs(m_pieceParticle[i].vz)>0.0001
				&&m_pieceParticle[i].y>0&&m_pieceParticle[i].x>0)
			{
				m_pieceParticle[i].vx=m_pieceParticle[i].vx+float(rand()%50-25)/30000;
				m_pieceParticle[i].vy=m_pieceParticle[i].vy+float(rand()%50-25)/30000;
				m_pieceParticle[i].vz=m_pieceParticle[i].vz+float(rand()%50-25)/30000;
			}
			else
			{
				m_pieceParticle[i].x=m_pieceParticle[i].y=m_pieceParticle[i].z=0.0f;
				m_pieceParticle[i].vx=0.04f;
				m_pieceParticle[i].vy=0.02f;
				m_pieceParticle[i].vz=float(-400+rand()%900)/500000;
			}


		}
		renderCount++;
		if(renderCount>m_particleNumber)
			beginningFlag=1;
	}
	else
		for(int i=0;i<m_particleNumber;i++)
		{
			m_pieceParticle[i].x+=m_pieceParticle[i].vx;
			m_pieceParticle[i].y+=m_pieceParticle[i].vy;
			m_pieceParticle[i].z+=m_pieceParticle[i].vz;
			if(fabs(m_pieceParticle[i].vx)<0.3&&fabs(m_pieceParticle[i].vx)>0.0001
				&&fabs(m_pieceParticle[i].vy)<0.3&&fabs(m_pieceParticle[i].vy)>0.0001
				&&fabs(m_pieceParticle[i].vz)<0.3&&fabs(m_pieceParticle[i].vz)>0.0001
				&&m_pieceParticle[i].y>0&&m_pieceParticle[i].x>0)
			{
				m_pieceParticle[i].vx=m_pieceParticle[i].vx+float(rand()%50-25)/30000;
				m_pieceParticle[i].vy=m_pieceParticle[i].vy+float(rand()%50-25)/30000;
				m_pieceParticle[i].vz=m_pieceParticle[i].vz+float(rand()%50-25)/30000;
			}
			else
			{
				m_pieceParticle[i].x=m_pieceParticle[i].y=m_pieceParticle[i].z=0.0f;
				m_pieceParticle[i].vx=0.04f;
				m_pieceParticle[i].vy=0.02f;
				m_pieceParticle[i].vz=float(-400+rand()%900)/500000;
			}
		}
		//adjust the speed
		for(int i=0;i<m_particleNumber;i++)
		{
			m_pieceParticle[i].vx*=speedLevel;
			m_pieceParticle[i].vy*=speedLevel;
			m_pieceParticle[i].vz*=speedLevel;
		}

}

void PieceParticle::RenderPieceParticle(D3DXMATRIX &matView)
{
	//forbid the light
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING,false);
	//set the texture state
	m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
	m_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	m_pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	m_pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	//set alpha merge parameters
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	//do not leave out any surface
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);


	//render----------------
	//-----------------------
	static int beginningFlag=0;  //the first render,don't want the particle be generated together
	static int renderCount=1;//count the render sequence number
	//set matrix
	D3DXMATRIX matWorld;
	//根据取景变换矩阵来计算并构造公告板矩阵
	D3DXMATRIX matBillboard;
	D3DXMatrixIdentity(&matBillboard);
	matBillboard._11 = matView._11;
	matBillboard._13 = matView._13;
	matBillboard._31 = matView._31;
	matBillboard._33 = matView._33;

	//the other direction?
	matBillboard._12 = matView._12;
	matBillboard._21 = matView._21;
	matBillboard._22 = matView._22;

	//and?
	matBillboard._23 = matView._23;
	matBillboard._32 = matView._32;
	D3DXMatrixInverse(&matBillboard, NULL, &matBillboard);

	//this strategy is to let the particles not generate at the same time
	if(!beginningFlag)
	{
		for(int i=0;i<renderCount;i++)
		{
			//use the d3dxmatrixtranslation to generate matword is right
			//if use the identity then multiply ,will get the wrong result
			D3DXMatrixTranslation(&matWorld,m_pieceParticle[i].x,m_pieceParticle[i].y,m_pieceParticle[i].z);   
			matWorld=matBillboard*matWorld;   //set the billboard to world
			m_pd3dDevice->SetTransform(D3DTS_WORLD,&matWorld);

			//render
			m_pd3dDevice->SetTexture(0,m_pTexture);
			m_pd3dDevice->SetStreamSource(0,m_pVertexBuffer,0,sizeof(PIECEVERTEX));
			m_pd3dDevice->SetFVF(D3DFVF_PIECEVERTEX);
			m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);

		}
		renderCount++;
		if(renderCount>m_particleNumber)
			beginningFlag=1;
	}
	else
	for(int i=0;i<m_particleNumber;i++)
	{
		D3DXMatrixTranslation(&matWorld,m_pieceParticle[i].x,m_pieceParticle[i].y,m_pieceParticle[i].z);
		matWorld=matBillboard*matWorld;    //set the billboard to world
		m_pd3dDevice->SetTransform(D3DTS_WORLD,&matWorld);
		//render
		m_pd3dDevice->SetTexture(0,m_pTexture);
		m_pd3dDevice->SetStreamSource(0,m_pVertexBuffer,0,sizeof(PIECEVERTEX));
		m_pd3dDevice->SetFVF(D3DFVF_PIECEVERTEX);
		m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);
	}
	//恢复相关渲染状态：Alpha混合 、剔除状态、光照
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, true );
	
}
void PieceParticle::IsResetDevice()
{
	isReset=true;
}
PieceParticle::~PieceParticle(void)
{
	delete [] m_pieceParticle;
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pTexture);
}
