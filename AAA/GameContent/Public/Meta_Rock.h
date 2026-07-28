#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CMeta_Rock final : public CEffect_Container
{
	GENERATED_BODY(CMeta_Rock)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Meta_RockEffect";
	static constexpr const _tchar* MODEL_PROTO_TAG_ROCK = L"Prototype_Component_Model_Meta_RockEffect";

private:
	CMeta_Rock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeta_Rock(const CMeta_Rock& Prototype);
	virtual ~CMeta_Rock() = default;

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
	static CMeta_Rock* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
