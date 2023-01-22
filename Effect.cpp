#include "pch.h"
#include "Effect.h"

Effect::Effect(ID3D11Device* device, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(device, assetFile);

	

	//technique
	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
		std::wcout << L"Technique is not valid\n";

	m_pWorldViewProjMatrix = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pWorldViewProjMatrix->IsValid()) {
		std::wcout << L"m_pWorldViewProjMatrix is not valid! \n";
	}

	//maps
	m_pDiffuse = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuse->IsValid()) {
		std::wcout << L"m_pDiffuse is not valid! \n";
	}
	m_pGloss = m_pEffect->GetVariableByName("gGlossyMap")->AsShaderResource();
	if (!m_pDiffuse->IsValid()) {
		std::wcout << L"m_pGloss is not valid! \n";
	}
	m_pNormals = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pDiffuse->IsValid()) {
		std::wcout << L"m_pNormals is not valid! \n";
	}
	m_pSpec = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pDiffuse->IsValid()) {
		std::wcout << L"m_pSpec is not valid! \n";
	}

	//matrix
	m_pWorldMatrix = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pWorldMatrix->IsValid()) {
		std::wcout << L"m_pWorldMatrix is not valid! \n";
	}

	m_pInverseViewMatrix = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if (!m_pInverseViewMatrix->IsValid()) {
		std::wcout << L"m_pInverseViewMatrix is not valid! \n";
	}

	//Samplers
	m_pSampleVar = m_pEffect->GetVariableByName("gSamp")->AsSampler();
	if (!m_pSampleVar->IsValid()) {
		std::wcout << L"m_pSampleVar is not valid! \n";
	}
}

Effect::~Effect()
{
	

	m_pDiffuse->Release();
	m_pDiffuse = nullptr;

	m_pGloss->Release();
	m_pGloss = nullptr;

	m_pNormals->Release();
	m_pNormals = nullptr;

	m_pSpec->Release();
	m_pSpec = nullptr;

	m_pWorldMatrix->Release();
	m_pWorldMatrix = nullptr;

	m_pWorldViewProjMatrix->Release();
	m_pWorldViewProjMatrix = nullptr;

	m_pInverseViewMatrix->Release();
	m_pInverseViewMatrix = nullptr;

	m_pEffect->GetVariableByName("gWorldViewProj")->Release();
	m_pEffect->GetVariableByName("gDiffuseMap")->Release();
	m_pEffect->GetVariableByName("gGlossyMap")->Release();
	m_pEffect->GetVariableByName("gNormalMap")->Release();
	m_pEffect->GetVariableByName("gSpecularMap")->Release();
	m_pEffect->GetVariableByName("gWorldMatrix")->Release();
	m_pEffect->GetVariableByName("gViewInverse")->Release();

	m_pEffect->GetTechniqueByName("DefaultTechnique")->Release();
	m_pEffect->GetTechniqueByName("LinearTechnique")->Release();
	m_pEffect->GetTechniqueByName("AniTechnique")->Release();
	m_pEffect->GetTechniqueByName("FlatTechnique")->Release();

	m_pTechnique->Release();
	m_pTechnique = nullptr;

	m_pEffect->Release();
	m_pEffect = nullptr;
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if(FAILED(result))
	{
		if(pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		} else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}
	return pEffect;
}

ID3DX11Effect* Effect::GetEffect()
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetTechnique()
{
	return m_pTechnique;
}

void Effect::UpdateData(dae::Matrix* worldViewProjection)
{
	m_pWorldViewProjMatrix->SetMatrix(reinterpret_cast<const float*>(worldViewProjection));
}


void Effect::SetMatrix(const dae::Matrix* matrix, const dae::Matrix* worldMatrix, const dae::Matrix* cameraPos)
{
	m_pWorldViewProjMatrix->SetMatrix(reinterpret_cast<const float*>(matrix));
	m_pWorldMatrix->SetMatrix(reinterpret_cast<const float*>(worldMatrix));
	m_pInverseViewMatrix->SetMatrix(reinterpret_cast<const float*>(cameraPos));
}

void Effect::SetMaps(dae::Texture* pDiffuseTexture, dae::Texture* pSpecularMap, dae::Texture* pNormalMap, dae::Texture* pGlossMap)
{
	if (m_pDiffuse) 
	{
		m_pDiffuse->SetResource(pDiffuseTexture->GetSRV());
	}
	if (m_pSpec) 
	{
		m_pSpec->SetResource(pSpecularMap->GetSRV());
	}
	if (m_pNormals) 
	{
		m_pNormals->SetResource(pNormalMap->GetSRV());
	}
	if (m_pGloss)
	{
		m_pGloss->SetResource(pGlossMap->GetSRV());
	}
}

void Effect::SetMaps(dae::Texture* pDiffuseTexture)
{
	if (m_pDiffuse) 
	{
		m_pDiffuse->SetResource(pDiffuseTexture->GetSRV());
	}
	
}
void Effect::ChangeEffect(LPCSTR name)
{
	m_pTechnique = m_pEffect->GetTechniqueByName(name);
}

void Effect::SetSampler(ID3D11SamplerState* sampler)
{
	m_pSampleVar->SetSampler(0,sampler);
}
