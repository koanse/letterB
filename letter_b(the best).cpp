//////////////////////////////////////////////////////////////
// начало файла main.cpp
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <math.h>

#include <d3d9.h>
#include <d3dx9math.h>

IDirect3D9        *D3dObject;
IDirect3DDevice9  *D3dDevice; 
ID3DXMesh	      *Mesh;
IDirect3DTexture9 *D3dTexture; 

const float epsilon = 0.0001f;

const float anglePlusX = D3DX_PI/100;
const float anglePlusY = D3DX_PI/130;
const float anglePlusZ = D3DX_PI/180;

float rotX = 0, rotY = 0, rotZ = 0;

struct Vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 n;
};

#define CUSTOM_FVF (D3DFVF_XYZ | D3DFVF_NORMAL)

const int nGridL = 64;
const int nGridX = 8;
const int numVertices = ( nGridL + 6 ) * ( nGridX + 6) * 6;

const float l1=2;		// длина верхней детали тв. знака
const float l2=3;		// длина вертикальной пр€мой детали
const float R=1;		// радиус закруглени€
const float r=0.5f;		// радиус полуцилиндра

// длина "распр€мленной буквы"
const float LEN = l1 + l2 + 2*3.141592*R;


void __stdcall TimerProc
	(
		HWND hwnd,
		UINT uMsg,
		UINT_PTR idEvent,
		DWORD dwTime
	);

LRESULT __stdcall MainWindowProc
	(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
	);

void FillMesh ();
D3DXVECTOR3 GetTranslatedCoordinates (float x,
									  float y,
									  float z,
                                      D3DXMATRIX *tr );

D3DXMATRIX CreateTranslationToNewBasis ( D3DXVECTOR3 origin,
										 D3DXVECTOR3 new_Z_axis,
										 float angle);
void DrawWindowArea(HWND hwnd);
D3DXVECTOR3 rect_to_surf ( float l, float x, D3DXVECTOR3 *p);
void center (float l, float *result);

