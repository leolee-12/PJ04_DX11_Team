#pragma once
#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CLensFlare final : public CEffect_Container
{
	GENERATED_BODY(CLensFlare)

	PROPERTY(_bool, m_bUseScreenAxis, L"Use Screen Axis", L"LensFlare");
	PROPERTY(_float, m_fAxisExtent, L"Axis Extent", L"LensFlare");
	PROPERTY(_float, m_fGhostViewDepth, L"Ghost View Depth", L"LensFlare");
	PROPERTY(_float, m_fScreenShowMargin, L"Screen Show Margin", L"LensFlare");
	PROPERTY(_float, m_fScreenHideMargin, L"Screen Hide Margin", L"LensFlare");
	PROPERTY(_float, m_fAxisRotationDegree, L"Axis Rotation Degree", L"LensFlare");

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LensFlare";

private:
	CLensFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLensFlare(const CLensFlare& Prototype);
	virtual ~CLensFlare() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	On_Deserialized() override;

public:
	virtual void    Priority_Update(_float fTimeDelta) override;
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual json	Serialize() const override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	_bool Is_Playing() const { return m_bIsPlay; }
	void Reset_LensRuntimeState();

private:
	struct LENS_ELEMENT
	{
		CEffect_Part* pPart = nullptr;
		_float* pEmitterAlpha = nullptr;
		_float fRuntimeEmitterAlpha = 1.f;
		_float3 vAuthorLocalPosition{};
		_float fAxisPosition = 0.f;
		_float2 vScreenOffset{};
	};

	struct PENDING_LENS_POSITION
	{
		CTransform* pTransform = nullptr;
		_float3 vLocalPosition{};
	};

	unordered_map<_wstring, LENS_ELEMENT> m_LensElements;
	_bool m_bLensElementCacheReady = false;
	_bool m_bLensElementCacheWarningLogged = false;
	_bool m_bAuthorPlacementRestored = false;
	_bool m_bScreenVisible = false;

	_bool m_bFadeInPending = true;
	_float m_fFadeInAccTime = 0.f;
	_float m_fFadeInAlpha = 0.f;

private:
	HRESULT Ready_EffectPartObjects();
	void    Cache_LensElements();
	_bool   Try_GetAuthorLocalPosition(const CEffect_Part* pPart, _float3* pOutLocalPosition) const;
	_float* Find_EmitterAlpha(CEffect_Part* pPart) const;
	_bool   Validate_LensProperties();
	_bool   Project_SourceToNDC(_float2* pOutSourceNDC) const;
	_float2 Calculate_GhostNDC(const _float2& vSourceNDC, _float fAxisPosition, const _float2& vScreenOffset) const;
	_bool   Unproject_AtViewDepth(const _float2& vNDC, _float fViewDepth, _float3* pOutWorldPosition) const;
	_bool   Update_ScreenVisibility(const _float2& vSourceNDC);
	_bool   Update_LensFlarePlacement();
	void    Restore_AuthorPlacement();
	void    Queue_FadeIn();
	void    Update_FadeIn(_float fTimeDelta);
	void    Apply_FadeInAlpha();
	void    Restore_EmitterAlpha();

public:
	static CLensFlare* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END