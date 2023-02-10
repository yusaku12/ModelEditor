#pragma once

#include <memory>
#include "Singleton.h"
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d11.h>

using namespace DirectX;
class Model;

class INPUTMANAGER : public Singleton<INPUTMANAGER>
{
public:
	class KEYBOARD;
	class MOUSE;
	class ALTKEY;
	class KEYSTATE
	{
		friend class KEYBOARD;
		friend class MOUSE;
		friend class ALTKEY;
		bool held{};				// Returns true if the key is pressed
		bool triggered{};
		bool released{};			// Returns true if the key is released
		long code{};
		long last_code{};
	public:
		void CheckLastState();
		void CheckReleasedState();
		bool Held();
		bool Triggered();
		bool Released();
	};
	class KEYBOARD
	{
		KEYSTATE Keys[256]{};
		XMFLOAT2 axisX{}, axisY{};

	public:
		KEYBOARD() {};
		void Execute();

		void KeyDown(unsigned int k);
		void KeyUp(unsigned int k);

		bool Held(unsigned int k);
		bool Released(unsigned int k);
		bool Triggered(unsigned int k);

		KEYSTATE KeyState(unsigned int k);
		XMFLOAT2 AxisX();
		XMFLOAT2 AxisY();
	};
	class MOUSE
	{
		struct POSITION
		{
			int x{}, y{};
		}position;
		class MOUSE_WHEEL
		{
			friend class MOUSE;
			KEYSTATE up, down;
		public:
			KEYSTATE Up();
			KEYSTATE Down();
		}wheel;
		KEYSTATE lButton{}, rButton{}, mButton{};

	public:
		MOUSE() {};

		void OnPressLButton();
		void OnReleaseLButton();

		void OnPressRButton();
		void OnReleaseRButton();

		void OnPressMButton();
		void OnReleaseMButton();

		void OnMousewheelUp();
		void OnMouseWheelDown();

		void ResetState();

		KEYSTATE LButton();
		KEYSTATE RButton();
		KEYSTATE MButton();

		KEYSTATE* pLButton() { return &lButton; }
		KEYSTATE* pRButton() { return &rButton; }
		KEYSTATE* pMButton() { return &mButton; }

		MOUSE_WHEEL Wheel();
		XMFLOAT2 fPosition();
		void SetPosition(int x, int y);
	};
	class ALTKEY
	{
		KEYSTATE state;
	public:
		ALTKEY() {}
		void OnPress();
		void OnRelease();
		KEYSTATE State();
	};

private:
	std::shared_ptr<KEYBOARD>keyboard;
	std::shared_ptr<MOUSE>mouse;
	std::shared_ptr<ALTKEY>altKeys;

public:

	INPUTMANAGER();

	void Initialize();
	void Execute();
	void ResetState();
	void DragMousePosition(XMFLOAT2* v, KEYSTATE* k);
	bool MouseRayCast(Model* m, D3D11_VIEWPORT vp);

	std::shared_ptr<KEYBOARD> Keyboard();
	std::shared_ptr<MOUSE> Mouse();
	std::shared_ptr<ALTKEY>AltKeys();
};

namespace InputController
{
	enum class MBS
	{
		LB, RB, MB
	};
	inline bool Pressed(unsigned int key)
	{
		return INPUTMANAGER::Instance()->Keyboard()->Held(key);
	}
	inline bool Triggered(unsigned int key)
	{
		return INPUTMANAGER::Instance()->Keyboard()->Triggered(key);
	}
	inline bool Released(unsigned int key)
	{
		INPUTMANAGER::Instance()->Keyboard()->Released(key);
	}
	inline bool Clicked(MBS mouseButton)
	{
		switch (mouseButton)
		{
		case MBS::LB:
			return INPUTMANAGER::Instance()->Mouse()->LButton().Held();
		case MBS::RB:
			return INPUTMANAGER::Instance()->Mouse()->RButton().Held();
		case MBS::MB:
			return INPUTMANAGER::Instance()->Mouse()->MButton().Held();
		}
		return false;
	}
	inline bool Triggered(MBS mouseButton)
	{
		switch (mouseButton)
		{
		case MBS::LB:
			return INPUTMANAGER::Instance()->Mouse()->LButton().Triggered();
		case MBS::RB:
			return INPUTMANAGER::Instance()->Mouse()->RButton().Triggered();
		case MBS::MB:
			return INPUTMANAGER::Instance()->Mouse()->MButton().Triggered();
		}
		return false;
	}
	inline bool Released(MBS mouseButton)
	{
		switch (mouseButton)
		{
		case MBS::LB:
			return INPUTMANAGER::Instance()->Mouse()->LButton().Released();
		case MBS::RB:
			return INPUTMANAGER::Instance()->Mouse()->RButton().Released();
		case MBS::MB:
			return INPUTMANAGER::Instance()->Mouse()->MButton().Released();
		}
		return false;
	}
}