//--------------------------------------------------------------------------------------
// File: dxut1test.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTguiIME.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmesh.h"
#include "SDKmisc.h"
#include "DXUTcamera.h"
#include "MeshLoader.h"
#include "resource.h"
#include "SphereParticle.h"
#include "PieceParticle.h"
// includes, cuda
#include <cuda_runtime_api.h>
#include <cuda_d3d9_interop.h>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*                  g_pFont = NULL;         // Font for drawing text
ID3DXSprite*                g_pTextSprite = NULL;   // Sprite for batching draw text calls
ID3DXEffect*                g_pEffect = NULL;       // D3DX effect interface
CDXUTXFileMesh              g_Mesh;                 // Background mesh
D3DXMATRIXA16               g_mView;
CModelViewerCamera          g_Camera;               // A model viewing camera
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                  // dialog for standard controls
CDXUTDialog                 g_SampleUI;             // dialog for sample specific controls
CMeshLoader                 g_MeshLoader;            // Loads a mesh from an .obj file
SphereParticle                *g_SphereParticle=NULL;   //the sphere particle pointer
PieceParticle                  *g_PieceParticle=NULL;  // the piece particle pointer

LPDIRECT3DVERTEXBUFFER9    g_pGroundVB;             //地面顶点缓冲区
LPDIRECT3DTEXTURE9         g_pGroundTex;            //地面纹理
D3DXMATRIX                 g_matGround;             //地面世界矩阵

//cuda para
IDirect3DVertexBuffer9 *g_pVB         = NULL;  // Buffer to hold vertices
struct cudaGraphicsResource *cuda_VB_resource; // handles D3D9-CUDA exchange
const unsigned int g_MeshWidth    = 256;
const unsigned int g_MeshHeight   = 256;

const unsigned int g_NumVertices  = g_MeshWidth * g_MeshHeight;
float anim=0.0f;   //the time increment

int mode=0;    //the render mode ,set default to 0, ie. the sphere
int groundMode=0;  //the ground mode,set default to 0,ie.the ground  
int renderState=-1;  //start,pause,stop
float sliderValue=0.99;   //default is the medium speed
#define START 0
#define PAUSE 1
#define STOP   2

//-----------------------------------------------------------------------------
// Desc: 顶点结构
//-----------------------------------------------------------------------------
struct  CUSTOMVERTEX
{
	float x, y, z;    //顶点位置
	float u,v ;		  //顶点纹理坐标
};
#define D3DFVF_CUSTOMVERTEX   (D3DFVF_XYZ|D3DFVF_TEX1)

// A structure for our custom vertex type
struct CUSTOMCUDAVERTEX
{
	FLOAT x, y, z;      // The untransformed, 3D position for the vertex
	DWORD color;        // The vertex color
};
// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMCUDAVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)
//-----------------------------------------------------------------------------
// Control ID
//-----------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_STATIC_MUSIC              4    //music label
#define IDC_OUTPUT              5    //标签控件, 显示用户操作说明
#define IDC_RADIO1A             6    //第一组单选按钮
#define IDC_RADIO1B             7
#define IDC_RADIO1C             8
#define IDC_RADIO2A             9    //第二组单选按钮
#define IDC_RADIO2B             10
#define IDC_RADIO2C             11
#define IDC_SLIDER              12   //滑竿  adjust the particle move speed
#define IDC_LISTBOX             13   //单选列表框
#define IDC_LISTBOXM            14   //多选列表框
#define IDC_COMBOBOX            15   //组合框
#define IDC_CHECKBOX            16   //复选框1
#define IDC_CLEAREDIT           17   //复选框2
#define IDC_BTN_START           18   //start the particle rendering
#define IDC_BTN_STOP          19   //stop the particle rendering
#define IDC_BTN_PAUSE        20    //pause the rendering
#define IDC_BTN_RESET_CAMERA 21 //reset the camera
#define IDC_COMBOBOX1     22 //tranfer the ground and the CUDA wave
//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed,
	void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );

void    InitApp();
void RenderSubset( UINT iSubset, IDirect3DDevice9* pd3dDevice);  //render the obj file
void    RenderText();   //render the screen fonts

HRESULT InitVertexBuffer(IDirect3DDevice9 *pd3dDevice);
void runCuda(IDirect3DDevice9 *pd3dDevice);  //cuda function
HRESULT FreeVertexBuffer();

