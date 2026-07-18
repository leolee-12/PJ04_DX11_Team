#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CLeopard_Impact final : public CEffect_Container
{
	GENERATED_BODY(CLeopard_Impact)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Leopard_Impact";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING = L"Prototype_Component_Model_Leopard_ImpactRing";
	static constexpr const _tchar* MODEL_PROTO_TAG_CIRCLE = L"Prototype_Component_Model_Leopard_ImpactCircle";

private:
	CLeopard_Impact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLeopard_Impact(const CLeopard_Impact& Prototype);
	virtual ~CLeopard_Impact() = default;

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
	static CLeopard_Impact* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
