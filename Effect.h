#pragma once
#include "Texture.h"
class Effect
{
public:

	Effect(ID3D11Device* device, const std::wstring& assetFile);
	~Effect();
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	ID3DX11Effect* GetEffect();
	ID3DX11EffectTechnique* GetTechnique();
	void UpdateData(dae::Matrix* worldViewProjection);
	void SetMatrix(const dae::Matrix* matrix, const dae::Matrix* worldMatrix, const dae::Vector3* cameraPos);
	void SetMaps(dae::Texture* pDiffuseTexture, dae::Texture* pSpecularMap, dae::Texture* pNormalMap, dae::Texture* pGlossMap);
	void SetMaps(dae::Texture* pDiffuseTexture);
	void ChangeEffect(LPCSTR name);
	void SetSampler(ID3D11SamplerState* sampler);
private:
	
	ID3DX11EffectShaderResourceVariable* m_pDiffuse{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pGloss{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pNormals{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pSpec{ nullptr };
	ID3DX11Effect* m_pEffect{ nullptr };
	ID3DX11EffectTechnique* m_pTechnique{ nullptr };
	ID3DX11EffectMatrixVariable* m_pWorldViewProjMatrix{ nullptr };
	ID3DX11EffectMatrixVariable* m_pWorldMatrix{ nullptr };
	ID3DX11EffectVectorVariable* m_pOnbMatrix{ nullptr };

	ID3DX11EffectSamplerVariable* m_pSampleVar{ nullptr };
};

