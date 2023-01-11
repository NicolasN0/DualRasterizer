float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : World;
float3 gOnb : ViewInverse;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gGlossyMap : GlossyMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gNormalMap : NormalMap;

float3 lightDirection = float3(0.577f, -0.577f, 0.577f);
float diffuseReflectance = 7.f; //kd
float shininess = 25.f;
float PI = 3.14;

//
// Sampler States
//
//SamplerState gState : SV_SampleIndex;
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samPointLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samPointAni
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false; //default;
};

RasterizerState gRasterizerStateDefault
{
   
};

BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState gBlendStateDefault
{
 
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
    //Demo purposes
    StencilReadMask = 0x0F;
    StencilWriteMask = 0x0F;

    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;

    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
};

DepthStencilState gDepthStencilStateDefault
{
   
};



//
// Input/Output Structs
//

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 Uv : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
    float2 Uv : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

float3 Phong(float3 ks, float exp, float3 l, float3 v, float3 n)
{
	float3 reflect = l - 2 * (dot(n,l) * n);
	float cosa = dot(reflect, v);
	return ks * pow(cosa, exp);
}

float3 Lambert(float3 color)
{
    return (color * diffuseReflectance) / PI;
}

//
// Vertex Shader
//
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.Color = float4(input.Color, 1.f);
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
    output.Uv = input.Uv;
    return output;
}

//
//Pixel Shaders
//
float3 CalculatePixelShader(VS_OUTPUT input, float4 gloss, float4 spec, float4 color, float4 normals)
{
    float3 viewDir = (input.Position.rgb - gOnb);
    float3 binormal = cross(input.Normal, input.Tangent);
    float3x3 tangentSpaceAxis = float3x3(input.Tangent.x, input.Tangent.y, input.Tangent.z, binormal.x, binormal.y, binormal.z, input.Normal.x, input.Normal.y, input.Normal.z);
    float3 computedNormals = normalize(mul(normals.rgb, tangentSpaceAxis) + input.Normal);

    //Lambert cosine law (is correct)
    float observedArea = saturate(dot(computedNormals, -lightDirection));
    clamp(observedArea, 0, 1);

    //diffuse (is correct)
    float3 diffuse = Lambert(color.rgb) * observedArea;

    //specular
    float3 specular = saturate(Phong(spec.rgb, gloss.x * shininess, -lightDirection, normalize(viewDir), computedNormals));

    //phong
    float3 phong = diffuse + specular;
    return phong;
}

//float4 PS1(VS_OUTPUT input) : SV_TARGET
//{
//    //maps
//    float4 gloss = gGlossyMap.Sample(samPoint, input.Uv);
//    float4 spec = gSpecularMap.Sample(samPoint, input.Uv);
//    float4 color = gDiffuseMap.Sample(samPoint, input.Uv);
//    float4 normals = gNormalMap.Sample(samPoint, input.Uv);
//
//    float3 viewDir = (input.Position.rgb - gOnb);
//    float3 binormal = cross(input.Normal, input.Tangent);
//    float3x3 tangentSpaceAxis = float3x3(input.Tangent.x, input.Tangent.y, input.Tangent.z, binormal.x, binormal.y, binormal.z, input.Normal.x, input.Normal.y, input.Normal.z);
//    float3 computedNormals = normalize(mul(normals.rgb, tangentSpaceAxis) + input.Normal);
//
//    //Lambert cosine law (is correct)
//    float observedArea = saturate(dot(computedNormals, -lightDirection));
//    clamp(observedArea, 0, 1);
//
//    //diffuse (is correct)
//    float3 diffuse = Lambert(color.rgb) * observedArea;
//
//    ////specular
//    //float3 specular = saturate(Phong(spec.rgb, gloss.x * shininess, -lightDirection, normalize(viewDir), computedNormals));
//
//    ////phong
//    //float3 phong = diffuse + specular;
//
//    float4 output;
//    //output.rgb = (observedArea, observedArea, observedArea);
//    output.rgb = diffuse;
//    //output.rgb = specular;
//    //output.rgb = phong * observedArea;
//    output.a = 1;
//    return output;
//}
//
////specular
//float4 PS2(VS_OUTPUT input) : SV_TARGET
//{
//    //maps
//     float4 gloss = gGlossyMap.Sample(samPoint, input.Uv);
//     float4 spec = gSpecularMap.Sample(samPoint, input.Uv);
//     float4 color = gDiffuseMap.Sample(samPoint, input.Uv);
//     float4 normals = gNormalMap.Sample(samPoint, input.Uv);
//
//     float3 viewDir = (input.Position.rgb - gOnb);
//     float3 binormal = cross(input.Normal, input.Tangent);
//     float3x3 tangentSpaceAxis = float3x3(input.Tangent.x, input.Tangent.y, input.Tangent.z, binormal.x, binormal.y, binormal.z, input.Normal.x, input.Normal.y, input.Normal.z);
//     float3 computedNormals = normalize(mul(normals.rgb, tangentSpaceAxis) + input.Normal);
//
//     //Lambert cosine law (is correct)
//     float observedArea = saturate(dot(computedNormals, -lightDirection));
//     clamp(observedArea, 0, 1);
//
//     //diffuse (is correct)
//     float3 diffuse = Lambert(color.rgb) * observedArea;
//
//     //specular
//     float3 specular = saturate(Phong(spec.rgb, gloss.x * shininess, -lightDirection, normalize(viewDir), computedNormals));
//
//     ////phong
//     //float3 phong = diffuse + specular;
//
//     float4 output;
//     //output.rgb = (observedArea, observedArea, observedArea);
//     //output.rgb = diffuse;
//     output.rgb = specular;
//     //output.rgb = phong * observedArea;
//     output.a = 1;
//     return output;
//}
//
////phong
//float4 PS3(VS_OUTPUT input) : SV_TARGET
//{
//    //maps
//    float4 gloss = gGlossyMap.Sample(samPoint, input.Uv);
//    float4 spec = gSpecularMap.Sample(samPoint, input.Uv);
//    float4 color = gDiffuseMap.Sample(samPoint, input.Uv);
//    float4 normals = gNormalMap.Sample(samPoint, input.Uv);
//
//    float3 viewDir = (input.Position.rgb - gOnb);
//    float3 binormal = cross(input.Normal, input.Tangent);
//    float3x3 tangentSpaceAxis = float3x3(input.Tangent.x, input.Tangent.y, input.Tangent.z, binormal.x, binormal.y, binormal.z, input.Normal.x, input.Normal.y, input.Normal.z);
//    float3 computedNormals = normalize(mul(normals.rgb, tangentSpaceAxis) + input.Normal);
//
//    //Lambert cosine law (is correct)
//    float observedArea = saturate(dot(computedNormals, -lightDirection));
//    clamp(observedArea, 0, 1);
//
//    //diffuse (is correct)
//    float3 diffuse = Lambert(color.rgb) * observedArea;
//
//    //specular
//    float3 specular = saturate(Phong(spec.rgb, gloss.x * shininess, -lightDirection, normalize(viewDir), computedNormals));
//
//    //phong
//    float3 phong = diffuse + specular;
//
//    float4 output;
//    //output.rgb = (observedArea, observedArea, observedArea);
//    //output.rgb = diffuse * observedArea;
//    //output.rgb = specular;
//    output.rgb = phong;
//    output.a = 1;
//    return output;
//}

