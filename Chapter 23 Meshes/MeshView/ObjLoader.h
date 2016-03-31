#pragma once

#include "MeshGeometry.h"
#include "LightHelper.h"
#include "Vertex.h"

class ObjLoader
{
public:
	static const size_t Buffersize = 4096;
	typedef std::vector<char>::iterator DataArrayIt;
	bool LoadObj(const std::string& filename,
		std::vector<Vertex::PosNormalTexTan>& vertices,
		std::vector<USHORT>& indices);
private:
	void ReadVertices(std::ifstream& fin, std::vector<Vertex::Basic32>& vertices);
	void ReadIndices(std::ifstream& fin, std::vector<USHORT>& indices);
	void ObjLoader::ReadNormals(std::ifstream & fin, std::vector<Vertex::Basic32>& vertices);

	void ObjLoader::ParseFile();
	void ObjLoader::getVector(std::vector<Vertex::Basic32>& vertices);
	void ObjLoader::copyNextWord(char *pBuffer, size_t length);
	void ObjLoader::copyNextLine(char *pBuffer, size_t length);
	DataArrayIt m_DataIt;
	DataArrayIt m_DataItEnd;
	char m_buffer[Buffersize];

	static const unsigned int BufferSize = 4096;

	// ---------------------------------------------------------------------------------
	template <class char_t>
	char_t ToLower(char_t in)
	{
		return (in >= (char_t)'A' && in <= (char_t)'Z') ? (char_t)(in + 0x20) : in;
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	char_t ToUpper(char_t in) {
		return (in >= (char_t)'a' && in <= (char_t)'z') ? (char_t)(in - 0x20) : in;
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool IsUpper(char_t in)
	{
		return (in >= (char_t)'A' && in <= (char_t)'Z');
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool IsLower(char_t in)
	{
		return (in >= (char_t)'a' && in <= (char_t)'z');
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool IsSpace(char_t in)
	{
		return (in == (char_t)' ' || in == (char_t)'\t');
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool IsLineEnd(char_t in)
	{
		return (in == (char_t)'\r' || in == (char_t)'\n' || in == (char_t)'\0' || in == (char_t)'\f');
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool IsSpaceOrNewLine(char_t in)
	{
		return IsSpace<char_t>(in) || IsLineEnd<char_t>(in);
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool SkipSpaces(const char_t* in, const char_t** out)
	{
		while (*in == (char_t)' ' || *in == (char_t)'\t') {
			++in;
		}
		*out = in;
		return !IsLineEnd<char_t>(*in);
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool SkipSpaces(const char_t** inout)
	{
		return SkipSpaces<char_t>(*inout, inout);
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool SkipLine(const char_t* in, const char_t** out)
	{
		while (*in != (char_t)'\r' && *in != (char_t)'\n' && *in != (char_t)'\0') {
			++in;
		}

		// files are opened in binary mode. Ergo there are both NL and CR
		while (*in == (char_t)'\r' || *in == (char_t)'\n') {
			++in;
		}
		*out = in;
		return *in != (char_t)'\0';
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool SkipLine(const char_t** inout)
	{
		return SkipLine<char_t>(*inout, inout);
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool SkipSpacesAndLineEnd(const char_t* in, const char_t** out)
	{
		while (*in == (char_t)' ' || *in == (char_t)'\t' || *in == (char_t)'\r' || *in == (char_t)'\n') {
			++in;
		}
		*out = in;
		return *in != '\0';
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool SkipSpacesAndLineEnd(const char_t** inout)
	{
		return SkipSpacesAndLineEnd<char_t>(*inout, inout);
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool GetNextLine(const char_t*& buffer, char_t out[BufferSize])
	{
		if ((char_t)'\0' == *buffer) {
			return false;
		}

		char* _out = out;
		char* const end = _out + BufferSize;
		while (!IsLineEnd(*buffer) && _out < end) {
			*_out++ = *buffer++;
		}
		*_out = (char_t)'\0';

		while (IsLineEnd(*buffer) && '\0' != *buffer) {
			++buffer;
		}

		return true;
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool IsNumeric(char_t in)
	{
		return (in >= '0' && in <= '9') || '-' == in || '+' == in;
	}

	// ---------------------------------------------------------------------------------
	template <class char_t>
	bool TokenMatch(char_t*& in, const char* token, unsigned int len)
	{
		if (!::strncmp(token, in, len) && IsSpaceOrNewLine(in[len])) {
			if (in[len] != '\0') {
				in += len + 1;
			}
			else {
				// If EOF after the token make sure we don't go past end of buffer
				in += len;
			}
			return true;
		}

		return false;
	}
	// ---------------------------------------------------------------------------------
	void SkipToken(const char*& in)
	{
		SkipSpaces(&in);
		while (!IsSpaceOrNewLine(*in))++in;
	}
	// ---------------------------------------------------------------------------------
	std::string GetNextToken(const char*& in)
	{
		SkipSpacesAndLineEnd(&in);
		const char* cur = in;
		while (!IsSpaceOrNewLine(*in))++in;
		return std::string(cur, (size_t)(in - cur));
	}
	// ---------------------------------------------------------------------------------
};

