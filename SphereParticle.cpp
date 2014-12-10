#include "SphereParticle.h"
SphereParticle::SphereParticle(IDirect3DDevice9 *pd3dDevice,long particleNumber)
{
	//difinite the parameters
	m_pd3dDevice=pd3dDevice;
	m_particleNumber=particleNumber;
	m_pSphere=NULL;
	isReset=false;
	
}
void SphereParticle::InitSphereParticle(float radius,UINT slices,UINT stacks)
{
	D3DXCreateSphere(m_pd3dDevice,radius,slices,stacks,&m_pSphere,NULL);
	m_SphereParticle=new Sphere[m_particleNumber];
	srand(time(0));  //watch out for the srand place
	for(int i=0;i<m_particleNumber;i++)
	{

		m_SphereParticle[i].x=0.0f;
		m_SphereParticle[i].y=0.0f;
		m_SphereParticle[i].z=0.0f;
		//m_SphereParticle[i].vx=float(7+rand()%4)/1000;   //remember to convert to float
		//m_SphereParticle[i].vy=float(3+rand()%4)/1000;
		//m_SphereParticle[i].vz=float(-4+rand()%9)/1000;
		m_SphereParticle[i].vx=0.04f;
		m_SphereParticle[i].vy=0.02f;
		m_SphereParticle[i].vz=float(-400+rand()%900)/500000;
		m_SphereParticle[i].xrand=0;
		m_SphereParticle[i].yrand=0;
		m_SphereParticle[i].zrand=0;
	}

	//设置灯光
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type       = D3DLIGHT_DIRECTIONAL;
	//light.Diffuse.r   = 1.0f;
	//light.Diffuse.g   = 1.0f;
	//light.Diffuse.b   = 1.0f;
	//light.Diffuse.a   = 1.0f;
	//light.Specular.r  = 1.0f;
	//light.Specular.g  = 1.0f;
	//light.Specular.b  = 1.0f;
	//light.Specular.a  = 1.0f;
	light.Ambient       = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);  
	light.Diffuse       = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
	light.Specular      = D3DXCOLOR(0.9f, 0.9f, 0.9f, 1.0f);  
	light.Direction     = D3DXVECTOR3(1.0f, 0.5f, 0.0f);  

	//vecDir = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	//D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	//light.Range       = 1000.0f;
	m_pd3dDevice->SetLight( 0, &light );
	m_pd3dDevice->LightEnable( 0, true );
	//pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS,TRUE);
	//设置环境光
	m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00808080);
	//pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE,TRUE);  //open the mirror reflect
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);   //开启背面消隐
	//enable the light ,default setting
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING,true);

	// 设置材质
	D3DMATERIAL9 mtrl;
	::ZeroMemory(&mtrl, sizeof(mtrl));
	mtrl.Ambient  = D3DXCOLOR(0.5f, 0.5f, 0.7f, 1.0f);
	mtrl.Diffuse  = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
	mtrl.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
	mtrl.Emissive = D3DXCOLOR(0.3f, 0.0f, 0.1f, 1.0f);
	m_pd3dDevice->SetMaterial(&mtrl);
}
void SphereParticle::UpdateSphereParticle(float speedLevel)
{
	if(isReset)     //reset the light and mtl if device reset
	{
		//设置灯光
		D3DXVECTOR3 vecDir;
		D3DLIGHT9 light;
		ZeroMemory( &light, sizeof(D3DLIGHT9) );
		light.Type       = D3DLIGHT_DIRECTIONAL;
		//light.Diffuse.r   = 1.0f;
		//light.Diffuse.g   = 1.0f;
		//light.Diffuse.b   = 1.0f;
		//light.Diffuse.a   = 1.0f;
		//light.Specular.r  = 1.0f;
		//light.Specular.g  = 1.0f;
		//light.Specular.b  = 1.0f;
		//light.Specular.a  = 1.0f;
		light.Ambient       = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);  
		light.Diffuse       = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);  
		light.Specular      = D3DXCOLOR(0.9f, 0.9f, 0.9f, 1.0f);  
		light.Direction     = D3DXVECTOR3(1.0f, 0.5f, 0.0f);  

		//vecDir = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		//D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
		//light.Range       = 1000.0f;
		m_pd3dDevice->SetLight( 0, &light );
		m_pd3dDevice->LightEnable( 0, true );
		//pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS,TRUE);
		//设置环境光
		m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0x00808080);
		//pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE,TRUE);  //open the mirror reflect
		m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);   //开启背面消隐
		//enable the light ,default setting
		m_pd3dDevice->SetRenderState(D3DRS_LIGHTING,true);

		// 设置材质
		D3DMATERIAL9 mtrl;
		::ZeroMemory(&mtrl, sizeof(mtrl));
		mtrl.Ambient  = D3DXCOLOR(0.5f, 0.5f, 0.7f, 1.0f);
		mtrl.Diffuse  = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
		mtrl.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
		mtrl.Emissive = D3DXCOLOR(0.3f, 0.0f, 0.1f, 1.0f);
		m_pd3dDevice->SetMaterial(&mtrl);

		isReset=false;
	}
	//actually the move speed depends on the FPS value sometimes
		static int beginningFlag=0;  //the first render,don't want the particle be generated together
		static int renderCount=1;//count the render sequence number
		static float speedMax=0.3;
		static float speedMin=0.0001;
		//speedMax*=speedLevel;
		//speedMin*=speedLevel;
		srand(time(0));
		if(!beginningFlag)
		{
			for(int i=0;i<renderCount;i++)
			{
				m_SphereParticle[i].x+=m_SphereParticle[i].vx;
				m_SphereParticle[i].y+=m_SphereParticle[i].vy;
				m_SphereParticle[i].z+=m_SphereParticle[i].vz;
				if(fabs(m_SphereParticle[i].vx)<speedMax&&fabs(m_SphereParticle[i].vx)>speedMin
					&&fabs(m_SphereParticle[i].vy)<speedMax&&fabs(m_SphereParticle[i].vy)>speedMin
					&&fabs(m_SphereParticle[i].vz)<speedMax&&fabs(m_SphereParticle[i].vz)>speedMin
					&&m_SphereParticle[i].y>0&&m_SphereParticle[i].x>0)
				{
					m_SphereParticle[i].vx=m_SphereParticle[i].vx+float(rand()%50-25)/30000;
					m_SphereParticle[i].vy=m_SphereParticle[i].vy+float(rand()%50-25)/30000;
					m_SphereParticle[i].vz=m_SphereParticle[i].vz+float(rand()%50-25)/30000;
				}
				else
				{
					m_SphereParticle[i].x=m_SphereParticle[i].y=m_SphereParticle[i].z=0.0f;
					m_SphereParticle[i].vx=0.04f;
					m_SphereParticle[i].vy=0.02f;
					m_SphereParticle[i].vz=float(-400+rand()%900)/500000;
				}


			}
			renderCount++;
			if(renderCount>m_particleNumber)
				beginningFlag=1;
		}
		else
		for(int i=0;i<m_particleNumber;i++)
		{
			m_SphereParticle[i].x+=m_SphereParticle[i].vx;
			m_SphereParticle[i].y+=m_SphereParticle[i].vy;
			m_SphereParticle[i].z+=m_SphereParticle[i].vz;
			if(fabs(m_SphereParticle[i].vx)<speedMax&&fabs(m_SphereParticle[i].vx)>speedMin
				&&fabs(m_SphereParticle[i].vy)<speedMax&&fabs(m_SphereParticle[i].vy)>speedMin
				&&fabs(m_SphereParticle[i].vz)<speedMax*speedLevel&&fabs(m_SphereParticle[i].vz)>speedMin
				&&m_SphereParticle[i].y>0&&m_SphereParticle[i].x>0)
			{
				m_SphereParticle[i].vx=m_SphereParticle[i].vx+float(rand()%50-25)/30000;
				m_SphereParticle[i].vy=m_SphereParticle[i].vy+float(rand()%50-25)/30000;
				m_SphereParticle[i].vz=m_SphereParticle[i].vz+float(rand()%50-25)/30000;
			}
			else
			{
				m_SphereParticle[i].x=m_SphereParticle[i].y=m_SphereParticle[i].z=0.0f;
				m_SphereParticle[i].vx=0.04f;
				m_SphereParticle[i].vy=0.02f;
				m_SphereParticle[i].vz=float(-400+rand()%900)/500000;
			}
		}
		for(int i=0;i<m_particleNumber;i++)
		{
			m_SphereParticle[i].vx*=speedLevel;
			m_SphereParticle[i].vy*=speedLevel;
			m_SphereParticle[i].vz*=speedLevel;
		}

}
void SphereParticle::RenderSphereParticle()
{
	static int beginningFlag=0;  //the first render,don't want the particle be generated together
	static int renderCount=1;//count the render sequence number
	//set matrix
	static D3DXMATRIX matWorld;
	//this strategy is to let the particles not generate at the same time
    if(!beginningFlag)
	{
		for(int i=0;i<renderCount;i++)
		{
			D3DXMatrixTranslation(&matWorld,m_SphereParticle[i].x,m_SphereParticle[i].y,m_SphereParticle[i].z);
			m_pd3dDevice->SetTransform(D3DTS_WORLD,&matWorld);
			//render
			m_pSphere->DrawSubset(0);
		}
		renderCount++;
		if(renderCount>m_particleNumber)
			beginningFlag=1;
	}
	else
	for(int i=0;i<m_particleNumber;i++)
	{

		D3DXMatrixTranslation(&matWorld,m_SphereParticle[i].x,m_SphereParticle[i].y,m_SphereParticle[i].z);
		m_pd3dDevice->SetTransform(D3DTS_WORLD,&matWorld);
		//render
		m_pSphere->DrawSubset(0);
	}



}

void SphereParticle::IsResetDevice()
{
	isReset=true;
}
SphereParticle::~SphereParticle(void)
{
	delete [] m_SphereParticle;
	SAFE_RELEASE(m_pSphere);
}
