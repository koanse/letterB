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
ID3DXMesh	      *MeshTrap;
ID3DXMesh	      *MeshRect;



const float epsilon = 0.0001f;

const float anglePlusX = D3DX_PI/100;
const float anglePlusY = D3DX_PI/130;
const float anglePlusZ = D3DX_PI/180;

float rotX = 0, rotY = 0, rotZ = 0, A = 0.8;

struct Vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 n;
};

#define CUSTOM_FVF (D3DFVF_XYZ | D3DFVF_NORMAL)

const int nGridL = 64;
const int nGridX = 8;
const int numVertices = nGridL * (nGridX + 2) * 6 + 12;

const float l1_v=2;		// длина одной палки
const float l2_v=2;		// длина другой палки
const float r=0.6f;		// толщина

// количество треугольников в основании трапеции
const int nGridTrap = 8;

const int nGridRectA = 4;
const int nGridRectB = 8;
const int numVerticesTrap = nGridTrap * nGridTrap * 3 / 4 * 3;
const int numVerticesRect = nGridRectA * nGridRectB * 6;

// длина "распр€мленной буквы"
const float LEN = l1_v + l2_v;

void FillMashY();

void DrawBlock (D3DXMATRIX general_tr,D3DXMATRIX general_rt);

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
D3DXVECTOR3 upper_function ( float l, float x, D3DXVECTOR3 *p);
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

	light1.Position = D3DXVECTOR3( 4, 0, 0.0f );
	
	light1.Range = 10;
	light1.Attenuation0 = 0.0f;
	light1.Attenuation1 = 0.0f;
	light1.Attenuation2 = 0.0f;

	D3DLIGHT9 light2;		
	ZeroMemory(&light2, sizeof(light2));
	
	light2.Type = D3DLIGHT_SPOT;

	light2.Diffuse.r = 0.0f;
	light2.Diffuse.g = 0.4f;
	light2.Diffuse.b = 0.4f;

	light2.Position = D3DXVECTOR3( 0.9, 0, 2.5 );	
	light2.Direction = D3DXVECTOR3( 0.0f, 0, 0.0f );

	light2.Range = 10;	
	light2.Falloff = 1.0f;
	light2.Attenuation0 = 0.2f;
	light2.Attenuation1 = 0.2f;
	light2.Attenuation2 = 0.2f;
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

	D3DXVECTOR3 eye( 29, 0, 0 );
	D3DXVECTOR3 at ( 0, 0, 0 );
	D3DXVECTOR3 up ( 0, 0, 1 );    
	D3DXMATRIX  matView;	
	D3DXMatrixLookAtLH( &matView, &eye, &at, &up );   

	D3DXMATRIX matProj;			
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/12, 1, 1, 100 );

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
	static float B;

	if(A <= 0.8) B = 0.01;
	if(A >= 1.2f) B = -0.01;
	A += B;

	DrawWindowArea(hwnd);
	//FillMesh();

	

	rotX += anglePlusX;
	
	if( rotX > 2*D3DX_PI )
		rotX -= 2*D3DX_PI;

	rotY += anglePlusY;
	
	if( rotY > 2*D3DX_PI )
		rotY -= 2*D3DX_PI;

	rotZ += anglePlusZ;
	
	if( rotZ > 2*D3DX_PI )
		rotZ -= 2*D3DX_PI;
}


void center (float l, float *result)
{
    // Ќахождение координат центра текущего сечени€
	float xc, yc, zc;
		
	yc = 0;

	if( l < l1_v )
	{
		xc = l - l1_v;
		zc = -1.5f * xc;
	}

	if( l >= l1_v )
	{
		xc = l - l1_v;
		zc = 1.5f * xc;
	}

	result[0] = xc;
	result[1] = yc;
	result[2] = zc;

	return;
}

D3DXVECTOR3 upper_function ( float l, float x, D3DXVECTOR3 *p)
{
	float  y, res[3], x1, y1, z1, module_p;
	D3DXVECTOR3 result, n;
	D3DXMATRIX m;

	if(x >= 0)
	{
		y = x;
		*p = D3DXVECTOR3(-1, 1, 0);
	}
	else
	{
		y = -x;
		*p = D3DXVECTOR3(1, 1, 0);
	}

	if(x == -r)
	{
		y = -r;
		*p = D3DXVECTOR3(0, -1, 0);
	}

	if(x == r)
	{
		y = -r;
		*p = D3DXVECTOR3(0, 1, 0);
	}

    center (l, res);
	x1 = res[0]; y1 = res[1]; z1 = res[2];

	center (l + epsilon, res);
	
	// нормаль к плоскости сечени€
	n = D3DXVECTOR3(res[0] - x1, res[1] - y1, res[2] - z1);

	m = CreateTranslationToNewBasis (D3DXVECTOR3(x1, y1, z1),
									n,
									rotX+rotY+rotZ+0.7f*l);
	result = GetTranslatedCoordinates (x, y, 0, &m);
	
	*p = GetTranslatedCoordinates (p->x, p->y, p->z, &m);
	module_p = pow(p->x * p->x + p->y * p->y + p->z * p->z, 0.5f);
	p->x /= module_p; p->y /= module_p; p->z /= module_p;

	return result;
}

