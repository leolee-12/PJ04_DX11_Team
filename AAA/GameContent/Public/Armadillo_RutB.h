#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CArmadillo_RutB final : public CEffect_Container
{
	GENERATED_BODY(CArmadillo_RutB)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Armadillo_RutB";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_RutB";

private:
	CArmadillo_RutB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArmadillo_RutB(const CArmadillo_RutB& Prototype);
	virtual ~CArmadillo_RutB() = default;

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
	static CArmadillo_RutB* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
