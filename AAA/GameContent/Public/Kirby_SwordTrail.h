#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CKirby_SwordTrail final : public CEffect_Container
{
	GENERATED_BODY(CKirby_SwordTrail)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Kirby_SwordTrail";

private:
	CKirby_SwordTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_SwordTrail(const CKirby_SwordTrail& Prototype);
	virtual ~CKirby_SwordTrail() = default;

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
	static CKirby_SwordTrail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
