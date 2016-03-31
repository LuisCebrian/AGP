//***************************************************************************************
// BoxDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates rendering a colored box.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3DX11EffectScalarVariable* mTessFactor;

	ID3D11InputLayout* mInputLayout;

	ID3D11RasterizerState* mWireframeRS;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMFLOAT4X4 mCenterSphere;
	XMFLOAT4X4 mLeftSphere;
	XMFLOAT4X4 mRightSphere;
	XMFLOAT4X4 mOctagonalPrism;

	UINT mSphereIndexCount;
	UINT mSphereVertexCount;
	UINT mOctagonalPrismIndexCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	BoxApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 

BoxApp::BoxApp(HINSTANCE hInstance)
: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mFX(0), mTech(0),
  mfxWorldViewProj(0), mInputLayout(0), 
  mTheta(1.5f*MathHelper::Pi), mPhi(0.25f*MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"Box Demo";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 1.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	XMMATRIX leftSphereScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX leftSphereOffset = XMMatrixTranslation(-3.0f, 4.0f, 0.0f);
	XMStoreFloat4x4(&mLeftSphere, XMMatrixMultiply(leftSphereScale, leftSphereOffset));

	XMMATRIX rightSphereScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX rightSphereOffset = XMMatrixTranslation(3.0f, 4.0f, 0.0f);
	XMStoreFloat4x4(&mRightSphere, XMMatrixMultiply(rightSphereScale, rightSphereOffset));

	XMMATRIX prismOffset = XMMatrixTranslation(0.0f, -4.0f, 0.0f);
	XMStoreFloat4x4(&mOctagonalPrism, prismOffset);
}

BoxApp::~BoxApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mFX);
	ReleaseCOM(mInputLayout);
}

bool BoxApp::Init()
{
	if(!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_NONE;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	mTessFactor->SetFloat(1);

	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));
	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::UpdateScene(float dt)
{
	if (GetAsyncKeyState('1') & 0x8000)
		mTessFactor->SetFloat(1);
	if (GetAsyncKeyState('2') & 0x8000)
		mTessFactor->SetFloat(2);
	if (GetAsyncKeyState('3') & 0x8000)
		mTessFactor->SetFloat(3);

	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void BoxApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);


	md3dImmediateContext->RSSetState(mWireframeRS);

	UINT stride = sizeof(Vertex);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);


	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world*view*proj;

	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        
		world = XMLoadFloat4x4(&mWorld);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*view*proj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		md3dImmediateContext->RSSetState(mWireframeRS);
		md3dImmediateContext->DrawIndexed(18, 18, 0);

		world = XMLoadFloat4x4(&mWorld);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*view*proj)));
		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->DrawIndexed(18, 0, 0);

		md3dImmediateContext->RSSetState(mWireframeRS);
		world = XMLoadFloat4x4(&mCenterSphere);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*view*proj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSphereIndexCount, 36, 9);

		world = XMLoadFloat4x4(&mLeftSphere);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*view*proj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSphereIndexCount, 36, 9);

		world = XMLoadFloat4x4(&mRightSphere);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*view*proj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mSphereIndexCount, 36, 9);

		//Prism
		
		world = XMLoadFloat4x4(&mOctagonalPrism);
		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&(world*view*proj)));
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mOctagonalPrismIndexCount, 36 + mSphereIndexCount, 9 + mSphereVertexCount);
		
	}

	HR(mSwapChain->Present(0, 0));
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void createOctagonalPrism(float radius, float height,GeometryGenerator::MeshData & meshData)
{
	meshData.Vertices.clear();
	meshData.Indices.clear();

	UINT sliceCount = 8;
	
	float dTheta = 2.0f*XM_PI / sliceCount;
	//Build the vertex buffer
	//Bottom
	float y = -0.5f*height;
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		GeometryGenerator::Vertex vertex;

		float c = cosf(i*dTheta);
		float s = sinf(i*dTheta);

		vertex.Position = XMFLOAT3(radius*c, y, radius*s);
		meshData.Vertices.push_back(vertex);
	}

	//Top
	y = 0.5f*height;
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		GeometryGenerator::Vertex vertex;

		float c = cosf(i*dTheta);
		float s = sinf(i*dTheta);

		vertex.Position = XMFLOAT3(radius*c, y, radius*s);
		meshData.Vertices.push_back(vertex);
	}

	//Build the indices
	for (UINT i = 0; i < 1; ++i)
	{
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			meshData.Indices.push_back(i*sliceCount + j);
			meshData.Indices.push_back((i + 1)*sliceCount + j);
			meshData.Indices.push_back((i + 1)*sliceCount + j + 1);

			meshData.Indices.push_back(i*sliceCount + j);
			meshData.Indices.push_back((i + 1)*sliceCount+ j + 1);
			meshData.Indices.push_back(i*sliceCount + j + 1);
		}
	}
	
	//Tap and bottom of the prism
	y = 0.5f*height;
	meshData.Vertices.push_back(GeometryGenerator::Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));
	y = -0.5f*height;
	meshData.Vertices.push_back(GeometryGenerator::Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));
	
	for (UINT j = 0; j < 2; ++j) 
	{
		UINT centerIndex;
		if (j == 0)
			centerIndex = meshData.Vertices.size() - 1;
		else
			centerIndex = meshData.Vertices.size() - 2;
		for (UINT i = 0; i < sliceCount; ++i)
		{
			if (j == 0) 
			{
				meshData.Indices.push_back(centerIndex);
				meshData.Indices.push_back(j*sliceCount+ i);
				meshData.Indices.push_back(j*sliceCount+ i + 1);	
			}
			else
			{
				meshData.Indices.push_back(centerIndex);
				meshData.Indices.push_back(j*sliceCount+ i + 1);
				meshData.Indices.push_back(j*sliceCount + i);

			}
		}
	}
}

