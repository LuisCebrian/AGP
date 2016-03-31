#pragma once

#include "MeshGeometry.h"
#include "LightHelper.h"
#include "Vertex.h"

class M3bLoader
{
public:
	bool LoadM3b(const std::string& filename, std::vector<Vertex::PosNormalTexTan>& vertices, std::vector<USHORT>& indices);

};