D3DXVECTOR3 lower_function ( float l, float x, D3DXVECTOR3 *p)
{
	float  y, res[3], x1, y1, z1, module_p;
	D3DXVECTOR3 result, n;
	D3DXMATRIX m;

	if(x >= 0)
	{
		y = x;
		*p = D3DXVECTOR3(-1, 1, 0);
	}
	else
	{
		y = -x; 
		*p = D3DXVECTOR3(1, 1, 0);
	}

	if(x == -r)
	{
		y = -r;
		*p = D3DXVECTOR3(0, -1, 0);
	}

	if(x == r)
	{
		y = -r;
		*p = D3DXVECTOR3(0, 1, 0);
	}

    center (l, res);
	x1 = res[0]; y1 = res[1]; z1 = res[2];

	center (l + epsilon, res);
	
	// нормаль к плоскости сечени€
	n = D3DXVECTOR3(res[0] - x1, res[1] - y1, res[2] - z1);

	m = CreateTranslationToNewBasis (D3DXVECTOR3(x1, y1, z1),
									n,
									rotX+rotY+rotZ+0.7f*l);
	result = GetTranslatedCoordinates (x, y, 0, &m);
	
	*p = GetTranslatedCoordinates (p->x, p->y, p->z, &m);
	module_p = pow(p->x * p->x + p->y * p->y + p->z * p->z, 0.5f);
	p->x /= module_p; p->y /= module_p; p->z /= module_p;

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
		
		p[k].pos = upper_function(l, r, &p[k].n);

		++k;

		p[k].pos = upper_function(l+stepL, -r, &p[k].n);

		++k;

		p[k].pos = upper_function(l, -r, &p[k].n);
		
		++k;
	
	///////////////////////////////////////////////////
		
		p[k].pos = upper_function(l, r, &p[k].n);
		
		++k;

		p[k].pos = upper_function(l+stepL, r, &p[k].n);

		++k;

		p[k].pos = upper_function(l+stepL, -r, &p[k].n);

		++k;
	

		for(j = 0; j < nGridX; ++j)
		{		
			x = j*stepX - r;

			p[k].pos = upper_function(l, x+stepX, &p[k].n);
						
			++k;
			
			p[k].pos = upper_function(l+stepL,x,&p[k].n);

			++k;

			p[k].pos = upper_function(l+stepL,x+stepX,&p[k].n);

			++k;

			p[k].pos = upper_function(l,x+stepX,&p[k].n);
			
			++k;

			p[k].pos = upper_function(l,x,&p[k].n);
						
			++k;

			p[k].pos = upper_function(l+stepL,x,&p[k].n);
			
			++k;
		}	
	}



	/*float tmp[3], xc, yc, zc, module_n;
	D3DXVECTOR3 n;
	///////////////////////////////////////////////////////
	center(0, tmp);
	xc = tmp[0]; yc = tmp[1]; zc = tmp[2];
	center(0+stepL, tmp);
	n.x = xc - tmp[0]; n.y = yc - tmp[1]; n.z = zc - tmp[2];
	module_n = pow (n.x * n.x + n.y * n.y + n.z * n.z, 0.5f);
	n.x /= module_n; n.y /= module_n; n.z /= module_n;

		
	p[k].pos = upper_function(0, -r, &p[k].n); ++k;
	p[k].pos = upper_function(0, -r+stepX, &p[k].n); ++k;
	p[k].pos = D3DXVECTOR3(xc, yc, zc); ++k;
	
	p[k-3].n = p[k-2].n = p[k-1].n = n;
	///////////////////////////////////////////////////
	
	p[k].pos = upper_function(0, -r, &p[k].n); ++k;
	p[k].pos = D3DXVECTOR3(xc, yc, zc); ++k;
	p[k].pos = upper_function(0, r, &p[k].n); ++k;
	
	p[k-3].n = p[k-2].n = p[k-1].n = n;
	///////////////////////////////////////////////////

	p[k].pos = upper_function(0, r, &p[k].n); ++k;
	p[k].pos = D3DXVECTOR3(xc, yc, zc); ++k;
	p[k].pos = upper_function(0, r-stepX, &p[k].n); ++k;
	
	p[k-3].n = p[k-2].n = p[k-1].n = n;
	///////////////////////////////////////////////////

	
	///////////////////////////////////////////////////////
	center(LEN, tmp);
	xc = tmp[0]; yc = tmp[1]; zc = tmp[2];
	center(LEN+stepL, tmp);
	n.x = tmp[0] - xc; n.y = tmp[1] - yc; n.z = tmp[2] - zc;
	module_n = pow (n.x * n.x + n.y * n.y + n.z * n.z, 0.5f);
	n.x /= module_n; n.y /= module_n; n.z /= module_n;

		
	p[k].pos = upper_function(LEN, -r+stepX, &p[k].n); ++k;
	p[k].pos = upper_function(LEN, -r, &p[k].n); ++k;
	p[k].pos = D3DXVECTOR3(xc, yc, zc); ++k;
	
	p[k-3].n = p[k-2].n = p[k-1].n = n;
	///////////////////////////////////////////////////
	
	p[k].pos = D3DXVECTOR3(xc, yc, zc); ++k;
	p[k].pos = upper_function(LEN, -r, &p[k].n); ++k;
	p[k].pos = upper_function(LEN, r, &p[k].n); ++k;
	
	p[k-3].n = p[k-2].n = p[k-1].n = n;
	///////////////////////////////////////////////////

	p[k].pos = D3DXVECTOR3(xc, yc, zc); ++k;
	p[k].pos = upper_function(LEN, r, &p[k].n); ++k;
	p[k].pos = upper_function(LEN, r-stepX, &p[k].n); ++k;
	
	p[k-3].n = p[k-2].n = p[k-1].n = n;
	///////////////////////////////////////////////////
	*/

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

