#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CJumpEffect final : public CEffect_Container
{
	GENERATED_BODY(CJumpEffect)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_JumpEffect";

private:
	CJumpEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CJumpEffect(const CJumpEffect& Prototype);
	virtual ~CJumpEffect() = default;

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
	static CJumpEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
