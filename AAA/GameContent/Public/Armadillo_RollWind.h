#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CArmadillo_RollWind final : public CEffect_Container
{
	GENERATED_BODY(CArmadillo_RollWind)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Armadillo_RollWind";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_RollWind";

private:
	CArmadillo_RollWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArmadillo_RollWind(const CArmadillo_RollWind& Prototype);
	virtual ~CArmadillo_RollWind() = default;

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
	static CArmadillo_RollWind* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
