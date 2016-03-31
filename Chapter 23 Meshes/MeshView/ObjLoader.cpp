#include "ObjLoader.h"


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

bool ObjLoader::LoadObj(const std::string & filename, std::vector<Vertex::PosNormalTexTan>& vertices, std::vector<USHORT>& indices)
{
	std::ifstream fin(filename);
	std::vector<char> fileContents((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
	/*
	if (!fin) 
	{
		return false;
	}
	*/
	std::string line;
	std::string ignore;
	std::string h1;
	std::string h2;
	std::string h3;
	std::string h4;
	int verticesCoordCount = 0;
	int verticesNormCount = 0;
	int indicesCount = 0;
	
	while(std::getline(fin,line))
	{
		std::istringstream iss(line);
		if (line.length() == 0)
		{
			continue;
		}
		char firstElement = line[0];

		switch (firstElement) 
		{
			case '#':
				break;

			case 'v':
				if(line[1] == 'n')
				{
					iss >> ignore >> vertices[verticesNormCount].Normal.x >> vertices[verticesNormCount].Normal.y >> vertices[verticesNormCount].Normal.z;
					verticesNormCount++;
				}
				else
				{
					iss >> ignore >> vertices[verticesCoordCount].Pos.x >> vertices[verticesCoordCount].Pos.y >> vertices[verticesCoordCount].Pos.z;
					verticesCoordCount++;
				}
				break;

			case 'f':
					iss >> ignore >> h1 >> h2 >> h3 >> h4;
					indices[indicesCount*4+0]= (USHORT)atoi(split(h1, '/')[0].c_str());
					indices[indicesCount*4+1]= (USHORT)atoi(split(h2, '/')[0].c_str());
					indices[indicesCount*4+2]= (USHORT)atoi(split(h3, '/')[0].c_str());
					indices[indicesCount*4+3]= (USHORT)atoi(split(h4, '/')[0].c_str());
					indicesCount++;
				break;

			default:
				break;
		}

	}
	
	/*
	m_DataIt = fileContents.begin();
	m_DataItEnd = fileContents.end();
	std::fill_n(m_buffer, Buffersize, 0);
	*/
	return true;
}

/*

void ObjLoader::ReadIndices(std::ifstream & fin, std::vector<USHORT>& indices)
{
	std::string ignore;
	UINT i = 0;
	std::string line;
	std::string h1;
	std::string h2;
	std::string h3;
	std::string h4;
	while(std::getline(fin,line))
	{
		std::istringstream iss(line);
		if (!(iss >> ignore >> h1 >> h2 >> h3 >> h4))
		{
			break;
		}
		indices[i*4+0]= (USHORT)atoi(split(h1, '/')[0].c_str());
		indices[i*4+1]= (USHORT)atoi(split(h2, '/')[0].c_str());
		indices[i*4+2]= (USHORT)atoi(split(h3, '/')[0].c_str());
		indices[i*4+3]= (USHORT)atoi(split(h4, '/')[0].c_str());
		i++;
	}
}
*/
