//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************
#define occlusion_enabled
#define occlusion_quality 4

#define noise_use_smoothstep

#define light_color float3(0.1,0.4,0.6)
#define light_direction normalize(float3(.2,1.0,-0.2))
#define light_speed_modifier 1.0

#define object_color float3(0.9,0.1,0.1)
#define object_count 9
#define object_speed_modifier 1.0

#define render_steps 33
//https://www.shadertoy.com/view/Mss3WN
cbuffer cbPerObject
{
	float4x4 gWorldViewProj; 
};

struct VertexIn
{
	float3 Pos   : POSITION;
	float4 Color : COLOR;
};

cbuffer globalTime
{
	float iGlobalTime;
};
struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float hash(float x)
{
	return frac(sin(x*.0127863)*17143.321);
}

float hash(float2 x)
{
	return frac(cos(dot(x.xy, float2(2.31, 53.21))*124.123)*412.0);
}

float3 cc(float3 color, float factor, float factor2) //a wierd color modifier
{
	float w = color.x + color.y + color.z;
	return lerp(color, float3(w,w,w)*factor, w*factor2);
}

float hashmix(float x0, float x1, float interp)
{
	x0 = hash(x0);
	x1 = hash(x1);
	#ifdef noise_use_smoothstep
		interp = smoothstep(0.0, 1.0, interp);
	#endif
	return lerp(x0, x1, interp);
}

float noise(float p) // 1D noise
{
	float pm = fmod(p, 1.0);
	float pd = p - pm;
	return hashmix(pd, pd + 1.0, pm);
}

float3 rotate_y(float3 v, float angle)
{
	float ca = cos(angle); 
	float sa = sin(angle);
	float3x3  p = float3x3( +ca, +.0, -sa, +.0, +1.0, +.0, +sa, +.0, +ca);
	return mul(v,p);
}

float3 rotate_x(float3 v, float angle)
{
	float ca = cos(angle); float sa = sin(angle);
	float3x3 p = float3x3(+1.0, +.0, +.0, +.0, +ca, -sa, +.0, +sa, +ca);
	return mul(v,p);
}

float max3(float a, float b, float c)//returns the maximum of 3 values
{
	return max(a, max(b, c));
}

static float3 bpos[object_count];

float dist(float3 p)//distance function
{
	float d = 1024.0;
	float nd;
	for (int i = 0; i<object_count; i++)
	{
		float3 np = p + bpos[i];
		float shape0 = max3(abs(np.x), abs(np.y), abs(np.z)) - 1.0;
		float shape1 = length(np) - 1.0;
		nd = shape0 + (shape1 - shape0)*2.0;
		d = lerp(d, nd, smoothstep(-1.0, +1.0, d - nd));
	}
	return d;
}

float3 normal(float3 p, float e) //returns the normal, uses the distance function
{
	float d = dist(p);
	return normalize(float3(dist(p + float3(e, 0, 0)) - d, dist(p + float3(0, e, 0)) - d, dist(p + float3(0, 0, e)) - d));
}

float3 light = light_direction; //global variable that holds light direction

float3 background(float3 d)//render background
{
	float t = iGlobalTime*0.5*light_speed_modifier;
	float qq = dot(d, light)*.5 + .5;
	float bgl = qq;
	float q = (bgl + noise(bgl*6.0 + t)*.85 + noise(bgl*12.0 + t)*.85);
	q += pow(qq, 32.0)*2.0;
	float3 sky = float3(0.1, 0.4, 0.6)*q;
	return sky;
}

float occlusion(float3 p, float3 d)//returns how much a point is visible from a given direction
{
	float occ = 1.0;
	p = p + d;
	for (int i = 0; i<occlusion_quality; i++)
	{
		float dd = dist(p);
		p += d*dd;
		occ = min(occ, dd);
	}
	return max(.0, occ);
}

float3 object_material(float3 p, float3 d)
{
	float3 color = normalize(object_color*light_color);
	float3 n = normal(p, 0.1);
	float3 r = reflect(d, n);

	float reflectance = dot(d, r)*.5 + .5; reflectance = pow(reflectance, 2.0);
	float diffuse = dot(light, n)*.5 + .5; diffuse = max(.0, diffuse);

	#ifdef occlusion_enabled
		float oa = occlusion(p, n)*.4 + .6;
		float od = occlusion(p, light)*.95 + .05;
		float os = occlusion(p, r)*.95 + .05;
	#else
		float oa = 1.0;
		float ob = 1.0;
		float oc = 1.0;
	#endif

	#ifndef occlusion_preview
		color =
			color*oa*.2 + //ambient
			color*diffuse*od*.7 + //diffuse
			background(r)*os*reflectance*.7; //reflection
	#else
		color = vec3((oa + od + os)*.3);
	#endif

	return color;
}

#define offset1 4.7
#define offset2 4.6

float4 PS(VertexOut pin) : SV_Target
{
	float2 iResolution = float2(800,800);
	float3 iMouse = float3(400,400,150);
	float2 uv = pin.PosH.xy / iResolution.xy - 0.5;
	uv.x *= 1; //fix aspect ratio
	float3 mouse = float3(iMouse.xy / iResolution.xy - 0.5,iMouse.z - .5);

	float t = iGlobalTime*.5*object_speed_modifier + 2.0;

	for (int i = 0; i<object_count; i++) //position for each metaball
	{
		bpos[i] = 1.3*float3(
			sin(t*0.967 + float(i)*42.0),
			sin(t*.423 + float(i)*152.0),
			sin(t*.76321 + float(i)));
	}

	//setup the camera
	float3 p = float3(.0,0.0,-4.0);
	p = rotate_x(p,mouse.y*9.0 + offset1);
	p = rotate_y(p,mouse.x*9.0 + offset2);
	float3 d = float3(uv,1.0);
	d.z -= length(d)*.5; //lens distort
	d = normalize(d);
	d = rotate_x(d,mouse.y*9.0 + offset1);
	d = rotate_y(d,mouse.x*9.0 + offset2);

	//and action!
	float dd;
	float3 color;
	for (int i = 0; i<render_steps; i++) //raymarch
	{
		dd = dist(p);
		p += d*dd*.7;
		if (dd<.04 || dd>4.0) break;
	}

	if (dd<0.5) //close enough
		color = object_material(p,d);
	else
		color = background(d);

	//post procesing
	color *= .85;
	color = lerp(color,color*color,0.3);
	color -= hash(color.xy + uv.xy)*.015;
	color -= length(uv)*.1;
	color = cc(color,.5,.6);
	pin.Color = float4(color,1.0);
    return pin.Color;
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

