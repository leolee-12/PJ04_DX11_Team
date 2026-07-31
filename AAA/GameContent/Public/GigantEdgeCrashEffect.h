#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CGigantEdgeCrashEffect final : public CEffect_Container
{
	GENERATED_BODY(CGigantEdgeCrashEffect)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_GigantEdgeCrashEffect";

	static constexpr const _tchar* FLOOR_MODEL_TAG =
		L"Prototype_Component_Model_GigantEdge_Floor";
	static constexpr const _tchar* SMOKE_MODEL_TAG =
		L"Prototype_Component_Model_GigantEdge_CrashSmoke";

private:
	CGigantEdgeCrashEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGigantEdgeCrashEffect(const CGigantEdgeCrashEffect& Prototype);
	virtual ~CGigantEdgeCrashEffect() = default;

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
	static CGigantEdgeCrashEffect* Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
