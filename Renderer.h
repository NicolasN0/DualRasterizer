#pragma once

struct SDL_Window;
struct SDL_Surface;
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
namespace dae
{
    enum class ShadingMode {
        ObservedArea,
        Diffuse,
        Specular,
        Combined
    };


	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;
        void CycleTecnhique();
        void CylceShadingMode();
        void ToggleRotation() { m_isRotating = !m_isRotating; }
	private:
        SDL_Window* m_pWindow{};

        int m_Width{};
        int m_Height{};
        float m_Rot{ 0 };

        bool m_IsInitialized{ false };
        bool m_IsUsingHardware{true};
        ShadingMode m_ShadingMode{};
        Camera* m_pCamera{ nullptr };

        ID3D11Device* m_pDevice{ nullptr };
        ID3D11DeviceContext* m_pDeviceContext{ nullptr };
        IDXGISwapChain* m_pSwapChain{ nullptr };
        ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
        ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
        ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
        ID3D11RenderTargetView* m_PRenderTargetView{ nullptr };

        Mesh* m_pCombustionMesh{ nullptr };
        Mesh* m_pMesh{ nullptr };
        Texture* m_pTexture{ nullptr };
        Texture* m_pTextureGloss{ nullptr };
        Texture* m_pTextureNormal{ nullptr };
        Texture* m_pTextureSpecular{ nullptr };

        Texture* m_pTextureFire{ nullptr };

        Matrix m_TransMatrix{};
        Matrix m_RotMatrix{};
        Matrix m_ScaleMatrix{};

		//DIRECTX
		HRESULT InitializeDirectX();
        void RenderHardware() const;
        //Software
        bool m_isRotating{true};
        bool m_HasNormalMap;
       
        void ToggleNormalMap() { m_HasNormalMap = !m_HasNormalMap; }

        SDL_Surface* m_pFrontBuffer{ nullptr };
        SDL_Surface* m_pBackBuffer{ nullptr };
        uint32_t* m_pBackBufferPixels{};

        float* m_pDepthBufferPixels{};
        ColorRGB* m_ColorBuffer;
        void RenderSoftware() const;

        void RenderTriangle(std::vector<Vertex_PosColOut> newTriangle) const;
        void VertexTransformationFunction(const std::vector<Vertex_PosCol>& vertices_in, std::vector<Vertex_PosColOut>& vertices_out, Matrix worldMatrix) const; //W1 Version
        ColorRGB PixelShading(const Vertex_PosColOut& v, float spec, float glos) const;

		//...
	};
}
