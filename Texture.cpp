#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <iostream>
#include <d3d11.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels },
		m_pDevice{ pDevice }
	{
		DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = m_pSurface->w;
		desc.Height = m_pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = m_pSurfacePixels;
		initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

		HRESULT hr{ pDevice->CreateTexture2D(&desc, &initData, &m_pResource) };
		if (SUCCEEDED(hr)) {

			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
			SRVDesc.Format = format;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = 1;

			hr = m_pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
			if (SUCCEEDED(hr)) {
				SDL_FreeSurface(m_pSurface);
				m_pSurface = nullptr;
			}
		}
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
		m_pSRV->Release();
		m_pResource->Release();
		m_pSRV = nullptr;
		m_pResource = nullptr;
	}

	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		//Load SDL_Surface using IMG_LOAD
		SDL_Surface* loadedSurface = IMG_Load(path.c_str());

		//Create & Return a new Texture Object (using SDL_Surface)
		return new Texture{ loadedSurface, pDevice};
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv

		int width = uv.x * m_pSurface->w;
		int height = uv.y * m_pSurface->h;

		SDL_Color finalColor;



		SDL_GetRGB(m_pSurfacePixels[width + height * m_pSurface->w], m_pSurface->format,
			&finalColor.r,
			&finalColor.g,
			&finalColor.b);

		ColorRGB color{ (float)finalColor.r / 255,(float)finalColor.g / 255,(float)finalColor.b / 255 };
		return (color);



	}
}