//import the cuda function
extern "C"
	void D3DKernel(float4 *pos,unsigned int width,unsigned int height,float time );
//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Set DXUT callbacks
	DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );
	DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
	DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
	DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
	DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( KeyboardProc );
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	// TODO: Perform any application-level initialization here
	

	InitApp();
	// Initialize DXUT and create the desired Win32 window and Direct3D device for the application
	DXUTInit( true, true ); // Parse the command line and show msgboxes
	DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"CUDA particle" );
	DXUTCreateDevice( true, 800, 600 );

	PlaySound(L"media\\雅尼 - 兰花.wav", NULL, SND_FILENAME | SND_ASYNC|SND_LOOP);   //background music,put here to play after window constructed
	// Start the render loop
	DXUTMainLoop();

	// TODO: Perform any application-level cleanup here

	return DXUTGetExitCode();
}
//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	// Initialize dialogs
	g_SettingsDlg.Init( &g_DialogResourceManager );
	g_HUD.Init( &g_DialogResourceManager );
	g_SampleUI.Init( &g_DialogResourceManager );

	g_HUD.SetCallback( OnGUIEvent );
	int iY = 10;
	g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY, 125, 22 );
	g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 35, iY += 24, 125, 22 );
	g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );

	g_SampleUI.SetCallback(OnGUIEvent);
	CDXUTComboBox *pCombo1;
	g_SampleUI.AddComboBox(IDC_COMBOBOX1,0,70,150,24,NULL,false,&pCombo1);
	if(pCombo1)
	{
		pCombo1->SetDropHeight(40);
		pCombo1->AddItem(L"ground",(LPVOID)0);
		pCombo1->AddItem(L"sine wave",(LPVOID)1);
	}

	CDXUTComboBox *pCombo;
	g_SampleUI.AddComboBox(IDC_COMBOBOX,0,100,150,24,NULL,false,&pCombo);
	if(pCombo)
	{
		pCombo->SetDropHeight(50);
		pCombo->AddItem( L"Serial render", ( LPVOID )0);
		pCombo->AddItem( L"Parallel render 1", ( LPVOID )1 );
		pCombo->AddItem( L"parallel render 2", ( LPVOID )2 );

	}

	//CDXUTButton *pBtnStart,*pBtnStop;
	g_SampleUI.AddButton(IDC_BTN_RESET_CAMERA,L"reset camera (HK 1)",0,150,100,35,L'1');
	g_SampleUI.AddButton(IDC_BTN_START,L"particle start (HK 1)",0,250,100,35,L'2');
	g_SampleUI.AddButton(IDC_BTN_PAUSE,L"particle pause (HK 2)",30,300,100,35,L'3');
	g_SampleUI.AddButton(IDC_BTN_STOP,L"particle stop (HK 3)",60,350,100,35,L'4');
	g_SampleUI.AddSlider(IDC_SLIDER,0,400,200,24);    //default the min=0,max=100

	


}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	WCHAR wssOutput[1024];
	switch(nControlID)
	{
	case IDC_TOGGLEFULLSCREEN:
		DXUTToggleFullScreen();
		break;
	case IDC_TOGGLEREF:
		DXUTToggleREF();
		break;
	case IDC_CHANGEDEVICE:
		g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive());
		break;
	case IDC_COMBOBOX:
		{
			DXUTComboBoxItem* pItem = ( ( CDXUTComboBox* )pControl )->GetSelectedItem();
			if( pItem )
			{
				mode=(int)pItem->pData;
			}
		}
		break;
	case IDC_COMBOBOX1:
		{
			DXUTComboBoxItem* pItem = ( ( CDXUTComboBox* )pControl )->GetSelectedItem();
			if( pItem )
			{
				groundMode=(int)pItem->pData;
			}
		}
		break;
	case IDC_SLIDER:
		sliderValue=0.98+(float)(( CDXUTSlider* )pControl )->GetValue()/5000;
		break;
	case IDC_BTN_START:
		renderState=START;
		break;
	case IDC_BTN_PAUSE:
		renderState=PAUSE;
		break;
	case IDC_BTN_STOP:
		renderState=STOP;
		break;
	case IDC_BTN_RESET_CAMERA:
		// reset the camera's view parameters
		D3DXVECTOR3 vecEye( 0.0f, 1.0f, -20.0f );
		D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
		g_Camera.SetViewParams( &vecEye, &vecAt );
		break;
	}
}
//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
	bool bWindowed, void* pUserContext )
{
	// Typically want to skip back buffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

	HRESULT hr;
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	D3DCAPS9 caps;

	V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
		pDeviceSettings->d3d9.DeviceType,
		&caps ) );

	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
	// then switch to SWVP.
	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
			DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext )
{
	 HRESULT hr;
	 V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
	 V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );
	// Initialize the font
	V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Arial", &g_pFont ) );

	// Create the mesh and load it with data already gathered from a file
	V_RETURN( g_MeshLoader.Create( pd3dDevice, L"media\\feitingObj.obj" ) );
	for( UINT i = 0; i < g_MeshLoader.GetNumMaterials(); i++ )
	{
		Material* pMaterial = g_MeshLoader.GetMaterial( i );
		//pComboBox->AddItem( pMaterial->strName, ( void* )( INT_PTR )i );
	}
	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye( 0.0f, 1.0f, -20.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
	g_Camera.SetViewParams( &vecEye, &vecAt );
	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );

	//create the sphere particle
	g_SphereParticle=new SphereParticle(pd3dDevice,(long)30000);
	g_SphereParticle->InitSphereParticle(0.15,15,15);

	//create the piece particle
	g_PieceParticle=new PieceParticle(pd3dDevice,(long)30000);
	g_PieceParticle->InitPieceParticle(0.4,L"media\\texture2.png");

	//创建地面顶点缓冲区
	V_RETURN( pd3dDevice->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_MANAGED, &g_pGroundVB, NULL ));


	//CUSTOMVERTEX vertices[] =
	//{
	//	{ -32.0f,  0.0f, -32.0f,  0.0f,  16.0f, },
	//	{ -32.0f,  0.0f,  32.0f,  0.0f,   0.0f, },
	//	{  32.0f,  0.0f, -32.0f, 16.0f,  16.0f, }, 
	//	{  32.0f,  0.0f,  32.0f, 16.0f,   0.0f, }
	//};
	CUSTOMVERTEX vertices[] =
	{
		{ -100.0f,  0.0f, -100.0f,  0.0f,  16.0f, },
		{ -100.0f,  0.0f,  100.0f,  0.0f,   0.0f, },
		{  100.0f,  0.0f, -100.0f, 16.0f,  16.0f, }, 
		{  100.0f,  0.0f,  100.0f, 16.0f,   0.0f, }
	};

	VOID* pVertices;
	if( FAILED( g_pGroundVB->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )
		return E_FAIL;
	memcpy( pVertices, vertices, sizeof(vertices) );
	g_pGroundVB->Unlock();

	//创建地面纹理
	V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, L"media\\ground.jpg", &g_pGroundTex ));

	//初始化地面世界矩阵
	D3DXMatrixIdentity(&g_matGround);

	//init the CUDA mesh
    InitVertexBuffer(pd3dDevice);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InitVertexBuffer()
