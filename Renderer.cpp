#include "pch.h"
#include "Renderer.h"

#include "Effect.h"
#include "Material.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
		m_Handle = GetStdHandle(STD_OUTPUT_HANDLE);

		ShowKeybindings();
		//Software
		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	
		int size{ m_Width * m_Height };
		m_ColorBuffer = new ColorRGB[size];
		for (int i{ 0 }; i < size; i++)
		{
			m_ColorBuffer[i] = colors::Gray;
		}
		m_pDepthBufferPixels = new float[size];
		for (int i{ 0 }; i < size; i++)
		{
			m_pDepthBufferPixels[i] = FLT_MAX;
		}
		//Hardware
		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}


		//General
		//Vehicle
		std::vector<Vertex_PosCol>vertices{};

		std::vector<uint32_t> indices{};


		m_pTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png", m_pDevice);
		m_pTextureGloss = Texture::LoadFromFile("Resources/vehicle_gloss.png", m_pDevice);
		m_pTextureNormal = Texture::LoadFromFile("Resources/vehicle_normal.png", m_pDevice);
		m_pTextureSpecular = Texture::LoadFromFile("Resources/vehicle_specular.png", m_pDevice);
		Utils::ParseOBJ("Resources/vehicle.obj",vertices,indices);

		for (Vertex_PosCol& vert : vertices)
		{
			vert.Color = { 1,1,1 };
		}

		m_pVehicleMesh = new Mesh{ m_pDevice, vertices, indices };
		m_pVehicleMesh->indices = indices; // for software
		m_pVehicleMesh->vertices = vertices; // for software
		m_TransMatrix = Matrix::CreateTranslation(0, 0, 50);
		m_RotMatrix = Matrix::CreateRotationZ(0);
		m_ScaleMatrix = Matrix::CreateScale(1, 1, 1);
		m_pVehicleMesh->SetWorldMatrix(m_ScaleMatrix * m_RotMatrix * m_TransMatrix);

		m_pVehicleMesh->m_pEffect->SetMaps(m_pTexture, m_pTextureSpecular, m_pTextureNormal, m_pTextureGloss);


		//Fire

		std::vector<Vertex_PosCol>vertices2{};
		std::vector<uint32_t> indices2{};
		m_pTextureFire = Texture::LoadFromFile("Resources/fireFX_diffuse.png", m_pDevice);
		Utils::ParseOBJ("Resources/fireFX.obj",vertices2,indices2);
		for (Vertex_PosCol& vert2 : vertices2) 
		{
			vert2.Color = { 1,1,1 };
		}
		m_pCombustionMesh = new Mesh{ m_pDevice, vertices2, indices2 };
		m_pCombustionMesh->indices = indices2; // for software
		m_pCombustionMesh->vertices = vertices2; //  for software

		m_pCombustionMesh->SetWorldMatrix(m_ScaleMatrix * m_RotMatrix * m_TransMatrix);
		m_pCombustionMesh->m_pEffect->SetMaps(m_pTextureFire);


		const float screenWidth{ static_cast<float>(m_Width) };
		const float screenHeight{ static_cast<float>(m_Height) };


	
		m_pCamera = new Camera(Vector3{ 0.f, 0.f, 0.f }, 45.f);

		m_pCamera->aspectRatio = screenWidth / screenHeight;
		m_pCamera->worldViewProjMatrix = m_pVehicleMesh->m_WorldMatrix * m_pCamera->viewMatrix * m_pCamera->GetProjectionMatrix();

		//Vehicle
		m_pVehicleMesh->SetMatrix(&m_pCamera->worldViewProjMatrix, &m_pVehicleMesh->m_WorldMatrix, &m_pCamera->invViewMatrix);

		//Fire
		m_pCombustionMesh->SetMatrix(&m_pCamera->worldViewProjMatrix, &m_pCombustionMesh->m_WorldMatrix, &m_pCamera->invViewMatrix);
		m_pCombustionMesh->m_pEffect->ChangeEffect("FlatTechnique");
	}

	Renderer::~Renderer()
	{
		delete m_pVehicleMesh;
		m_pVehicleMesh = nullptr;

		delete m_pCombustionMesh;
		m_pCombustionMesh = nullptr;

		delete m_pCamera;
		m_pCamera = nullptr;

		delete m_pTexture;
		m_pTexture = nullptr;
		delete m_pTextureGloss;
		m_pTextureGloss = nullptr;
		delete m_pTextureNormal;
		m_pTextureNormal = nullptr;
		delete m_pTextureSpecular;
		m_pTextureSpecular = nullptr;
		delete m_pTextureFire;
		m_pTextureFire = nullptr;

		delete[] m_pDepthBufferPixels;
		m_pDepthBufferPixels = nullptr;
		delete[] m_ColorBuffer;
		m_ColorBuffer = nullptr;

		m_pBackBufferPixels = nullptr;
		if (m_pFrontBuffer) {
			SDL_FreeSurface(m_pFrontBuffer);
			m_pFrontBuffer = nullptr;
		}
		if (m_pBackBuffer) {

			SDL_FreeSurface(m_pBackBuffer);
			m_pBackBuffer = nullptr;
		}
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		m_PRenderTargetView->Release();
		m_pRenderTargetBuffer->Release();
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilView->Release();
		m_pSwapChain->Release();
		m_pDevice->Release();

		m_pDefaultState->Release();
		m_pFrontCullState->Release();
		m_pBackCullState->Release();

		m_pPointSample->Release();
		m_pLinearSample->Release();
		m_pAnisotropicSample->Release();
		
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);

		if(m_isRotating)
		{
			m_Rot = pTimer->GetTotal() * (45 * PI / 180);
		}

		m_RotMatrix = Matrix::CreateRotationY(m_Rot);
		m_pVehicleMesh->SetWorldMatrix(m_ScaleMatrix * m_RotMatrix * m_TransMatrix);
		m_pCombustionMesh->SetWorldMatrix(m_ScaleMatrix * m_RotMatrix * m_TransMatrix);
		m_pCamera->worldViewProjMatrix = m_pVehicleMesh->m_WorldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix;

		m_pVehicleMesh->SetMatrix(&m_pCamera->worldViewProjMatrix, &m_pVehicleMesh->m_WorldMatrix, &m_pCamera->invViewMatrix);
		m_pCombustionMesh->SetMatrix(&m_pCamera->worldViewProjMatrix, &m_pCombustionMesh->m_WorldMatrix, &m_pCamera->invViewMatrix);
	}


	void Renderer::Render() const
	{
		if(m_IsUsingHardware)
		{
			RenderHardware();
		} else
		{
			RenderSoftware();
		}
	}

	void Renderer::CycleTecnhique()
	{
		SetConsoleTextAttribute(m_Handle, 14);

		m_IsUsingHardware = !m_IsUsingHardware;
		if (m_IsUsingHardware)
		{
			std::cout << "**(SHARED) Rasterizer Mode = HARDWARE" << std::endl;
		}
		else
		{
			std::cout << "**(SHARED) Rasterizer Mode = SOFTWARE" << std::endl;

		}
	}

	void Renderer::CylceShadingMode()
	{
		if (!m_IsUsingHardware)
		{
			SetConsoleTextAttribute(m_Handle, 5);
			m_ShadingMode == ShadingMode::Combined ?
				m_ShadingMode = ShadingMode(0) :
				m_ShadingMode = ShadingMode(static_cast<int>(m_ShadingMode) + 1);

			switch (m_ShadingMode)
			{
			case ShadingMode::Combined:
				std::cout << "**(SOFTWARE) Shading Mode = COMBINED" << std::endl;
				break;
			case ShadingMode::Specular:
				std::cout << "**(SOFTWARE) Shading Mode = SPECULAR" << std::endl;
				break;
			case ShadingMode::Diffuse:
				std::cout << "**(SOFTWARE) Shading Mode = DIFFUSE" << std::endl;
				break;
			case ShadingMode::ObservedArea:
				std::cout << "**(SOFTWARE) Shading Mode = OBSERVED_AREA" << std::endl;
				break;
			}
		}
		
		
	}

	void Renderer::ToggleRotation()
	{
		SetConsoleTextAttribute(m_Handle, 14);
		m_isRotating = !m_isRotating;
		if (m_isRotating)
		{
			std::cout << "**(SHARED) Rotating ON" << std::endl;
		}
		else
		{
			std::cout << "**(SHARED) Rotating OFF" << std::endl;

		}
	}

	void Renderer::ToggleFireMesh()
	{
		if(m_IsUsingHardware)
		{
			SetConsoleTextAttribute(m_Handle, 2);
			m_IsShowingFire = !m_IsShowingFire;
			if (m_IsShowingFire)
			{
				std::cout << "**(HARDWARE) FireFX ON" << std::endl;
			}
			else
			{
				std::cout << "**(HARDWARE) FireFX OFF" << std::endl;

			}
		}
		
	}

	void Renderer::ToggleNormalMap()
	{
		if(!m_IsUsingHardware)
		{
			SetConsoleTextAttribute(m_Handle, 5);
			m_HasNormalMap = !m_HasNormalMap;
			if (m_HasNormalMap)
			{
				std::cout << "**(SOFTWARE) NormalMap ON" << std::endl;
			}
			else
			{
				std::cout << "**(SOFTWARE) NormalMap OFF" << std::endl;

			}
		}
	}

	void Renderer::CycleCullMode()
	{
		SetConsoleTextAttribute(m_Handle, 14);
		m_RasterState == RasterState::Back ?
			m_RasterState = RasterState(0) :
			m_RasterState = RasterState(static_cast<int>(m_RasterState) + 1);

		switch (m_RasterState)
		{
		case RasterState::None:
			m_pDeviceContext->RSSetState(m_pDefaultState);
			std::cout << "**(SHARED) CullMode = NONE" << std::endl;
			break;
		case RasterState::Front:
			m_pDeviceContext->RSSetState(m_pFrontCullState);
			std::cout << "**(SHARED) CullMode = FRONT" << std::endl;
			break;
		case RasterState::Back:
			m_pDeviceContext->RSSetState(m_pBackCullState);
			std::cout << "**(SHARED) CullMode = BACK" << std::endl;
			break;
		}
	}

	void Renderer::CycleSampler()
	{
		if(m_IsUsingHardware)
		{
			SetConsoleTextAttribute(m_Handle, 2);

			m_SamplerState == SamplerState::Anisotropic ?
				m_SamplerState = SamplerState(0) :
				m_SamplerState = SamplerState(static_cast<int>(m_SamplerState) + 1);

			switch (m_SamplerState)
			{
			case SamplerState::Point:
				m_pVehicleMesh->m_pEffect->SetSampler(m_pPointSample);
				std::cout << "**(HARDWARE) Sampler Filter = POINT" << std::endl;

				break;
			case SamplerState::Linear:
				m_pVehicleMesh->m_pEffect->SetSampler(m_pLinearSample);
				std::cout << "**(HARDWARE) Sampler Filter = LINEAR" << std::endl;

				break;
			case SamplerState::Anisotropic:
				m_pVehicleMesh->m_pEffect->SetSampler(m_pAnisotropicSample);
				std::cout << "**(HARDWARE) Sampler Filter = ANISOTROPIC" << std::endl;

				break;
			}
		}
		

	}

	void Renderer::ToggleUniformColor()
	{
		SetConsoleTextAttribute(m_Handle,14);
		m_IsUniformColor = !m_IsUniformColor;
		if (m_IsUniformColor)
		{
			std::cout << "**(SHARED) Uniform ClearColor ON" << std::endl;
		} else
		{
			std::cout << "**(SHARED) Uniform ClearColor OFF" << std::endl;

		}
			
	}

	void Renderer::ToggleDepthShow()
	{
		if(!m_IsUsingHardware)
		{
			SetConsoleTextAttribute(m_Handle, 5);
			m_IsShowingDepth = !m_IsShowingDepth;
			if (m_IsShowingDepth)
			{
				std::cout << "**(SOFTWARE) DepthBuffer Visualization ON" << std::endl;
			}
			else
			{
				std::cout << "**(SOFTWARE) DepthBuffer Visualization OFF" << std::endl;

			}
		}
	}

	void Renderer::ToggleBoundingBoxShow()
	{
		if(!m_IsUsingHardware)
		{
			SetConsoleTextAttribute(m_Handle, 5);
			m_IsShowingBoundingBox = !m_IsShowingBoundingBox;
			if (m_IsShowingBoundingBox)
			{
				std::cout << "**(SOFTWARE) BoundingBox Visualization ON" << std::endl;
			}
			else
			{
				std::cout << "**(SOFTWARE) BoundingBox Visualization OFF" << std::endl;

			}
		}
	}

	

	void Renderer::ShowKeybindings()
	{
		
		SetConsoleTextAttribute(m_Handle, 14);
		std::cout << "[Key Bindings - SHARED]" << std::endl;
		std::cout << "\t [F1] Toggle Rasterizer Mode (HARDWARE/SOFTWARE)" << std::endl;
		std::cout << "\t [F2] Toggle Vehicle Rotation (ON/OFF)" << std::endl;
		std::cout << "\t [F9] Cycle CullMode (BACK/FRONT/NONE)" << std::endl;
		std::cout << "\t [F10] Toggle Uniform ClearColor (ON/OFF)" << std::endl;
		std::cout << "\t [F11] Toggle Print FPS (ON/OFF)" << std::endl;
		std::cout << std::endl;
		SetConsoleTextAttribute(m_Handle, 2);
		std::cout << "[Key Bindings - HARDWARE]" << std::endl;
		std::cout << "\t [F3] Toggle FireFX (ON/OFF)" << std::endl;
		std::cout << "\t [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)" << std::endl;
		std::cout << std::endl;
		SetConsoleTextAttribute(m_Handle, 5);
		std::cout << "[Key Bindings - SOFTWARE]" << std::endl;
		std::cout << "\t [F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)" << std::endl;
		std::cout << "\t [F6] Toggle NormalMap (ON/OFF)" << std::endl;
		std::cout << "\t [F7] Toggle DepthBuffer Visualization (ON/OFF)" << std::endl;
		std::cout << "\t [F8] Toggle BoundingBox Visualization (ON/OFF)" << std::endl;

	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		//=====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);


		if (FAILED(result))
			return result;

		//Create DXGI factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;
		//2. Create Swapchain
		//=====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMinfo{};
		SDL_VERSION(&sysWMinfo.version)
			SDL_GetWindowWMInfo(m_pWindow, &sysWMinfo);
		swapChainDesc.OutputWindow = sysWMinfo.info.win.window;
		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;
		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;
		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;
		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//=====

		//Resources
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//view
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_PRenderTargetView);
		if (FAILED(result))
			return result;

		//5. Bind RTV & DSV to Ouput Merger Stage
		//=====
		m_pDeviceContext->OMSetRenderTargets(1, &m_PRenderTargetView, m_pDepthStencilView);

		//Extra Rasterizer States
		D3D11_RASTERIZER_DESC defaultCull{};
		defaultCull.FillMode = D3D11_FILL_SOLID; //?
		defaultCull.CullMode = D3D11_CULL_NONE;
		defaultCull.FrontCounterClockwise = false;
		defaultCull.DepthBias = 0;
		defaultCull.SlopeScaledDepthBias = 0.0f;
		defaultCull.DepthBiasClamp = 0.0f;
		defaultCull.DepthClipEnable = true;
		defaultCull.ScissorEnable = false;
		defaultCull.MultisampleEnable = false;
		defaultCull.AntialiasedLineEnable = false;

		result = m_pDevice->CreateRasterizerState(&defaultCull,&m_pDefaultState);
		if (FAILED(result))
			return result;

		D3D11_RASTERIZER_DESC frontCull{};
		frontCull.FillMode = D3D11_FILL_SOLID; //?
		frontCull.CullMode = D3D11_CULL_FRONT;
		frontCull.FrontCounterClockwise = false;
		frontCull.DepthBias = 0;
		frontCull.SlopeScaledDepthBias = 0.0f;
		frontCull.DepthBiasClamp = 0.0f;
		frontCull.DepthClipEnable = true;
		frontCull.ScissorEnable = false;
		frontCull.MultisampleEnable = false;
		frontCull.AntialiasedLineEnable = false;

		result = m_pDevice->CreateRasterizerState(&frontCull, &m_pFrontCullState);
		if (FAILED(result))
			return result;


		D3D11_RASTERIZER_DESC backCull{};
		backCull.FillMode = D3D11_FILL_SOLID; 
		backCull.CullMode = D3D11_CULL_BACK;
		backCull.FrontCounterClockwise = false;
		backCull.DepthBias = 0;
		backCull.SlopeScaledDepthBias = 0.0f;
		backCull.DepthBiasClamp = 0.0f;
		backCull.DepthClipEnable = true;
		backCull.ScissorEnable = false;
		backCull.MultisampleEnable = false;
		backCull.AntialiasedLineEnable = false;

		result = m_pDevice->CreateRasterizerState(&backCull, &m_pBackCullState);
		if (FAILED(result))
			return result;



		//Extra Sampler States
		D3D11_SAMPLER_DESC PointSamp{};
		PointSamp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		PointSamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		PointSamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		PointSamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		PointSamp.MinLOD = -FLT_MAX;
		PointSamp.MaxLOD = FLT_MAX;
		PointSamp.MipLODBias = 0.0f;
		PointSamp.MaxAnisotropy = 1;
		PointSamp.ComparisonFunc = D3D11_COMPARISON_NEVER;
		PointSamp.BorderColor[0] = 1.0f;
		PointSamp.BorderColor[1] = 1.0f;
		PointSamp.BorderColor[2] = 1.0f;
		PointSamp.BorderColor[3] = 1.0f;
		result = m_pDevice->CreateSamplerState(&PointSamp, &m_pPointSample);
		if (FAILED(result))
			return result;


		D3D11_SAMPLER_DESC linearSamp{};
		linearSamp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		linearSamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearSamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearSamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		linearSamp.MinLOD = -FLT_MAX;
		linearSamp.MaxLOD = FLT_MAX;
		linearSamp.MipLODBias = 0.0f;
		linearSamp.MaxAnisotropy = 1;
		linearSamp.ComparisonFunc = D3D11_COMPARISON_NEVER;
		linearSamp.BorderColor[0] = 1.0f;
		linearSamp.BorderColor[1] = 1.0f;
		linearSamp.BorderColor[2] = 1.0f;
		linearSamp.BorderColor[3] = 1.0f;
		result = m_pDevice->CreateSamplerState(&linearSamp, &m_pLinearSample);
		if (FAILED(result))
			return result;


		D3D11_SAMPLER_DESC AnisotropicSamp{};
		AnisotropicSamp.Filter = D3D11_FILTER_ANISOTROPIC;
		AnisotropicSamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		AnisotropicSamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		AnisotropicSamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		AnisotropicSamp.MinLOD = -FLT_MAX;
		AnisotropicSamp.MaxLOD = FLT_MAX;
		AnisotropicSamp.MipLODBias = 0.0f;
		AnisotropicSamp.MaxAnisotropy = 1;
		AnisotropicSamp.ComparisonFunc = D3D11_COMPARISON_NEVER;
		AnisotropicSamp.BorderColor[0] = 1.0f;
		AnisotropicSamp.BorderColor[1] = 1.0f;
		AnisotropicSamp.BorderColor[2] = 1.0f;
		AnisotropicSamp.BorderColor[3] = 1.0f;
		result = m_pDevice->CreateSamplerState(&AnisotropicSamp, &m_pAnisotropicSample);
		if (FAILED(result))
			return result;


		//6. Set Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		//here?
		pDxgiFactory->Release();

		return result;
	}


	void Renderer::RenderSoftware() const
	{
		SDL_LockSurface(m_pBackBuffer);

		//RENDER LOGIC

		//ResetBuffers
		int size{ m_Width * m_Height };
	
		for (int i{ 0 }; i < size; i++)
		{
			if(m_IsUniformColor)
			{
				m_ColorBuffer[i] = m_UniformCol;
			} else
			{
				m_ColorBuffer[i] = m_SoftCol;
			}
		}
	
		for (int i{ 0 }; i < size; i++)
		{
			m_pDepthBufferPixels[i] = FLT_MAX;
		}

		SDL_FillRect(m_pBackBuffer, NULL, 0x000000);
		if(m_IsUniformColor)
		{
			SDL_FillRect(m_pBackBuffer, NULL, 0x191919);

		} else
		{
			SDL_FillRect(m_pBackBuffer, NULL, 0x636363);

		}


	
		for (int i{}; i < m_pVehicleMesh->indices.size(); i += 3)
		{
			std::vector<Vertex_PosCol> triangle{ m_pVehicleMesh->vertices[m_pVehicleMesh->indices[i]],m_pVehicleMesh->vertices[m_pVehicleMesh->indices[i + 1]],m_pVehicleMesh->vertices[m_pVehicleMesh->indices[i + 2]] };

			std::vector<Vertex_PosColOut> totalVertices;
			VertexTransformationFunction(triangle, totalVertices, m_pVehicleMesh->m_WorldMatrix);

			RenderTriangle(totalVertices);
		}

		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}




	void dae::Renderer::RenderTriangle(std::vector<Vertex_PosColOut> newTriangle) const
	{

		Vector2 a = Vector2{ newTriangle[1].Pos.x,newTriangle[1].Pos.y } - Vector2{ newTriangle[0].Pos.x,newTriangle[0].Pos.y };
		Vector2 b = Vector2{ newTriangle[2].Pos.x,newTriangle[2].Pos.y } - Vector2{ newTriangle[1].Pos.x,newTriangle[1].Pos.y };
		Vector2 c = Vector2{ newTriangle[0].Pos.x,newTriangle[0].Pos.y } - Vector2{ newTriangle[2].Pos.x,newTriangle[2].Pos.y };

		Vector2 triangleV1 = { newTriangle[0].Pos.x,newTriangle[0].Pos.y };
		Vector2 triangleV2 = { newTriangle[1].Pos.x,newTriangle[1].Pos.y };
		Vector2 triangleV3 = { newTriangle[2].Pos.x,newTriangle[2].Pos.y };

		Vector2 edge = Vector2{ newTriangle[2].Pos.x,newTriangle[2].Pos.y } - Vector2{ newTriangle[0].Pos.x,newTriangle[0].Pos.y };
		float totalArea = Vector2::Cross(a, edge);

		//BoundingBox
		float minX = std::min(std::min(triangleV1.x, triangleV2.x), triangleV3.x);
		float minY = std::min(std::min(triangleV1.y, triangleV2.y), triangleV3.y);
		float maxX = std::max(std::max(triangleV1.x, triangleV2.x), triangleV3.x);
		float maxY = std::max(std::max(triangleV1.y, triangleV2.y), triangleV3.y);

		if (
			((minX >= 0) && (maxX <= (m_Width - 1))) &&
			((minY >= 0) && (maxY <= (m_Height - 1)))) {
			for (int px{ static_cast<int>(minX) }; px < std::ceil(maxX); ++px)
			{
				for (int py{ static_cast<int>(minY) }; py < std::ceil(maxY); ++py)
				{
					float gradient = px / static_cast<float>(m_Width);
					gradient += py / static_cast<float>(m_Width);
					gradient /= 2.0f;


					Vector2 p{ float(px),float(py) };

					Vector2 pointToSide{ p - triangleV2 };
					float signedArea1{ Vector2::Cross(b, pointToSide) };

					pointToSide = p - triangleV3;
					float signedArea2{ Vector2::Cross(c, pointToSide) };

					pointToSide = p - triangleV1;
					float signedArea3{ Vector2::Cross(a, pointToSide) };

					float W1 = signedArea1 / totalArea;
					float W2 = signedArea2 / totalArea;
					float W3 = signedArea3 / totalArea;
				

					int curPixel = px + (py * m_Width);
					ColorRGB finalColor{};
					
					if(m_IsShowingBoundingBox)
					{
						m_ColorBuffer[curPixel] = ColorRGB{ 1,1,1 };
					} else
					if (W1 > 0.f && W2 > 0.f && W3 > 0.f) {

						//CullingTest
						switch(m_RasterState)
						{
						case RasterState::Back:
							if (Vector3::Dot(newTriangle[0].Normal.Normalized(), newTriangle[0].viewDirection.Normalized()) < 0)
							{
								return;
							}
							break;
						case RasterState::Front:
							if (Vector3::Dot(newTriangle[0].Normal.Normalized(), newTriangle[0].viewDirection.Normalized()) > 0)
							{
								return;
							}
							break;
							default:
								break;
						}

					

						//depth test
						float interpolatedDepth{ 1 / ((1 / newTriangle[0].Pos.z) * W1 + (1 / newTriangle[1].Pos.z) * W2 + (1 / newTriangle[2].Pos.z) * W3) };
						if (interpolatedDepth > m_pDepthBufferPixels[curPixel]) {
							continue;
						}
						m_pDepthBufferPixels[curPixel] = interpolatedDepth;



						//Deciding color
						float interpolatedDepthW{ 1 / ((1 / newTriangle[0].Pos.w) * W1 + (1 / newTriangle[1].Pos.w) * W2 + (1 / newTriangle[2].Pos.w) * W3) };

						Vector2 interpolatedUV{
							(((newTriangle[0].Uv / newTriangle[0].Pos.w) * W1) +
							((newTriangle[1].Uv / newTriangle[1].Pos.w) * W2) +
							((newTriangle[2].Uv / newTriangle[2].Pos.w) * W3)) * interpolatedDepthW };

						//normal interpolating
						Vector3 interpolatedNormal{
							(((newTriangle[0].Normal / newTriangle[0].Pos.w) * W1) +
							((newTriangle[1].Normal / newTriangle[1].Pos.w) * W2) +
							((newTriangle[2].Normal / newTriangle[2].Pos.w) * W3)) * interpolatedDepthW };

						//tangent interpolating
						Vector3 interpolatedTangent{
							(((newTriangle[0].Tangent / newTriangle[0].Pos.w) * W1) +
							((newTriangle[1].Tangent / newTriangle[1].Pos.w) * W2) +
							((newTriangle[2].Tangent / newTriangle[2].Pos.w) * W3)) * interpolatedDepthW };


					
						ColorRGB interpolatedColor{ m_pTexture->Sample(interpolatedUV) };

						//pos interpolated
						Vector4 interpolatedPos{
							((newTriangle[0].Pos * W1) +
							(newTriangle[1].Pos * W2) +
							(newTriangle[2].Pos * W3)) * interpolatedDepthW
						};

						//interpolatedViewDirection;
						Vector3 interpolatedViewDir
						{
							(((newTriangle[0].viewDirection / newTriangle[0].Pos.w) * W1) +
							((newTriangle[1].viewDirection / newTriangle[1].Pos.w) * W2) +
							((newTriangle[2].viewDirection / newTriangle[2].Pos.w) * W3)) * interpolatedDepthW
						};
					
						Vector3 binormal = Vector3::Cross(interpolatedNormal, interpolatedTangent);
						Matrix tangentSpaceAxis = Matrix{ interpolatedTangent,binormal,interpolatedNormal,Vector3::Zero };
						


						ColorRGB interpolatedNormalMap{ m_pTextureNormal->Sample(interpolatedUV) };
						Vector3 normalVec{ interpolatedNormalMap.r,interpolatedNormalMap.g,interpolatedNormalMap.b };
						//multiply with matrix
						normalVec = { 2.f * normalVec.x - 1.f, 2.f * normalVec.y - 1.f, 2.f * normalVec.z - 1.f };
						normalVec = tangentSpaceAxis.TransformVector(normalVec);
						normalVec /= 255.f;
					


						Vertex_PosColOut interpolatedV = { interpolatedPos,Vector3(interpolatedColor.r,interpolatedColor.g,interpolatedColor.b),interpolatedUV,m_HasNormalMap ? normalVec.Normalized() : interpolatedNormal,interpolatedTangent,interpolatedViewDir };


						//Get Specular and gloss from maps
						ColorRGB glos = m_pTextureGloss->Sample(interpolatedUV);
						ColorRGB spec = m_pTextureSpecular->Sample(interpolatedUV);
						Vector3 glosVec{ glos.r,glos.g,glos.b };
						Vector3 specVec{ spec.r,spec.g,spec.b };


					
						m_ColorBuffer[curPixel] = PixelShading(interpolatedV, specVec.x , glosVec.x );

						if(m_IsShowingDepth)
						{
							float d = static_cast<float>((2.0 * m_pCamera->m_NearPlane) / (m_pCamera->m_FarPlane + m_pCamera->m_NearPlane - interpolatedDepth * (m_pCamera->m_FarPlane - m_pCamera->m_NearPlane)));
							m_ColorBuffer[curPixel] = ColorRGB{ d,d,d };
						}
					}

					finalColor = m_ColorBuffer[curPixel];
					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		} 
	}

	void Renderer::VertexTransformationFunction(const std::vector<Vertex_PosCol>& vertices_in, std::vector<Vertex_PosColOut>& vertices_out, Matrix worldMatrix) const
	{
		vertices_out.clear();
		float aspectRatio = { m_Width / static_cast<float>(m_Height) };
		Matrix end = worldMatrix * m_pCamera->viewMatrix * m_pCamera->projectionMatrix;
		for (int i{}; i < vertices_in.size(); i++)
		{
			Vertex_PosCol pWorld{};
			Vertex_PosColOut p{};
			//To view Space
		
			pWorld = vertices_in[i];
			Vector4 pos{ pWorld.Pos.x,pWorld.Pos.y,pWorld.Pos.z,1 };
			Vertex_PosColOut pView{};

			pView.Pos = end.TransformPoint(pos);


			//Perspective Divide
			p.Pos.x = pView.Pos.x / pView.Pos.w;
			p.Pos.y = pView.Pos.y / pView.Pos.w;
			p.Pos.z = pView.Pos.z / pView.Pos.w;
			p.Pos.w = pView.Pos.w;

		
			p.Pos.x = ((p.Pos.x + 1) / 2) * m_Width;
			p.Pos.y = ((1 - p.Pos.y) / 2) * m_Height;
			p.Color = vertices_in[i].Color;
			p.Uv = vertices_in[i].Uv;
		

			p.Normal = worldMatrix.TransformVector(vertices_in[i].Normal);
			p.Tangent = worldMatrix.TransformVector(vertices_in[i].Tangent);

			//calculateViewDirectionToo
			p.viewDirection = Vector3{ m_pCamera->origin - pView.Pos };
			vertices_out.push_back(p);
		}
	}


	void Renderer::RenderHardware() const
	{
		if (!m_IsInitialized)
			return;
		//1. CLEAR RTV & DSV
		ColorRGB clearColor;
		if(m_IsUniformColor)
		{
			clearColor = m_UniformCol;
		} else
		{
			clearColor = m_HardwareCol;
		}
		m_pDeviceContext->ClearRenderTargetView(m_PRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		//...
		//Update rasterstate to make up for overwrited state from .fx for flames
		switch (m_RasterState)
		{
		case RasterState::None:
			m_pDeviceContext->RSSetState(m_pDefaultState);
			break;
		case RasterState::Front:
			m_pDeviceContext->RSSetState(m_pFrontCullState);
			break;
		case RasterState::Back:
			m_pDeviceContext->RSSetState(m_pBackCullState);
			break;
		}

		m_pVehicleMesh->Render(m_pDeviceContext);
		if(m_IsShowingFire)
		{
			//will set rasterstate back to none because gets overwritten in fx file
		m_pCombustionMesh->Render(m_pDeviceContext);
		}

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	ColorRGB Renderer::PixelShading(const Vertex_PosColOut& v, float spec, float glos) const
	{
		constexpr float kd{ 7.f }; // = diffuse reflectance / intensity

		constexpr float shininess{ 25.f };

		ColorRGB ambient{ .025f, .025f, .025f };
		Vector3 lightDirection = { .577f, -.577f, .577f };
	

		//Lambert
		float ObservedArea{ Vector3::Dot(v.Normal.Normalized(), -lightDirection.Normalized()) };
		ObservedArea = Clamp(ObservedArea, 0.f, 1.f);




		switch (m_ShadingMode)
		{
		case ShadingMode::ObservedArea: {
			return ColorRGB{ ObservedArea,ObservedArea,ObservedArea };
			}

		case ShadingMode::Diffuse: {
			Material_Lambert material{ Material_Lambert(ColorRGB(v.Color.x,v.Color.y,v.Color.z), kd) };
			const ColorRGB diffuse{ material.Shade(v) * ObservedArea };
			return diffuse;
			}

		case ShadingMode::Specular: {
			const ColorRGB specular{ BRDF::Phong(spec , glos * shininess , lightDirection.Normalized(), v.viewDirection.Normalized(), v.Normal.Normalized()) };

			return (specular * ObservedArea);
			}

		case ShadingMode::Combined: {
			Material_Lambert material{ Material_Lambert(ColorRGB(v.Color.x,v.Color.y,v.Color.z), kd) };

			const ColorRGB diffuse{ material.Shade(v)};
			const ColorRGB specular{ BRDF::Phong(spec, glos * shininess, lightDirection.Normalized(), v.viewDirection.Normalized(), v.Normal.Normalized()) };

			const ColorRGB phong{ diffuse + specular };
			
			return ambient + phong * ObservedArea;
			
			}

		default: {
		
			Material_Lambert material{ Material_Lambert(ColorRGB(v.Color.x,v.Color.y,v.Color.z), kd) };

			const ColorRGB diffuse{ material.Shade(v) };
			const ColorRGB specular{ BRDF::Phong(spec, glos * shininess, lightDirection.Normalized(), v.viewDirection.Normalized(), v.Normal.Normalized()) };

			const ColorRGB phong{ diffuse + specular };

			return ambient + phong * ObservedArea;
			}
		}

	}


}




