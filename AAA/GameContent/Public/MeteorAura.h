#pragma once

#include "Effect_Container.h"
NS_BEGIN(Engine)
class CEffect_Part;
NS_END

NS_BEGIN(Client)

class CMeteorAura final : public CEffect_Container
{
	GENERATED_BODY(CMeteorAura)

public:
	static constexpr const _tchar* PROTOTYPE_TAG		= L"Proto_MeteorAura";
	static constexpr const _tchar* SHELL_MODEL_TAG		= L"Prototype_Component_Model_MeteorAuraShell";
	static constexpr const _tchar* FIRE01_MODEL_TAG		= L"Prototype_Component_Model_MeteorAuraFire01";
	static constexpr const _tchar* FIRE02_MODEL_TAG		= L"Prototype_Component_Model_MeteorAuraFire02";

private:
	CMeteorAura(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeteorAura(const CMeteorAura& Prototype);
	virtual ~CMeteorAura() = default;

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
	static CMeteorAura* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void				Free() override;
};

NS_END