void FillMashY()
{
	// mesh дл€ трапеции
	ID3DXMesh *MeshTmp;	
	res = D3DXCreateMeshFVF
		(
			numVerticesTrap/3,
			numVerticesTrap,
			D3DXMESH_SYSTEMMEM,
			CUSTOM_FVF,
			D3dDevice,
			&MeshTmp
		);

	if( res != D3D_OK )
	{
		D3dDevice -> Release();
		D3dObject -> Release();
		return 0;
	}

	Vertex *p;
	MeshTmp -> LockVertexBuffer(0, (void**)&p);

	int k = 0;
	float x;
	float y;

	float stepX = 2.0f / nGridTrap;
	float stepY = 2.0f / nGridTrap * sin(D3DX_PI / 3);

	for(int i = 0; i < nGridTrap / 2; ++i)
	{
		for(int j = 0; j < nGridTrap - i; ++j)
		{		
			x = j*stepX + i*stepX/2;
			y = i*stepY;

			p[k].pos = D3DXVECTOR3( x, y, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
			
			++k;

			p[k].pos = D3DXVECTOR3( x + stepX, y, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
			
			++k;

			p[k].pos = D3DXVECTOR3( x + stepX/2, y + stepY, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;
		}
		for(int j = 0; j < nGridTrap - i - 1; ++j)
		{		
			x = j*stepX + i*stepX/2 ;
			y = i*stepY;

			p[k].pos = D3DXVECTOR3( x + stepX/2, y + stepY, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;

			p[k].pos = D3DXVECTOR3( x + stepX, y, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;

			p[k].pos = D3DXVECTOR3( x + 1.5f*stepX, y + stepY, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;
		}	
	}

	MeshTmp -> UnlockVertexBuffer();

	WORD *indexBuf;
	
	MeshTmp -> LockIndexBuffer(0, (void**)&indexBuf);
	
	for(int i = 0; i < numVerticesTrap; ++i)
	{
		indexBuf[i] = i;
	}
	
	MeshTmp -> UnlockIndexBuffer();

	DWORD *adjArray = new DWORD[ numVerticesTrap*sizeof(DWORD) ];

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
				&MeshTrap
			);
	
	MeshTmp -> Release();
	delete [] adjArray;

//////////////////////////////////////////////////////////////
// mesh дл€ пр€моугольника
	res = D3DXCreateMeshFVF
		(
			numVerticesRect/3,
			numVerticesRect,
			D3DXMESH_SYSTEMMEM,
			CUSTOM_FVF,
			D3dDevice,
			&MeshTmp
		);

	if( res != D3D_OK )
	{
		D3dDevice -> Release();
		D3dObject -> Release();
		return 0;
	}

	MeshTmp -> LockVertexBuffer(0, (void**)&p);

	stepX = 1.0f / nGridRectA;
	stepY = 2.0f / nGridRectB;

	k = 0;
	for(int i = 0; i < nGridRectA; ++i)
	{
		for(int j = 0; j < nGridRectB; ++j)
		{		
			x = i*stepX;
			y = j*stepY;

			p[k].pos = D3DXVECTOR3( x, y, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
			
			++k;

			p[k].pos = D3DXVECTOR3( x + stepX, y, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
			
			++k;

			p[k].pos = D3DXVECTOR3( x + stepX, y + stepY, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;
		
			p[k].pos = D3DXVECTOR3( x + stepX, y + stepY, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;

			p[k].pos = D3DXVECTOR3( x , y + stepY, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;

			p[k].pos = D3DXVECTOR3( x, y, 0 );
			p[k].n   = D3DXVECTOR3( 0, 0, 1 );
						
			++k;
		}	
	}

	MeshTmp -> UnlockVertexBuffer();

	MeshTmp -> LockIndexBuffer(0, (void**)&indexBuf);
	
	for(int i = 0; i < numVerticesRect; ++i)
	{
		indexBuf[i] = i;
	}
	
	MeshTmp -> UnlockIndexBuffer();

	adjArray = new DWORD[ numVerticesTrap*sizeof(DWORD) ];

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
				&MeshRect
			);

///////////////////////////////////////////////////////////


	MeshTmp -> Release();
	
	delete [] adjArray;
	
	return;
}

void DrawBlock (
				D3DXMATRIX general_tr,
				D3DXMATRIX general_rt
				)
{
	D3DXMATRIX tr1, tr2, rot1, rot2, rotGen, m;
	
	D3DXMatrixRotationYawPitchRoll( &rotGen, rotY, rotX, rotZ );

	//////////////////////////////////////////////////////
	// рисование блока
	// 1-й пр€моугольник, составл€ющий заднюю часть
	D3DXMatrixRotationX( &rot1, D3DX_PI/2);
	D3DXMatrixTranslation( &tr1, 0, 0, -2 );

	m = rot1 * tr1 * general_rt * general_tr * rotGen;
	D3dDevice -> SetTransform( D3DTS_WORLD, &m );

	MeshRect -> DrawSubset(0);

	// 2-й пр€моугольник, составл€ющий заднюю часть
	D3DXMatrixRotationX( &rot1, D3DX_PI/2 );
	D3DXMatrixTranslation( &tr1, -1, 0, -2 );
	
	m = rot1 * tr1 * general_rt * general_tr * rotGen;
	D3dDevice -> SetTransform( D3DTS_WORLD, &m );

	MeshRect -> DrawSubset(0);

	// пр€моугольник, составл€ющий 1-ю боковую часть
	D3DXMatrixRotationX( &rot1, D3DX_PI/2 );
	D3DXMatrixRotationZ( &rot2, D3DX_PI*2/3 );
	D3DXMatrixTranslation( &tr1, 1, 0, -2 );
	
	m = rot1 * rot2 * tr1 * general_rt * general_tr * rotGen;
	D3dDevice -> SetTransform( D3DTS_WORLD, &m );

	MeshRect -> DrawSubset(0);

	// пр€моугольник, составл€ющий 2-ю боковую часть
	D3DXMatrixRotationX( &rot1, -D3DX_PI/2 );
	D3DXMatrixRotationZ( &rot2, D3DX_PI/3 );
	D3DXMatrixTranslation( &tr1, -1, 0, 0 );
	
	m = rot1 * rot2 * tr1 * general_rt * general_tr * rotGen;
	D3dDevice -> SetTransform( D3DTS_WORLD, &m );

	MeshRect -> DrawSubset(0);


	// пр€моугольник, составл€ющий переднюю часть
	D3DXMatrixRotationX( &rot1, -D3DX_PI/2 );
	D3DXMatrixTranslation( &tr1, -0.5, sin(D3DX_PI/3), 0 );
	
	m = rot1 * tr1 * general_rt * general_tr * rotGen;
	D3dDevice -> SetTransform( D3DTS_WORLD, &m );

	MeshRect -> DrawSubset(0);

	/////////////////////////////////////////////////////
	// трапеци€, наход€ща€с€ вверху
	D3DXMatrixTranslation( &tr1, -1, 0, 0 );
	
	m = tr1 * general_rt * general_tr * rotGen;
	D3dDevice -> SetTransform( D3DTS_WORLD, &m );

	MeshTrap -> DrawSubset(0);

	// трапеци€, наход€ща€с€ внизу
	D3DXMatrixRotationY( &rot1, D3DX_PI );
	D3DXMatrixTranslation( &tr1, 1, 0, -2 );
	
	m = rot1 * tr1 * general_rt * general_tr * rotGen;
	D3dDevice -> SetTransform( D3DTS_WORLD, &m );

	MeshTrap -> DrawSubset(0);

	return;
}



//////////////////////////////////////////////////////////////
// конец файла main.cpp
//////////////////////////////////////////////////////////////