float4 PS1(VS_OUTPUT input) : SV_TARGET
{
    //maps
    float4 gloss = gGlossyMap.Sample(samPoint, input.Uv);
    float4 spec = gSpecularMap.Sample(samPoint, input.Uv);
    float4 color = gDiffuseMap.Sample(samPoint, input.Uv);
    float4 normals = gNormalMap.Sample(samPoint, input.Uv);

    float4 output;
    output.rgb = CalculatePixelShader(input, gloss, spec, color, normals);
    output.a = 1;
    return output;
}

//specular
float4 PS2(VS_OUTPUT input) : SV_TARGET
{
    //maps
     float4 gloss = gGlossyMap.Sample(samPointLinear, input.Uv);
     float4 spec = gSpecularMap.Sample(samPointLinear, input.Uv);
     float4 color = gDiffuseMap.Sample(samPointLinear, input.Uv);
     float4 normals = gNormalMap.Sample(samPointLinear, input.Uv);

     float4 output;
     output.rgb = CalculatePixelShader(input, gloss, spec, color, normals);
     output.a = 1;
     return output;
}

//phong
float4 PS3(VS_OUTPUT input) : SV_TARGET
{
    //maps
    float4 gloss = gGlossyMap.Sample(samPointAni, input.Uv);
    float4 spec = gSpecularMap.Sample(samPointAni, input.Uv);
    float4 color = gDiffuseMap.Sample(samPointAni, input.Uv);
    float4 normals = gNormalMap.Sample(samPointAni, input.Uv);

    float4 output;
    output.rgb = CalculatePixelShader(input, gloss, spec, color, normals);
    output.a = 1;
    return output;
}

float4 PS4(VS_OUTPUT input) : SV_TARGET
{
    //maps
    float4 gloss = gGlossyMap.Sample(samPoint, input.Uv);
    float4 spec = gSpecularMap.Sample(samPoint, input.Uv);
    float4 color = gDiffuseMap.Sample(samPoint, input.Uv);
    float4 normals = gNormalMap.Sample(samPoint, input.Uv);

    float3 viewDir = (input.Position.rgb - gOnb);
    float3 binormal = cross(input.Normal, input.Tangent);
    float3x3 tangentSpaceAxis = float3x3(input.Tangent.x, input.Tangent.y, input.Tangent.z, binormal.x, binormal.y, binormal.z, input.Normal.x, input.Normal.y, input.Normal.z);
    float3 computedNormals = normalize(mul(normals.rgb, tangentSpaceAxis) + input.Normal);

    //Lambert cosine law (is correct)
    float observedArea = saturate(dot(computedNormals, -lightDirection));
    clamp(observedArea, 0, 1);

    //diffuse
    float3 diffuse = Lambert(color.rgb);

    //specular
    float specular = Phong(spec.rgb, gloss.x * shininess, -lightDirection, viewDir, computedNormals);

    //phong
    float3 phong = diffuse + specular;

    float4 output;
    //output.rgb = (observedArea, observedArea, observedArea);
    output.rgb = diffuse;
    //output.rgb = specular;
    //output.rgb = phong * observedArea;
    output.a = color.a;
    return output;
}

//
//Technique
//
technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerStateDefault);
        SetDepthStencilState(gDepthStencilStateDefault,0);
        SetBlendState(gBlendStateDefault, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS1()));
    }
}

technique11 LinearTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerStateDefault);
        SetDepthStencilState(gDepthStencilStateDefault, 0);
        SetBlendState(gBlendStateDefault, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS2()));
    }
}

technique11 AniTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerStateDefault);
        SetDepthStencilState(gDepthStencilStateDefault, 0);
        SetBlendState(gBlendStateDefault, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS3()));
    }
}

technique11 FlatTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS4()));
    }
}