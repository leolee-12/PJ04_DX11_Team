#pragma once
#include "Effect_Container.h"

NS_BEGIN(Client)

class CCrashChargeEffect final : public CEffect_Container
{
	GENERATED_BODY(CCrashChargeEffect)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_CrashChargeEffect";

	static constexpr const _tchar* MODEL_CIRCLE_TAG = L"Prototype_Component_Model_CrashCharge_Circle";
	static constexpr const _tchar* MODEL_THUNDER_SPIRAL_TAG = L"Prototype_Component_Model_CrashCharge_ThunderSpiral";

	static constexpr const _tchar* TEXTURE_WARP_CHARGE_TAG = L"Prototype_Component_Texture_CrashCharge_Warp";
	static constexpr const _tchar* TEXTURE_CIRCLE01_TAG = L"Prototype_Component_Texture_CrashCharge_Circle01";
	static constexpr const _tchar* TEXTURE_CIRCLE06_TAG = L"Prototype_Component_Texture_CrashCharge_Circle06";
	static constexpr const _tchar* TEXTURE_TWINKLE03_TAG = L"Prototype_Component_Texture_CrashCharge_Twinkle03";
	static constexpr const _tchar* TEXTURE_INDIRECT3_TAG = L"Prototype_Component_Texture_CrashCharge_Indirect3";
	static constexpr const _tchar* TEXTURE_RING07_TAG = L"Prototype_Component_Texture_CrashCharge_Ring07";
	static constexpr const _tchar* TEXTURE_SCROLL14_TAG = L"Prototype_Component_Texture_CrashCharge_Scroll14";
	static constexpr const _tchar* TEXTURE_SHINE_CIRCLE_TAG = L"Prototype_Component_Texture_CrashCharge_ShineCircle";
	static constexpr const _tchar* TEXTURE_CIRCLE_GLOW_TAG = L"Prototype_Component_Texture_CrashCharge_CircleGlow";
	static constexpr const _tchar* TEXTURE_LINE03_TAG = L"Prototype_Component_Texture_CrashCharge_Line03";
	static constexpr const _tchar* TEXTURE_LINE04_TAG = L"Prototype_Component_Texture_CrashCharge_Line04";
	static constexpr const _tchar* TEXTURE_STAR03_TAG = L"Prototype_Component_Texture_CrashCharge_Star03";
	static constexpr const _tchar* TEXTURE_INDIRECT_NORMAL_TAG = L"Prototype_Component_Texture_CrashCharge_IndirectNormal";
	static constexpr const _tchar* TEXTURE_THUNDER_STRAIGHT_TAG = L"Prototype_Component_Texture_CrashCharge_ThunderStraight";

private:
	CCrashChargeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCrashChargeEffect(const CCrashChargeEffect& Prototype);
	virtual ~CCrashChargeEffect() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

private:
	HRESULT Ready_EffectPartObjects();
	_float Resolve_TimeDelta(_float fTimeDelta);

public:
	static CCrashChargeEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