// Desc: Creates the scene geometry (Vertex Buffer)
//-----------------------------------------------------------------------------
HRESULT InitVertexBuffer(IDirect3DDevice9 *pd3dDevice)
{
	// Create vertex buffer
	if (FAILED(pd3dDevice->CreateVertexBuffer(g_NumVertices * sizeof(CUSTOMCUDAVERTEX),
		0, D3DFVF_CUSTOMCUDAVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	// Initialize interoperability between CUDA and Direct3D9
	// Register vertex buffer with CUDA
	cudaGraphicsD3D9RegisterResource(&cuda_VB_resource, g_pVB, cudaD3D9RegisterFlagsNone);
	return S_OK;
}
////////////////////////////////////////////////////////////////////////////////
//! Run the Cuda part of the computation
////////////////////////////////////////////////////////////////////////////////
void runCuda(IDirect3DDevice9 *pd3dDevice)
{
	HRESULT hr = S_OK;

	// Map vertex buffer to Cuda
	float4 *d_ptr;

	// CUDA Map call to the Vertex Buffer and return a pointer
    cudaGraphicsMapResources(1, &cuda_VB_resource, 0);
	// This gets a pointer from the Vertex Buffer
	size_t num_bytes;
	cudaGraphicsResourceGetMappedPointer((void **)&d_ptr, &num_bytes, cuda_VB_resource);

	// Execute kernel
	D3DKernel(d_ptr, g_MeshWidth, g_MeshHeight, anim);

	// CUDA Map Unmap vertex buffer
	cudaGraphicsUnmapResources(1, &cuda_VB_resource, 0);
}

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext )
{
	HRESULT hr;

	V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
	V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

	if( g_pFont )
		V_RETURN( g_pFont->OnResetDevice() );
	// Create a sprite to help batch calls when drawing many lines of text
	V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

	g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
	g_HUD.SetSize( 170, 170 );

	g_SampleUI.SetLocation( 0, pBackBufferSurfaceDesc->Height-500 );
	g_SampleUI.SetSize( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );


	//设置纹理状态
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	//设置环境光
	pd3dDevice->SetRenderState( D3DRS_AMBIENT,  0xff808080 );
	
	//indicate the device bas been reseted
	g_SphereParticle->IsResetDevice();

	//reset cuda,some problem happened
	//InitVertexBuffer(pd3dDevice);

	// if the window size changed,reset the camera's projection parameters 
	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 1000.0f );
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// Update the camera's position based on user input 
	g_Camera.FrameMove( fElapsedTime );
	if(renderState!=PAUSE)
	{
		g_SphereParticle->UpdateSphereParticle(sliderValue);
		g_PieceParticle->UpdatePieceParticle(sliderValue);
	}

}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.OnRender( fElapsedTime );
		return;
	}
	HRESULT hr;


	// Clear the render target and the zbuffer 
	V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 141, 153, 191 ), 1.0f, 0 ) );

	// Render the scene
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{

		//设置世界矩阵
		D3DXMATRIXA16 mWorld;
		mWorld = *g_Camera.GetWorldMatrix();
		pd3dDevice->SetTransform( D3DTS_WORLD, &mWorld );

		//设置观察矩阵
		D3DXMATRIXA16 mView; 
		mView = *g_Camera.GetViewMatrix();
		pd3dDevice->SetTransform( D3DTS_VIEW, &mView );

		//设置投影矩阵
		D3DXMATRIXA16 mProj; 
		mProj = *g_Camera.GetProjMatrix();
		pd3dDevice->SetTransform( D3DTS_PROJECTION, &mProj );

		switch(groundMode)
		{
		case 1:
			//render the CUDA scene,remember to turn off the light,otherwise the color will be the sphere's color
			runCuda(pd3dDevice);
			anim+=0.01;  //the increment for the mesh
			// Render the vertex buffer contents
			pd3dDevice->SetRenderState(D3DRS_LIGHTING,false);
			//pd3dDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);    //has no effect
			pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMCUDAVERTEX));
			pd3dDevice->SetFVF(D3DFVF_CUSTOMCUDAVERTEX);
			pd3dDevice->DrawPrimitive(D3DPT_POINTLIST, 0, g_NumVertices);   //trianglelist    pointlist  trianglestrip    any difference
			pd3dDevice->SetRenderState(D3DRS_LIGHTING,true);
			break;
		case 0:
			//remember to render the ground first ,otherwise the alpha png will not be right
			//渲染地面   
			pd3dDevice->SetTransform( D3DTS_WORLD, &g_matGround );
			pd3dDevice->SetTexture( 0, g_pGroundTex );
			pd3dDevice->SetStreamSource( 0, g_pGroundVB, 0, sizeof(CUSTOMVERTEX) );
			pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
			pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
			break;
		}
		
		


		//render the particles
		//decide whether it is allowed to render
		if(renderState!=STOP)
		switch(mode)
		{
		case 0:
			////draw the sphere particle
			pd3dDevice->SetTexture(0,NULL);    //do not let the ground texture affect the particles;
			g_SphereParticle->RenderSphereParticle();
			break;
		case 1:
			//draw the piece particle
			//取得当前的取景变换矩阵
			D3DXMATRIX matView;  
			matView=*(g_Camera.GetViewMatrix());
			g_PieceParticle->RenderPieceParticle(matView);
			break;
		}
		


		RenderText();
		V( g_HUD.OnRender( fElapsedTime ) );
		V(g_SampleUI.OnRender(fElapsedTime));
		V( pd3dDevice->EndScene() );
	}
}
//-----------------------------------------------------------------------------
// Desc: 辅助函数, 渲染指定的子网格模型
//-----------------------------------------------------------------------------
void RenderSubset( UINT iSubset, IDirect3DDevice9* pd3dDevice)
{

	//获取当前子网格模型的材质和纹理
	ID3DXMesh* pMesh = g_MeshLoader.GetMesh();
	Material* pMaterial = g_MeshLoader.GetMaterial( iSubset );

	//为当前子网格模型设置材质
	static D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	mtrl.Ambient.r = pMaterial->vAmbient.x;
	mtrl.Ambient.g = pMaterial->vAmbient.x;
	mtrl.Ambient.b = pMaterial->vAmbient.x;
	mtrl.Ambient.a = pMaterial->fAlpha;
	mtrl.Diffuse.r = pMaterial->vDiffuse.x;
	mtrl.Diffuse.g = pMaterial->vDiffuse.y;
	mtrl.Diffuse.b = pMaterial->vDiffuse.z;
	mtrl.Diffuse.a = pMaterial->fAlpha;
	mtrl.Specular.r = pMaterial->vSpecular.x;
	mtrl.Specular.g = pMaterial->vSpecular.y;
	mtrl.Specular.b = pMaterial->vSpecular.z;
	mtrl.Specular.a = pMaterial->fAlpha;
	pd3dDevice->SetMaterial( &mtrl );
	
	//为当前子网格模型设置纹理
	pd3dDevice->SetTexture( 0, pMaterial->pTexture );

	//渲染当前子网格模型
	pMesh->DrawSubset( iSubset );
}
//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing, void* pUserContext )
{
	//the message is handled according the order as follows

	//main msg
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	//then the settingDlg
	if( g_SettingsDlg.IsActive() )
	{
		g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
		return 0;
	}

	// then the user customized dlgs
	*pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;
	*pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	//the camera msg
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	return 0;
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText()
{
	RECT rcWin;
	static int musicX=30.0f;
	//HWND hwnd=DXUTGetHWND();
	//GetWindowRect(DXUTGetHWND(),&rcWin);    //!!!this is WRONG  ,I want the client area ,not the window area
	GetClientRect(DXUTGetHWND(),&rcWin);
	CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

	// Output statistics
	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 5 );
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	txtHelper.DrawTextLine( DXUTGetFrameStats(true) );
	txtHelper.DrawTextLine( DXUTGetDeviceStats() );
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
	txtHelper.DrawTextLine( L"Press ESC to quit" );
	txtHelper.End();

	CDXUTTextHelper txtHelper1(g_pFont,g_pTextSprite,15);
	txtHelper1.Begin();
	txtHelper1.SetInsertionPos(5,75);
	txtHelper1.SetForegroundColor(D3DXCOLOR(0.0f,1.0f,0.0f,1.0f));
	txtHelper1.DrawTextLine(L"this is a CUDA demo");
	txtHelper1.End();

	CDXUTTextHelper txtMusic(g_pFont,g_pTextSprite,15);
	txtMusic.Begin();
	if(musicX>rcWin.bottom-50)
		musicX=30.0f;
	txtMusic.SetInsertionPos(musicX,rcWin.bottom-30);
	txtMusic.SetForegroundColor(D3DXCOLOR(0.2f,1.0f,0.3f,1.0f));
	txtMusic.DrawTextLine(L"Music playing: Yani---With An Orchid");
	musicX+=1.0f;
	txtMusic.End();

}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
	g_DialogResourceManager.OnD3D9LostDevice();
	g_SettingsDlg.OnD3D9LostDevice();
	if( g_pFont )
		g_pFont->OnLostDevice();
	SAFE_RELEASE( g_pTextSprite );
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
	g_DialogResourceManager.OnD3D9DestroyDevice();
	g_SettingsDlg.OnD3D9DestroyDevice();
	SAFE_RELEASE( g_pFont );
   g_MeshLoader.Destroy();  //clear the mesh object
   delete g_SphereParticle;  //remember to destroy the sphere particle
   delete g_PieceParticle;   //destroy piece particle
   if(g_pGroundVB)
	   SAFE_RELEASE(g_pGroundVB);
   if(g_pGroundTex)
	   SAFE_RELEASE(g_pGroundTex);

   FreeVertexBuffer(); //clear the CUDA resource
}

//-----------------------------------------------------------------------------
// Name: FreeVertexBuffer()
// Desc: Free's the Vertex Buffer resource
//-----------------------------------------------------------------------------
HRESULT FreeVertexBuffer()
{
	if (g_pVB != NULL)
	{
		// Unregister vertex buffer
		cudaGraphicsUnregisterResource(cuda_VB_resource);

		g_pVB->Release();
	}

	return S_OK;
}



