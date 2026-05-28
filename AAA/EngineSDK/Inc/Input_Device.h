#ifndef InputDev_h__
#define InputDev_h__

#include "Base.h"

NS_BEGIN(Engine)

class  CInput_Device final : public CBase
{
private:
	CInput_Device(void);
	virtual ~CInput_Device(void) = default;
	
public:
	_byte	Get_DIKeyState(_ubyte byKeyID)		
	{ 
		if (!m_bActive)
			return NULL;

		return m_byKeyState[byKeyID]; 
	}

	_byte	Get_DIMouseState(DIMB eMouse)
	{ 	
		if (!m_bActive)
			return NULL;

		return m_tMouseState.rgbButtons[static_cast<_uint>(eMouse)]; 	
	}

	// 현재 마우스의 특정 축 좌표를 반환
	_long	Get_DIMouseMove(DIMM eMouseState)
	{	
		if (!m_bActive)
			return NULL;

		return *((reinterpret_cast<_long*>(&m_tMouseState)) 
			+ static_cast<_uint>(eMouseState));
	}
public:
	// 키보드
	_bool Key_Down(_ubyte byKeyID) {
		if (!m_bActive)
			return false;

		return (m_byKeyState[byKeyID] & 0x80) && !(m_byPrevKeyState[byKeyID] & 0x80);
	}
	_bool Key_Up(_ubyte byKeyID) {
		if (!m_bActive)
			return false;

		return !(m_byKeyState[byKeyID] & 0x80) && (m_byPrevKeyState[byKeyID] & 0x80);
	}
	_bool Key_Pressing(_ubyte byKeyID) {
		if (!m_bActive)
			return false;

		return (m_byKeyState[byKeyID] & 0x80);
	}

	// 마우스
	_bool Mouse_Down(DIMB eMouse) {
		if (!m_bActive)
			return false;

		return (m_tMouseState.rgbButtons[(_uint)eMouse] & 0x80) && !(m_tPrevMouseState.rgbButtons[(_uint)eMouse] & 0x80);
	}
	_bool Mouse_Up(DIMB eMouse) {
		if (!m_bActive)
			return false;

		return !(m_tMouseState.rgbButtons[(_uint)eMouse] & 0x80) && (m_tPrevMouseState.rgbButtons[(_uint)eMouse] & 0x80);
	}
	_bool Mouse_Pressing(DIMB eMouse) {
		if (!m_bActive)
			return false;

		return (m_tMouseState.rgbButtons[(_uint)eMouse] & 0x80);
	}

	POINT Get_MousePos() {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(m_hWnd, &pt);
		return pt;
	}

public: 
	void Disable_InputDeveice() { m_bActive = false; }
	void Enable_InputDeveice() { m_bActive = true; }
	
public:
	HRESULT Initialize(HINSTANCE hInst, HWND hWnd);
	void	Update(void);

private:
	LPDIRECTINPUT8			m_pInputSDK = { nullptr };

private:
	LPDIRECTINPUTDEVICE8	m_pKeyBoard = { nullptr };
	LPDIRECTINPUTDEVICE8	m_pMouse = { nullptr };

private:
	_bool					m_bActive = false;		
	_byte					m_byKeyState[256] = {};		// 키보드에 있는 모든 키값을 저장하기 위한 변수
	_byte					m_byPrevKeyState[256] = {};
	DIMOUSESTATE			m_tMouseState = {};
	DIMOUSESTATE			m_tPrevMouseState = {};
	HWND					m_hWnd = {};

public:
	static CInput_Device* Create(HINSTANCE hInstance, HWND hWnd);
	virtual void Free(void);

};
NS_END

#endif // InputDev_h__


