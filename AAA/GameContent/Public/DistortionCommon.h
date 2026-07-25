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

private:
	HRESULT Ready_Components();
	HRESULT Bind_DistortionResources(_float* pOutAlpha);

private:
	CModel* m_pModelCom{};

	_uint m_iModelLevel{};
	_wstring m_wstrModelTag;

public:
	static CDistortionCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
