#pragma once
#include "Effect_Container.h"

NS_BEGIN(Client)

class CHammerChargeGlow final : public CEffect_Container
{
	GENERATED_BODY(CHammerChargeGlow)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_HammerChargeGlow";
	static constexpr const _tchar* TEXTURE_FLASH_TAG = L"Prototype_Component_Texture_HammerChargeGlow_Flash";
	static constexpr const _tchar* TEXTURE_RING_TAG = L"Prototype_Component_Texture_HammerChargeGlow_Ring";

private:
	CHammerChargeGlow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHammerChargeGlow(const CHammerChargeGlow& Prototype);
	virtual ~CHammerChargeGlow() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_EffectPartObjects();

public:
	static CHammerChargeGlow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
