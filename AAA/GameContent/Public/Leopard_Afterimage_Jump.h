#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CLeopard_Afterimage_Jump final : public CEffect_Container
{
	GENERATED_BODY(CLeopard_Afterimage_Jump)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Leopard_Afterimage_Jump";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Leopard_Afterimage_Jump";

private:
	CLeopard_Afterimage_Jump(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLeopard_Afterimage_Jump(const CLeopard_Afterimage_Jump& Prototype);
	virtual ~CLeopard_Afterimage_Jump() = default;

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
	static CLeopard_Afterimage_Jump* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
