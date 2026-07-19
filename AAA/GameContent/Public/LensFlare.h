#pragma once
#include "Effect_Container.h"

NS_BEGIN(Engine)

class CEffect_Part;

class CLensFlare final : public CEffect_Container
{
	GENERATED_BODY(CLensFlare)

	PROPERTY(_bool, m_bUseScreenAxis, L"Use Screen Axis", L"LensFlare");
	PROPERTY(_float, m_fAxisSourceZ, L"Axis Source Z", L"LensFlare");
	PROPERTY(_float, m_fAxisOppositeZ, L"Axis Opposite Z", L"LensFlare");
	PROPERTY(_float, m_fAxisExtent, L"Axis Extent", L"LensFlare");
	PROPERTY(_float, m_fViewDepthScale, L"View Depth Scale", L"LensFlare");
	PROPERTY(_float, m_fScreenCullMargin, L"Screen Cull Margin", L"LensFlare");

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

private:
	struct LENS_ELEMENT
	{
		CEffect_Part* pPart = nullptr;
		_float3 vAuthorLocalPosition{};
	};

	unordered_map<_wstring, LENS_ELEMENT> m_LensElements;
	_bool m_bLensElementCacheReady = false;
	_bool m_bLensElementCacheWarningLogged = false;
	_bool m_bAuthorPlacementRestored = false;

private:
	HRESULT	Ready_EffectPartObjects();
	void	Cache_LensElements();
	_bool	Project_SourceToNDC(_float2* pOutSourceNDC) const;
	_float	Calculate_AxisRatio(_float fAuthorZ) const;
	_float2	Calculate_GhostNDC(const _float2& vSourceNDC, _float fAxisRatio) const;
	_bool	Unproject_AtViewDepth(const _float2& vNDC, _float fViewDepth, _float3* pOutWorldPosition) const;
	_bool	Update_LensFlarePlacement();
	void	Restore_AuthorPlacement();

public:
	static CLensFlare* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END