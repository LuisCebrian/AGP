//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject {
	float4x4 gWorldViewProj;
};

cbuffer tess{
	float tessFactor;
};
struct VertexIn
{
	float3 PosL : POSITION;
	float4 Color: COLOR;
};

struct VertexOut
{
	float3 PosL : POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosL = vin.PosL;
	vout.Color = vin.Color;

	return vout;
};

struct PatchTess
{
	float EdgeTess[3]    : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	pt.EdgeTess[0] = tessFactor;
	pt.EdgeTess[1] = tessFactor;
	pt.EdgeTess[2] = tessFactor;

	pt.InsideTess = 1;

	return pt;
}

struct HullOut
{
	float3 PosL : POSITION;
	float4 Color : COLOR;
};

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 3> p,
	uint i : SV_OutputControlPointID,
	uint patchId : SV_PrimitiveID)
{
	HullOut hout;

	hout.PosL = p[i].PosL;
	hout.Color = p[i].Color;

	return hout;
}

struct DomainOut
{
	float4 PosH : SV_POSITION;
	float4 Color: COLOR;
};

[domain("tri")]
DomainOut DS(PatchTess patchTess,
	float3 abc: SV_DomainLocation,
	const OutputPatch<HullOut, 3> tri)
{
	DomainOut dout;

	float3 p = abc.x * tri[0].PosL + abc.y * tri[1].PosL + abc.z*tri[2].PosL;
	dout.PosH = mul(float4(p, 1.0f), gWorldViewProj);
	dout.Color = tri[0].Color;
	return dout;

}

float4 PS(DomainOut pin) : SV_Target
{
	return pin.Color;
}

technique11 Tess
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetHullShader(CompileShader(hs_5_0, HS()));
		SetDomainShader(CompileShader(ds_5_0, DS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}