#pragma once

#include "GameContent_Defines.h"
#include "Effect_NonParticle.h"

NS_BEGIN(Engine)

class CModel;

NS_END

NS_BEGIN(Client)

class CDistortionCommon final : public CEffect_NonParticle
{
	GENERATED_BODY(CDistortionCommon)

	PROPERTY(_bool, m_bBillboard, L"Billboard", L"Distortion");
	PROPERTY(_bool, m_bRadialFromUV, L"Radial From UV", L"Distortion");
	PROPERTY(_float, m_fDistortionStrength, L"Strength", L"Distortion");
	PROPERTY(_bool, m_bFlipGreen, L"Flip Green", L"Distortion");
	PROPERTY(_bool, m_bUseUVEdgeFade, L"Use UV Edge Fade", L"Distortion");
	PROPERTY(_int, m_iUVEdgeFadeAxis, L"Edge Fade Axis", L"Distortion");
	PROPERTY(_float, m_fUVEdgeFadeStartRange, L"Edge Fade Start Range", L"Distortion");
	PROPERTY(_float, m_fUVEdgeFadeEndRange, L"Edge Fade End Range", L"Distortion");
	PROPERTY(_float, m_fUVEdgeFadePower, L"Edge Fade Power", L"Distortion");
	PROPERTY(_bool, m_bLinearReveal, L"Linear Reveal", L"Distortion");
	PROPERTY(_int, m_iLinearRevealAxis, L"Reveal Axis", L"Distortion");
	PROPERTY(_bool, m_bLinearRevealReverse, L"Reveal Reverse", L"Distortion");
	PROPERTY(_float, m_fLinearRevealStartRatio, L"Reveal Start Ratio", L"Distortion");
	PROPERTY(_float, m_fLinearRevealEndRatio, L"Reveal End Ratio", L"Distortion");
	PROPERTY(_bool, m_bLinearHide, L"Linear Hide", L"Distortion");
	PROPERTY(_int, m_iLinearHideAxis, L"Hide Axis", L"Distortion");
	PROPERTY(_bool, m_bLinearHideReverse, L"Hide Reverse", L"Distortion");
	PROPERTY(_float, m_fLinearHideStartRatio, L"Hide Start Ratio", L"Distortion");
	PROPERTY(_float, m_fLinearHideEndRatio, L"Hide End Ratio", L"Distortion");

public:
	struct DISTORTION_COMMON_DESC : public CEffect_NonParticle::EFFECT_NONEPARTICLE_DESC
	{
		_uint iModelLevel{};
		_wstring wstrModelTag;
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_DistortionCommon";

private:
	CDistortionCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDistortionCommon(const CDistortionCommon& Prototype);
	virtual ~CDistortionCommon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

protected:
	virtual void Update_Core(const _float fTimeDelta, const _float fRatio) override;

private:
	HRESULT Ready_Components();
	HRESULT Bind_DistortionResources(_float* pOutAlpha);

private:
	CModel* m_pModelCom{};

	_uint m_iModelLevel{};
	_wstring m_wstrModelTag;
	_float m_fLinearRevealRatio{ 1.f };
	_float m_fLinearHideRatio{ 1.f };

public:
	static CDistortionCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