void BoxApp::BuildGeometryBuffers()
{
	// Create vertex buffer

	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData octagonalPrism;

	GeometryGenerator geoGen;
	geoGen.CreateSphere(2.0f, 20, 20, sphere);
	createOctagonalPrism(1.0f, 2.0f, octagonalPrism);
	mOctagonalPrismIndexCount = octagonalPrism.Indices.size();
	mSphereIndexCount   = sphere.Indices.size();
	mSphereVertexCount = sphere.Vertices.size();

    Vertex verticesArray[] =
    {
		//First pyramid
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*)&Colors::White   },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*)&Colors::Black   },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*)&Colors::Red     },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*)&Colors::Green   },
		{ XMFLOAT3(0.0f, +1.0f, 0.0f), (const float*)&Colors::Blue    },
		//Second pyramid
		{ XMFLOAT3(-1.0f, +3.0f, +1.0f), (const float*)&Colors::White   },
		{ XMFLOAT3(+1.0f, +3.0f, +1.0f), (const float*)&Colors::Black },
		{ XMFLOAT3(+1.0f, +3.0f, -1.0f), (const float*)&Colors::Red },
		{ XMFLOAT3(-1.0f, +3.0f, -1.0f), (const float*)&Colors::Green}

    };
	
	UINT pyramidVertexCount = 9;
	UINT totalVertexCount = sphere.Vertices.size() + octagonalPrism.Vertices.size() + pyramidVertexCount;
	std::vector<Vertex> vertices(totalVertexCount);
	
	//Pyramids
	UINT k = 0;
	for(size_t i = 0; i < pyramidVertexCount; ++i, ++k)
	{
		vertices[k].Pos = verticesArray[i].Pos;
		vertices[k].Color = verticesArray[i].Color;
	}
	
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
	//Sphere
	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos   = sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}

	//Octagonal Prism
	for(size_t i = 0; i < octagonalPrism.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos   = octagonalPrism.Vertices[i].Position;
		vertices[k].Color = black;
	}


    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));


	// Create the index buffer

	std::vector<UINT> indices = {
		3, 1, 0,
		3, 2, 1,

		0, 1, 4,
		1, 2, 4,

		0, 4, 3,
		2, 3, 4,
		
		//Second pyramid
		4, 6, 5,
		4, 7, 6,

		4, 8, 7,
		4, 5, 8,
		
		5, 6, 8,
		6, 7, 8

	};
	
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), octagonalPrism.Indices.begin(), octagonalPrism.Indices.end());

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}
 
void BoxApp::BuildFX()
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
 
	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(L"FX/color.fx", 0, 0, 0, "fx_5_0", shaderFlags, 
		0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs can store errors or warnings.
	if( compilationMsgs != 0 )
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	// Even if there are no compilationMsgs, check to make sure there were no other errors.
	if(FAILED(hr))
	{
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 
		0, md3dDevice, &mFX));

	// Done with compiled shader.
	ReleaseCOM(compiledShader);

	mTech    = mFX->GetTechniqueByName("Tess");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();	
	mTessFactor = mFX->GetVariableByName("tessFactor")->AsScalar();

}

void BoxApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &mInputLayout));
}
 