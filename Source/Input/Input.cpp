#include "Input.h"
#include "Graphics/Model.h"
#include "Camera.h"
#include "COLLISION.h"

#pragma region KEYSTATE

void INPUTMANAGER::KEYSTATE::CheckLastState()
{
	if (last_code != code && code != 0)
	{
		last_code = code;
		triggered = true;
	}
	else
		triggered = false;
}
void INPUTMANAGER::KEYSTATE::CheckReleasedState()
{
	released = code == 0 && last_code != 0 ? true : false;
	last_code = released ? 0 : last_code;
}
bool INPUTMANAGER::KEYSTATE::Held()
{
	return held;
}
bool INPUTMANAGER::KEYSTATE::Released()
{
	return released;
}
bool INPUTMANAGER::KEYSTATE::Triggered()
{
	return triggered;
}

#pragma endregion
#pragma region KEYBOARD

void INPUTMANAGER::KEYBOARD::Execute()
{
#pragma region KEYSTATE SETTINGS

	for (auto& k : Keys)
	{
		if (k.last_code != k.code && k.code != 0)
		{
			k.triggered = true;
			k.last_code = k.code;
		}
		else
			k.triggered = false;
		if (k.code == 0 && k.last_code != k.code)
		{
			k.released = true;
			k.last_code = k.code;
		}
		else
			k.released = false;
		k.last_code = k.code;
	}

#pragma endregion

#pragma region AXISES

	if (Held('A') || Held(VK_LEFT))
		axisX.x = -1;
	else if (Held('D') || Held(VK_RIGHT))
		axisX.x = 1;
	else
		axisX.x = 0;

	// Axis X Y Axle
	if (Held('W') || Held(VK_UP))
		axisX.y = 1;
	else if (Held('S') || Held(VK_DOWN))
		axisX.y = -1;
	else
		axisX.y = 0;

	// Axis Y X Axle
	if (Held('J'))
		axisY.x = -1;
	else if (Held('L'))
		axisY.x = 1;
	else
		axisY.x = 0;

	// Axis Y Y Axle
	if (Held('I'))
		axisY.y = 1;
	else if (Held('K'))
		axisY.y = -1;
	else
		axisY.y = 0;
	if (axisX.x || axisX.y)
	{
		float l = sqrtf(axisX.x * axisX.x + axisX.y * axisX.y);
		axisX.x /= l;
		axisX.y /= l;
	}

	if (axisY.x || axisY.y)
	{
		float l = sqrtf(axisY.x * axisY.x + axisY.y * axisY.y);
		axisY.x /= l;
		axisY.y /= l;
	}

#pragma endregion
}

void INPUTMANAGER::KEYBOARD::KeyDown(unsigned int k)
{
	Keys[k].held = true;
	Keys[k].code = k;
}
void INPUTMANAGER::KEYBOARD::KeyUp(unsigned int k)
{
	Keys[k].released = true;
	Keys[k].held = false;
	Keys[k].code = 0;
}
bool INPUTMANAGER::KEYBOARD::Held(unsigned int k)
{
	return Keys[k].Held();
}
bool INPUTMANAGER::KEYBOARD::Released(unsigned int k)
{
	return Keys[k].Released();
}
bool INPUTMANAGER::KEYBOARD::Triggered(unsigned int k)
{
	return Keys[k].Triggered();
}
INPUTMANAGER::KEYSTATE INPUTMANAGER::KEYBOARD::KeyState(unsigned int k)
{
	return Keys[k];
}
XMFLOAT2 INPUTMANAGER::KEYBOARD::AxisX()
{
	return axisX;
}
XMFLOAT2 INPUTMANAGER::KEYBOARD::AxisY()
{
	return axisY;
}

#pragma endregion
#pragma region MOUSE

