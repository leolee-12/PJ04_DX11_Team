#pragma once

#include "GameContent_Defines.h"
#include "Component.h"
#include "UI_Structs.h"

NS_BEGIN(Engine)
class CUIPartObject;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CUIAnimatorCom final : public CComponent
{
public:
	using UI_BOUNCE_DESC = Client::UI_BOUNCE_DESC;
	using UI_FADE_DESC = Client::UI_FADE_DESC;

private:
	typedef struct tagUIBounceRuntime
	{
		CUIPartObject* pTarget = { nullptr };
		_float2 vBasePos = {};
		_float  fBaseZ = { 1.f };
		UI_BOUNCE_DESC Desc = {};
		_float  fElapsed = { 0.f };
	} UI_BOUNCE_RUNTIME;

	typedef struct tagUIFadeRuntime
	{
		CUIPartObject* pTarget = { nullptr };
		UI_FADE_DESC Desc = {};
		_float fBaseAlpha = { 1.f };
		_float fFromAlpha = { 1.f };
		_float fElapsed = { 0.f };
	} UI_FADE_RUNTIME;

public:
	CUIAnimatorCom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIAnimatorCom(const CUIAnimatorCom& Prototype);
	virtual ~CUIAnimatorCom() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	void Update(_float fTimeDelta);

public:
	void Bind_Parts(const unordered_map<_wstring, CUIPartObject*>& Parts);
	void Clear_Parts();

	HRESULT Play_Bounce(const _wstring& strPartTag, const UI_BOUNCE_DESC& Desc);
	HRESULT Play_Bounce(CUIPartObject* pPart, const UI_BOUNCE_DESC& Desc);
	void Stop_Bounce(const _wstring& strPartTag, _bool bRestoreBase = true);
	void Stop_Bounce(CUIPartObject* pPart, _bool bRestoreBase = true);
	void Stop_AllBounces(_bool bRestoreBase = true);
	_bool Is_Bouncing(const _wstring& strPartTag) const;
	_bool Is_Bouncing(CUIPartObject* pPart) const;

	HRESULT Play_Fade(const _wstring& strPartTag, const UI_FADE_DESC& Desc);
	HRESULT Play_Fade(CUIPartObject* pPart, const UI_FADE_DESC& Desc);
	void Play_FadeAll(const UI_FADE_DESC& Desc);
	void Stop_Fade(const _wstring& strPartTag, _bool bRestoreBase = false);
	void Stop_Fade(CUIPartObject* pPart, _bool bRestoreBase = false);
	void Stop_AllFades(_bool bRestoreBase = false);
	_bool Is_Fading(const _wstring& strPartTag) const;
	_bool Is_Fading(CUIPartObject* pPart) const;
	_bool Is_FadingAny() const;

private:
	CUIPartObject* Find_Part(const _wstring& strPartTag) const;
	void Restore_BounceBase(const UI_BOUNCE_RUNTIME& Runtime);
	void Restore_FadeBase(const UI_FADE_RUNTIME& Runtime);
	_bool Try_GetAlpha(CUIPartObject* pPart, _float* pOutAlpha) const;
	_bool Try_SetAlpha(CUIPartObject* pPart, _float fAlpha) const;

private:
	unordered_map<_wstring, CUIPartObject*> m_Parts;
	unordered_map<CUIPartObject*, UI_BOUNCE_RUNTIME> m_BounceRuntimes;
	unordered_map<CUIPartObject*, UI_FADE_RUNTIME> m_FadeRuntimes;

public:
	static CUIAnimatorCom* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

class CLIENT_DLL IUIAnimatorOwner
{
public:
	virtual ~IUIAnimatorOwner() = default;
	virtual CUIAnimatorCom* Get_UIAnimatorCom() = 0;
	virtual const CUIAnimatorCom* Get_UIAnimatorCom() const = 0;
};

NS_END
