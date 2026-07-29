#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CHammerFire final : public CEffect_Container
{
	GENERATED_BODY(CHammerFire)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_HammerFire";
	static constexpr const _tchar* MODEL_PROTO_TAG_FIRE01 = L"Prototype_Component_Model_HammerFire01";
	static constexpr const _tchar* MODEL_PROTO_TAG_FIRE03 = L"Prototype_Component_Model_HammerFire03";
	static constexpr const _tchar* WARP_PART_TAG = L"FireWarp";
	static constexpr const _tchar* OUTER_PART_TAG = L"FireOuter";
	static constexpr const _tchar* CORE_PART_TAG = L"FireCore";

private:
	CHammerFire(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHammerFire(const CHammerFire& Prototype);
	virtual ~CHammerFire() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	HRESULT Ready_EffectPartObjects();

public:
	static CHammerFire* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
