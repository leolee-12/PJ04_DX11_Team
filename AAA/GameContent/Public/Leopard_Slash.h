#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CLeopard_Slash final : public CEffect_Container
{
	GENERATED_BODY(CLeopard_Slash)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Leopard_Slash";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Leopard_Ring";

private:
	CLeopard_Slash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLeopard_Slash(const CLeopard_Slash& Prototype);
	virtual ~CLeopard_Slash() = default;

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
	static CLeopard_Slash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