int __stdcall WinMain
	(
		HINSTANCE instance,
		HINSTANCE previousInstance,
		LPSTR commandLine,
		int commandShow
	)
{
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = instance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "mywndclass";
	wc.lpfnWndProc = MainWindowProc;

	if( !RegisterClass(&wc) )
		return 0;	

	HWND AppWnd = CreateWindow
		(
			"mywndclass",
			"d3d test",
			WS_OVERLAPPEDWINDOW,
			40, 40, 500, 500,
			NULL,
			NULL,
			instance,
			NULL
		);

	if( !AppWnd )
		return 0;

	D3dObject = Direct3DCreate9(D3D_SDK_VERSION);
	if( !D3dObject )
		return 0;

	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	HRESULT res;
	res = D3dObject -> CreateDevice
					(
						D3DADAPTER_DEFAULT,
						D3DDEVTYPE_HAL,
						AppWnd,
						D3DCREATE_HARDWARE_VERTEXPROCESSING,
						&d3dpp,
						&D3dDevice
					);

	if( res != D3D_OK )
	{
		D3dObject -> Release();
		return 0;
	}
	
	///////////////////////////////////////////////////////
	
	FillMesh();	

	D3DMATERIAL9 material;	
	ZeroMemory(&material, sizeof(material));
	
	material.Diffuse.r = 0.9f;
	material.Diffuse.g = 0.9f;
	material.Diffuse.b = 0.9f;
	material.Diffuse.a = 1.0f;

	material.Ambient.r = 0.4f;
	material.Ambient.g = 0.4f;
	material.Ambient.b = 0.4f;
	material.Ambient.a = 1.0f;

	material.Specular.r = 0.7f;
	material.Specular.g = 0.7f;
	material.Specular.b = 0.7f;
	material.Specular.a = 1.0f;

	material.Power = 5.0f;

	D3dDevice -> SetMaterial(&material);

	D3dDevice -> SetRenderState( D3DRS_SPECULARENABLE, TRUE );

	///////////////////////////////////////////////////////

	D3DLIGHT9 light1;
	ZeroMemory(&light1, sizeof(light1));

	light1.Type = D3DLIGHT_POINT;

	light1.Diffuse.r = 0.8f;
	light1.Diffuse.g = 0.7f;
	light1.Diffuse.b = 0.5f;

	light1.Specular.r = 0.8f;
	light1.Specular.g = 0.7f;
	light1.Specular.b = 0.5f;

	light1.Position = D3DXVECTOR3( 0.8f, 0, -0.8f );
	
	light1.Range = 10;
	light1.Attenuation0 = 0.1f;
	light1.Attenuation1 = 0.1f;
	light1.Attenuation2 = 0.05f;

	D3DLIGHT9 light2;		
	ZeroMemory(&light2, sizeof(light2));
	
	light2.Type = D3DLIGHT_SPOT;

	light2.Diffuse.r = 0.0f;
	light2.Diffuse.g = 0.8f;
	light2.Diffuse.b = 0.1f;

	light2.Position = D3DXVECTOR3( 0.9, 0, 2.5 );	
	light2.Direction = D3DXVECTOR3( 0.0f, 0, 0.0f );

	light2.Range = 10;	
	light2.Falloff = 1.8f;
	light2.Attenuation0 = 0.1f;
	light2.Attenuation1 = 0.3f;
	light2.Attenuation2 = 1.3f;
	light2.Phi = D3DX_PI;
	light2.Theta = D3DX_PI/2;

	D3dDevice -> SetLight(0, &light1);
	D3dDevice -> SetLight(1, &light2);

	D3dDevice -> LightEnable(0, TRUE);
	D3dDevice -> LightEnable(1, TRUE);

	D3dDevice -> SetRenderState
				( 
					D3DRS_AMBIENT, 
					D3DCOLOR_XRGB(100, 100, 100) 
				);

	///////////////////////////////////////////////////////

	D3DXVECTOR3 eye( 20, 0, 0 );
	D3DXVECTOR3 at ( 0, 0, 0 );
	D3DXVECTOR3 up ( 0, 0, 1 );    
	D3DXMATRIX  matView;	
	D3DXMatrixLookAtLH( &matView, &eye, &at, &up );   

	D3DXMATRIX matProj;			
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/6, 1, 0.3f, 100 );

	D3dDevice -> SetTransform( D3DTS_VIEW, &matView );
	D3dDevice -> SetTransform( D3DTS_PROJECTION, &matProj );
   		
	///////////////////////////////////////////////////////
	
	SetTimer(AppWnd, 1, 1000/50, TimerProc);

	ShowWindow(AppWnd, commandShow);
	UpdateWindow(AppWnd);

	MSG msg;	

	while( GetMessage(&msg, NULL, 0, 0) > 0 )
	{
		DispatchMessage(&msg);
	}

	KillTimer(AppWnd, 1);

	Mesh       -> Release();
	D3dDevice  -> Release();
	D3dObject  -> Release();

	return msg.wParam;
}

