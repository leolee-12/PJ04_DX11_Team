#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CLeopard_Floor final : public CEffect_Container
{
	GENERATED_BODY(CLeopard_Floor)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Leopard_Floor";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Leopard_Floor";

private:
	CLeopard_Floor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLeopard_Floor(const CLeopard_Floor& Prototype);
	virtual ~CLeopard_Floor() = default;

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
	static CLeopard_Floor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
