#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CHammerImpactGround final : public CEffect_Container
{
	GENERATED_BODY(CHammerImpactGround)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_HammerImpactGround";
	static constexpr const _tchar* MODEL_RING_TAG = L"Prototype_Component_Model_HammerImpactGround_Ring";
	static constexpr const _tchar* TEXTURE_WARP_TAG = L"Prototype_Component_Texture_HammerImpactGround_Warp";
	static constexpr const _tchar* TEXTURE_RING_TAG = L"Prototype_Component_Texture_HammerImpactGround_Ring";
	static constexpr const _tchar* TEXTURE_STAR_TAG = L"Prototype_Component_Texture_HammerImpactGround_Star";

private:
	CHammerImpactGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHammerImpactGround(const CHammerImpactGround& Prototype);
	virtual ~CHammerImpactGround() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	HRESULT Ready_EffectPartObjects();

public:
	static CHammerImpactGround* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
