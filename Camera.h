#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		//Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		
		const float movementSpeed{ 15.f };
		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		Vector3 rightLocal{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

	/*	Matrix invViewMatrix{};
		Matrix viewMatrix{};*/
		Matrix projectionMatrix{};

		float rotspeed{ 0.005f };

		float aspectRatio{ 0.f };
		Matrix worldViewProjectionMatrix{};
		Matrix invViewMatrix{};
		Matrix onbMatrix{};
		Matrix viewMatrix{};
		Vector3 origin{ 0,0,0 };

		const float m_NearPlane = 0.1f;
		const float m_FarPlane = 100.f;

	/*	void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
		}*/

		void CalculateViewMatrix()
		{
			//TODO W1
		//first obn
			Vector3 right = Vector3::Cross(up, forward).Normalized();
			Vector4 rightMatrix{ right,0 };
			Vector3 up = Vector3::Cross(forward, right).Normalized();
			Vector4 upMatrix{ up,0 };
			Vector4 forwardMatrix{ forward,0 };
			Vector4 positionMatrix{ origin,1 };

			Matrix obn{ rightMatrix,upMatrix,forwardMatrix,positionMatrix };
			Matrix invView = Matrix::Inverse(obn);

			invViewMatrix = obn;
			viewMatrix = invView;
			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
			invViewMatrix = viewMatrix.Inverse();
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, m_NearPlane, m_FarPlane);
		};

		Matrix GetViewMatrix()
		{
	

			return viewMatrix;

			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		Matrix GetProjectionMatrix()
		{
			//TODO W2
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
			return projectionMatrix;
		}

		void Update(const Timer* pTimer)
		{
			/*const Vector3 forwardSpeed{ forward * pTimer->GetElapsed() * movementSpeed };
			const Vector3 sideSpeed{ right * pTimer->GetElapsed() * movementSpeed };
			const Vector3 upSpeed{ up * pTimer->GetElapsed() * movementSpeed };*/

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
		/*	origin += pKeyboardState[SDL_SCANCODE_W] * forwardSpeed;
			origin -= pKeyboardState[SDL_SCANCODE_S] * forwardSpeed;

			origin += pKeyboardState[SDL_SCANCODE_SPACE] * upSpeed;
			origin -= pKeyboardState[SDL_SCANCODE_LCTRL] * upSpeed;

			origin += pKeyboardState[SDL_SCANCODE_D] * sideSpeed;
			origin -= pKeyboardState[SDL_SCANCODE_A] * sideSpeed;*/
			if (pKeyboardState[SDL_SCANCODE_W])
			{

				origin += forward * pTimer->GetElapsed() * movementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{

				origin -= forward * pTimer->GetElapsed() * movementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{

				origin -= rightLocal * pTimer->GetElapsed() * movementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{

				origin += rightLocal * pTimer->GetElapsed() * movementSpeed;
			}


			//MouseInput
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//Middle mouse is 2
			//Right mouse is 8

			//MovementMouse
			if (SDL_BUTTON(mouseState) == 1)
			{
				//forward movement
				origin -= forward * float(mouseY) * pTimer->GetElapsed() * movementSpeed *5.f;
				//CameraMovement
				totalYaw -= mouseX * rotspeed;
			}

			if (SDL_BUTTON(mouseState) == 16)
			{
				origin.y -= mouseY * pTimer->GetElapsed() * 100.f;
			}

			//RotationMouse
			if (SDL_BUTTON(mouseState) == 8)
			{


				totalYaw += mouseX * rotspeed;
				totalPitch -= mouseY * rotspeed;
			}


			//Update After rotating
			Matrix finalRot = Matrix::CreateRotation(totalPitch, totalYaw, 0);
			forward = finalRot.TransformVector(Vector3::UnitZ);
			forward.Normalize();

			rightLocal = finalRot.TransformVector(Vector3::UnitX);
			rightLocal.Normalize();

			//General

			CalculateViewMatrix();
			CalculateProjectionMatrix();
		}
	};
}