LRESULT __stdcall MainWindowProc
	(
		HWND hwnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam
	)
{

	switch(msg)
	{
		case WM_PAINT:

			DrawWindowArea(hwnd);			
			return 0;

		case WM_DESTROY:

			PostQuitMessage(0);
			return 0;			
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}	

void DrawWindowArea(HWND hwnd)
{
	D3dDevice -> Clear
				(
					0, 
					NULL, 
					D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
					D3DCOLOR_XRGB(120, 120, 160), 
					1,
					0
				);

	D3dDevice -> BeginScene();

	D3DXMATRIX tr1, tr2, rot, rotGen, m;
	
	D3DXMatrixRotationYawPitchRoll( &rotGen, rotY, rotX, rotZ );
	
	D3dDevice -> SetTransform( D3DTS_WORLD, &rotGen );

	Mesh -> DrawSubset(0);

	D3dDevice -> EndScene();

	D3dDevice -> Present(NULL, NULL, NULL, NULL);

	RECT wndRect;
	GetClientRect(hwnd, &wndRect);
	ValidateRect(hwnd, &wndRect);
}

void __stdcall TimerProc
	(
		HWND hwnd,
		UINT uMsg,
		UINT_PTR idEvent,
		DWORD dwTime
	)
{
	bool enabled = TRUE;
	DrawWindowArea(hwnd);
	FillMesh();

	rotX += anglePlusX;
	
	if( rotX > 2*D3DX_PI )
			rotX -= 2*D3DX_PI;

	rotY += anglePlusY;
	
	if( rotY > 2*D3DX_PI )
		rotY -= 2*D3DX_PI;


	rotZ += anglePlusZ;
	
	if( rotZ > 2*D3DX_PI )
		rotZ -= 2*D3DX_PI;
	
	return;
}


void center (float l, float *result)
{
    // Ќахождение координат центра текущего сечени€
	float yc, zc;
		
	if( (l >= 0) && (l < l1) )
	{
		yc = l - l1;
		zc = l2 + R;
	}

	if( (l >= l1) && (l < (l1 + l2)) )
	{
		yc = 0;
		zc = l1 + l2 - l + R;
	}

	if( l >= (l1 + l2) )
	{
		yc = R * (1 - cos(l-l1-l2));
		zc = R * (1 - sin(l-l1-l2));
	}

	result[0] = 0;
	result[1] = yc;
	result[2] = zc;

	return;
}

D3DXVECTOR3 rect_to_surf ( float l, float x, D3DXVECTOR3 *p)
{
	float  y, res[3], x1, y1, z1, module_p;
	D3DXVECTOR3 result, n;
	D3DXMATRIX m;

	y = pow ( r*r - x*x, 0.5f );

	
    center (l, res);
	x1 = res[0]; y1 = res[1]; z1 = res[2];

	center (l + epsilon, res);
	
	// нормаль к плоскости сечени€
	n = D3DXVECTOR3(res[0] - x1, res[1] - y1, res[2] - z1);

	m = CreateTranslationToNewBasis (D3DXVECTOR3(x1, y1, z1),
									n,
									rotX+rotY+rotZ+0.2f*l);
	result = GetTranslatedCoordinates (x, y, 0, &m);
	
	*p = result - D3DXVECTOR3(x1, y1, z1);
	module_p = pow(p->x * p->x + p->y * p->y + p->z * p->z, 0.5f);
	p->x = p->x / module_p;
	p->y = p->y / module_p;
	p->z = p->z / module_p;

	return result;
}


D3DXVECTOR3 GetTranslatedCoordinates (float x,
									  float y,
									  float z,
                                      D3DXMATRIX *tr )
{
    float xt, yt, zt, A;
	xt = (*tr)(0,0)*x + (*tr)(1,0)*y + (*tr)(2,0)*z + (*tr)(3,0);
	yt = (*tr)(0,1)*x + (*tr)(1,1)*y + (*tr)(2,1)*z + (*tr)(3,1);
	zt = (*tr)(0,2)*x + (*tr)(1,2)*y + (*tr)(2,2)*z + (*tr)(3,2);
	A  = (*tr)(0,3)*x + (*tr)(1,3)*y + (*tr)(2,3)*z + (*tr)(3,3);
	
	if( A != 1.0f) { xt /= A; yt /= A; zt /= A;}
	return D3DXVECTOR3(xt, yt, zt);
}

D3DXMATRIX CreateTranslationToNewBasis ( D3DXVECTOR3 origin,
										 D3DXVECTOR3 new_Z_axis,
										 float angle_rot_Z)
{
	float angle_rot_X;
	D3DXMATRIX rot1, rot2, tr;

	angle_rot_X = acos(new_Z_axis.z / 
        pow(new_Z_axis.x * new_Z_axis.x +
		new_Z_axis.y * new_Z_axis.y +
		new_Z_axis.z * new_Z_axis.z, 0.5f));

	if(new_Z_axis.y > 0) angle_rot_X *= -1;

	D3DXMatrixRotationZ (&rot1, angle_rot_Z);
	D3DXMatrixRotationX (&rot2, angle_rot_X);
	D3DXMatrixTranslation (&tr, origin.x, origin.y, origin.z);

	return rot1*rot2*tr;
}

void FillMesh ()
{
	HRESULT res;	
	ID3DXMesh *MeshTmp;	
	res = D3DXCreateMeshFVF
		(
			numVertices/3,
			numVertices,
			D3DXMESH_SYSTEMMEM,
			CUSTOM_FVF,
			D3dDevice,
			&MeshTmp
		);

	if( res != D3D_OK )
	{
		D3dDevice -> Release();
		D3dObject -> Release();
		return;
	}

	Vertex *p;
	MeshTmp -> LockVertexBuffer(0, (void**)&p);

	int i, j, k = 0;
	float stepL = LEN / nGridL;
	float stepX = 2*r / nGridX;
	float l, x;

	for(i = 0, j = 0; i < nGridL; ++i)
	{
		l = i*stepL;
		x = j*stepX - r;
		
		p[k].pos = rect_to_surf(l, r, &p[k].n);

		++k;

		p[k].pos = rect_to_surf(l+stepL, -r, &p[k].n);

		++k;

		p[k].pos = rect_to_surf(l, -r, &p[k].n);
		
		++k;
	
	///////////////////////////////////////////////////
		
		p[k].pos = rect_to_surf(l, r, &p[k].n);
		
		++k;

		p[k].pos = rect_to_surf(l+stepL, r, &p[k].n);

		++k;

		p[k].pos = rect_to_surf(l+stepL, -r, &p[k].n);

		++k;
	

		for(j = 0; j < nGridX; ++j)
		{		
			l = i*stepL;
			x = j*stepX - r;

			p[k].pos = rect_to_surf(l, x+stepX, &p[k].n);
						
			++k;
			
			p[k].pos = rect_to_surf(l+stepL,x,&p[k].n);

			++k;

			p[k].pos = rect_to_surf(l+stepL,x+stepX,&p[k].n);

			++k;

			p[k].pos = rect_to_surf(l,x+stepX,&p[k].n);
			
			++k;

			p[k].pos = rect_to_surf(l,x,&p[k].n);
						
			++k;

			p[k].pos = rect_to_surf(l+stepL,x,&p[k].n);
			
			++k;
		}	
	}

	float tmp[3], xc, yc, zc;
	center(0, tmp);
	xc = tmp[0]; yc = tmp[1]; zc = tmp[2];


	for( i = 0; i < nGridX; i++)
	{
		x = i * stepX - r;
		
		p[k].pos = rect_to_surf(0, x, &p[k].n);
				
		++k;

		p[k].pos = rect_to_surf(0, x+stepX, &p[k].n);
					
		++k;

		p[k].pos = D3DXVECTOR3(xc, yc, zc);
		p[k].n = D3DXVECTOR3(0, -1, 0);				

		++k;
	}

	MeshTmp -> UnlockVertexBuffer();

	WORD *indexBuf;
	
	MeshTmp -> LockIndexBuffer(0, (void**)&indexBuf);
	
	for(int i = 0; i < numVertices; ++i)
	{
		indexBuf[i] = i;
	}
	
	MeshTmp -> UnlockIndexBuffer();

	DWORD *adjArray = new DWORD[ numVertices * sizeof(DWORD) ];

	D3DXWELDEPSILONS eps;
	ZeroMemory( &eps, sizeof(eps) );
	
	eps.Position    = epsilon;	
	eps.Texcoord[0] = epsilon;
	eps.Texcoord[1] = epsilon;

	D3DXWeldVertices
	(
		MeshTmp, 
		0,
		&eps,
		NULL, 
		adjArray, 
		NULL, 
		NULL
	);

	MeshTmp -> OptimizeInplace
			(
				D3DXMESHOPT_COMPACT,
				adjArray,
				NULL,
				NULL,
				NULL
			);

	MeshTmp -> CloneMeshFVF
			(
				D3DXMESH_WRITEONLY,
				CUSTOM_FVF,
				D3dDevice,
				&Mesh
			);

	MeshTmp -> Release();
	
	delete [] adjArray;
	return;
}





//////////////////////////////////////////////////////////////
// конец файла main.cpp
//////////////////////////////////////////////////////////////