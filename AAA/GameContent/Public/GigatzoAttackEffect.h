#pragma once

#include "Effect_Container.h"
NS_BEGIN(Engine)
class CEffect_Part;
NS_END

NS_BEGIN(Client)

class CGigatzoAttackEffect final : public CEffect_Container
{
	GENERATED_BODY(CGigatzoAttackEffect)

public:
	static constexpr const _tchar* PROTOTYPE_TAG		= L"Proto_GigatzoAttackEffect";
	static constexpr const _tchar* FX_ID				= L"GigatzoAttackEffect";
	static constexpr const _tchar* SMOKE_MODEL_TAG		= L"Prototype_Component_Model_GigatzoSmokeSphere";

private:
	CGigatzoAttackEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGigatzoAttackEffect(const CGigatzoAttackEffect& Prototype);
	virtual ~CGigatzoAttackEffect() = default;

protected:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	HRESULT						Ready_EffectPartObjects();

public:
	static CGigatzoAttackEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void				Free() override;
};

NS_END
