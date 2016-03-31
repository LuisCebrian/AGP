#include "BlenderModel.h"
#include <locale>
#include <codecvt>
#include <string>

BlenderModel::BlenderModel(Camera* mCamera, ID3D11Device* mDevice, ID3D11DeviceContext* mDeviceContext)
{
	mCam = mCamera;
	md3dDevice = mDevice;
	md3dImmediateContext = mDeviceContext;
}


BlenderModel::~BlenderModel()
{
	ReleaseCOM(mModel.VertexBuffer);
	ReleaseCOM(mModel.IndexBuffer); 
}

void BlenderModel::LoadModel(const std::string & filename,TextureMgr* mTexMgr)
{
	Assimp::Importer imp;

	const aiScene* pScene = imp.ReadFile(filename,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_ConvertToLeftHanded |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices);

	if (pScene == NULL)
		printf(imp.GetErrorString());

	std::vector<Vertex::Basic32> vertices;
	std::vector<USHORT> indices;
	std::vector<MeshGeometry::Subset> subsets;

	for (UINT i = 0; i < pScene->mNumMeshes; i++)
	{
		aiMesh* mesh = pScene->mMeshes[i];
		aiMaterial* material = pScene->mMaterials[mesh->mMaterialIndex];
		MeshGeometry::Subset subset;

		subset.VertexCount = mesh->mNumVertices;
		subset.VertexStart = vertices.size();
		subset.FaceStart = indices.size() / 3;
		subset.FaceCount = mesh->mNumFaces;
		subset.Id = mesh->mMaterialIndex;
		mModel.mNumFaces += mesh->mNumFaces;
		mModel.mNumVertices += mesh->mNumVertices;

		ReadVertices(mesh, vertices);
		ReadIndices(mesh, indices, subset);
		ReadMaterials(material);
		ReadTextures(material,mTexMgr);

		subsets.push_back(subset);
	}

	mModel.mSubsetCount = subsets.size();

	mModel.Mesh.SetSubsetTable(subsets);
	mModel.Mesh.SetIndices(md3dDevice, &indices[0], indices.size());
	mModel.Mesh.SetVertices(md3dDevice, &vertices[0], vertices.size());

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];

	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mModel.VertexBuffer));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];

	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mModel.IndexBuffer));
}

void BlenderModel::ReadVertices(aiMesh * mesh,std::vector<Vertex::Basic32> & vertices)
{
		for (UINT j = 0; j < mesh->mNumVertices; j++)
		{
			Vertex::Basic32 vertex;

			vertex.Pos.x = mesh->mVertices[j].x;
			vertex.Pos.y = mesh->mVertices[j].y;
			vertex.Pos.z = mesh->mVertices[j].z;

			vertex.Normal.x = mesh->mNormals[j].x;
			vertex.Normal.y = mesh->mNormals[j].y;
			vertex.Normal.z = mesh->mNormals[j].z;

			if (mesh->HasTextureCoords(0))
			{
				vertex.Tex.x = mesh->mTextureCoords[0][j].x;
				vertex.Tex.y = mesh->mTextureCoords[0][j].y;
			}

			vertices.push_back(vertex);
		}
}
void BlenderModel::ReadIndices(aiMesh * mesh, std::vector<USHORT> & indices, MeshGeometry::Subset subset)
{
	for (UINT c = 0; c < mesh->mNumFaces; c++)
	{
		for (UINT e = 0; e < mesh->mFaces[c].mNumIndices; e++)
		{
			indices.push_back(subset.VertexStart + mesh->mFaces[c].mIndices[e]);
		}

	}
}
void BlenderModel::ReadMaterials(aiMaterial * material)
{
	Material tempMat; 
	aiColor4D color(0.0f, 0.0f, 0.0f, 0.0f);

	material->Get(AI_MATKEY_COLOR_AMBIENT, color);
	tempMat.Ambient = XMFLOAT4(color.r, color.g, color.b, color.a);

	material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	tempMat.Diffuse = XMFLOAT4(color.r, color.g, color.b, color.a);

	material->Get(AI_MATKEY_COLOR_SPECULAR, color);
	tempMat.Specular = XMFLOAT4(color.r, color.g, color.b, color.a);

	material->Get(AI_MATKEY_COLOR_REFLECTIVE, color);
	tempMat.Reflect = XMFLOAT4(color.r, color.g, color.b, color.a);

	if (tempMat.Ambient.x == 0 && tempMat.Ambient.y == 0 && tempMat.Ambient.z == 0 && tempMat.Ambient.w == 0)
		tempMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	if (tempMat.Diffuse.x == 0 && tempMat.Diffuse.y == 0 && tempMat.Diffuse.z == 0 && tempMat.Diffuse.w == 0)
		tempMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	if (tempMat.Specular.x == 0 && tempMat.Specular.y == 0 && tempMat.Specular.z == 0 && tempMat.Specular.w == 0)
		tempMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	Materials.push_back(tempMat);

}
void BlenderModel::ReadTextures(aiMaterial *material, TextureMgr * mTexMgr)
{
	aiString path;
	std::string texDirectory = "Models";
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0 && material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
	{
		std::string fullPath = texDirectory + path.data;
		fullPath.replace(fullPath.length() - 3, fullPath.length(), "bmp");

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> stringConverter;
		std::wstring fullPathConverted = stringConverter.from_bytes(fullPath);

		ID3D11ShaderResourceView* texture = mTexMgr->CreateTexture(fullPathConverted);
		textureResourceView.push_back(texture);
	}
	
}
void BlenderModel::Render(CXMMATRIX world)
{
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light0TexTech;
	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);

	UINT Stride = sizeof(Vertex::Basic32);
	UINT Offset = 0;

	Effects::BasicFX->SetEyePosW(mCam->GetPosition());
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * mCam->ViewProj();

	Effects::BasicFX->SetWorld(world);
	Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
	Effects::BasicFX->SetWorldViewProj(worldViewProj);
	Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	md3dImmediateContext->RSSetState(0);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mModel.VertexBuffer, &Stride, &Offset);
		md3dImmediateContext->IASetIndexBuffer(mModel.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		for (UINT i = 0; i < mModel.mSubsetCount; i++)
		{
			Effects::BasicFX->SetMaterial(Materials[i]);
			Effects::BasicFX->SetDiffuseMap(textureResourceView[i]);
			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			mModel.Mesh.Draw(md3dImmediateContext, i);
		}
	}
}
