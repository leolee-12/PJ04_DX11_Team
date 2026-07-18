#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CLeopard_Flash final : public CEffect_Container
{
	GENERATED_BODY(CLeopard_Flash)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Leopard_Flash";
	static constexpr const _tchar* MODEL_PROTO_TAG_RING = L"Prototype_Component_Model_Leopard_Ring";
	static constexpr const _tchar* MODEL_PROTO_TAG_CIRCLE = L"Prototype_Component_Model_Leopard_Circle";

private:
	CLeopard_Flash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLeopard_Flash(const CLeopard_Flash& Prototype);
	virtual ~CLeopard_Flash() = default;

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
	static CLeopard_Flash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
