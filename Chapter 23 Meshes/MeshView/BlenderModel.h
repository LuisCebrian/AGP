#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "d3dUtil.h"
#include "Vertex.h"
#include "Effects.h"
#include "Camera.h"
#include "TextureMgr.h"
#include "MeshGeometry.h"

class BlenderModel
{
public:
	BlenderModel(Camera* mCam, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~BlenderModel();
	void LoadModel(const std::string& FileName,TextureMgr* mTexMgr);
	void Render(CXMMATRIX world);
	XMFLOAT4X4 mWorld;

private:
	struct ModelData
	{
		ModelData()
		{
			VertexBuffer = nullptr;
			IndexBuffer = nullptr;
			mNumVertices = 0;
			mNumFaces = 0;
			mSubsetCount = 0;
		}

		MeshGeometry Mesh;

		ID3D11Buffer* VertexBuffer;
		ID3D11Buffer* IndexBuffer;

		UINT mNumVertices;
		UINT mNumFaces;
		UINT mSubsetCount;
	};

	std::vector<ID3D11ShaderResourceView*> textureResourceView;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;
	std::vector<Material> Materials;
	
	ModelData mModel;

	void BlenderModel::ReadVertices(aiMesh * mesh, std::vector<Vertex::Basic32> & vertices);
	void BlenderModel::ReadIndices(aiMesh * mesh, std::vector<USHORT> & indices, MeshGeometry::Subset subset);
	void BlenderModel::ReadMaterials(aiMaterial * material);
	void BlenderModel::ReadTextures(aiMaterial *material,TextureMgr* mTexMgr);

	Camera* mCam;  
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
};

