#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CLeopard_Afterimage_Assault final : public CEffect_Container
{
	GENERATED_BODY(CLeopard_Afterimage_Assault)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Leopard_Afterimage_Assault";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Leopard_Afterimage_Assault";

private:
	CLeopard_Afterimage_Assault(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLeopard_Afterimage_Assault(const CLeopard_Afterimage_Assault& Prototype);
	virtual ~CLeopard_Afterimage_Assault() = default;

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
	static CLeopard_Afterimage_Assault* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