INPUTMANAGER::KEYSTATE INPUTMANAGER::MOUSE::MOUSE_WHEEL::Up()
{
	return up;
}
INPUTMANAGER::KEYSTATE INPUTMANAGER::MOUSE::MOUSE_WHEEL::Down()
{
	return down;
}
void INPUTMANAGER::MOUSE::OnPressLButton()
{
	lButton.held = true;
	lButton.code = WM_LBUTTONDOWN;
}
void INPUTMANAGER::MOUSE::OnReleaseLButton()
{
	lButton.released = true;
	lButton.held = false;
	lButton.code = 0;
}
void INPUTMANAGER::MOUSE::OnPressRButton()
{
	rButton.held = true;
	rButton.code = WM_RBUTTONDOWN;
}
void INPUTMANAGER::MOUSE::OnReleaseRButton()
{
	rButton.released = true;
	rButton.held = false;
	rButton.code = 0;
}
void INPUTMANAGER::MOUSE::OnPressMButton()
{
	mButton.held = true;
	mButton.code = WM_MBUTTONDOWN;
}
void INPUTMANAGER::MOUSE::OnReleaseMButton()
{
	mButton.released = true;
	mButton.held = false;
	mButton.code = 0;
}
void INPUTMANAGER::MOUSE::OnMousewheelUp()
{
	wheel.up.held = true;
}
void INPUTMANAGER::MOUSE::OnMouseWheelDown()
{
	wheel.down.held = true;
}
void INPUTMANAGER::MOUSE::ResetState()
{
	wheel.up.held = wheel.down.held = false;
	lButton.CheckReleasedState();
	rButton.CheckReleasedState();
	mButton.CheckReleasedState();
	lButton.CheckLastState();
	rButton.CheckLastState();
	mButton.CheckLastState();
}
INPUTMANAGER::KEYSTATE INPUTMANAGER::MOUSE::LButton()
{
	return lButton;
}
INPUTMANAGER::KEYSTATE INPUTMANAGER::MOUSE::RButton()
{
	return rButton;
}
INPUTMANAGER::KEYSTATE INPUTMANAGER::MOUSE::MButton()
{
	return mButton;
}
INPUTMANAGER::MOUSE::MOUSE_WHEEL INPUTMANAGER::MOUSE::Wheel()
{
	return wheel;
}
XMFLOAT2 INPUTMANAGER::MOUSE::fPosition()
{
	return { (float)position.x, (float)position.y };
}
void INPUTMANAGER::MOUSE::SetPosition(int x, int y)
{
	position = { x ,y };
}

#pragma endregion
#pragma region ALTKEY

void INPUTMANAGER::ALTKEY::OnPress()
{
	state.held = true;
}
void INPUTMANAGER::ALTKEY::OnRelease()
{
	state.held = false;
	state.released = true;
}
INPUTMANAGER::KEYSTATE INPUTMANAGER::ALTKEY::State()
{
	return state;
}

#pragma endregion
#pragma region INPUTMANAGER

INPUTMANAGER::INPUTMANAGER()
{
	keyboard = std::make_shared<KEYBOARD>();
	mouse = std::make_shared<MOUSE>();
	altKeys = std::make_shared<ALTKEY>();
};
void INPUTMANAGER::Initialize()
{

}
void INPUTMANAGER::Execute()
{
	ResetState();
	keyboard->Execute();
}
void INPUTMANAGER::ResetState()
{
	mouse->ResetState();
}
void INPUTMANAGER::DragMousePosition(XMFLOAT2* v, KEYSTATE* k)
{
	bool start{};
	static XMFLOAT2 pos, clicked_pos;
	if (k->Triggered())
	{
		clicked_pos.x += Mouse()->fPosition().x;
		clicked_pos.y += Mouse()->fPosition().y;
		start = true;
	}
	if (k->Held() && start)
	{
		*v = { Mouse()->fPosition().x - clicked_pos.x,Mouse()->fPosition().y - clicked_pos.y };
		clicked_pos = Mouse()->fPosition();
	}
	if (k->Released())
	{
		clicked_pos = {};
		start = false;
	}
}
bool INPUTMANAGER::MouseRayCast(Model* m, D3D11_VIEWPORT vp)
{
	XMFLOAT3 m_pos;
	XMFLOAT3 start, end;
	m_pos = { mouse->fPosition().x, mouse->fPosition().y, 0.0f };
	XMStoreFloat3(&start, (XMVector3Unproject(XMLoadFloat3(&m_pos), vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height, vp.MinDepth, vp.MaxDepth, DirectX11::Instance()->ProjectionMatrix(), Camera::Instance()->ViewMatrix(), m->TransformMatrix())));
	m_pos.z = 1.0f;
	XMStoreFloat3(&end, (XMVector3Unproject(XMLoadFloat3(&m_pos), vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height, vp.MinDepth, vp.MaxDepth, DirectX11::Instance()->ProjectionMatrix(), Camera::Instance()->ViewMatrix(), m->TransformMatrix())));
	COLLIDERS::RAYCASTDATA hr{};
	if (COLLIDERS::RayCast(start, end, m, hr, -1))
		return true;
	return false;
}
std::shared_ptr<INPUTMANAGER::KEYBOARD> INPUTMANAGER::Keyboard()
{
	return keyboard;
}
std::shared_ptr<INPUTMANAGER::MOUSE> INPUTMANAGER::Mouse()
{
	return mouse;
}
std::shared_ptr<INPUTMANAGER::ALTKEY>INPUTMANAGER::AltKeys()
{
	return altKeys;
}
#pragma endregion