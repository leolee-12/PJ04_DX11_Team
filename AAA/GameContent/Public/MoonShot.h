#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CMoonShot final : public CEffect_Container
{
	GENERATED_BODY(CMoonShot)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MoonShot";
	static constexpr const _tchar* MODEL_PROTO_TAG_MOON = L"Prototype_Component_Model_MetaMoonShot_Moon";
	static constexpr const _tchar* TEXTURE_PROTO_TAG_FIRE_FORM = L"Prototype_Component_Texture_MetaMoonShot_FireForm";

private:
	CMoonShot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMoonShot(const CMoonShot& Prototype);
	virtual ~CMoonShot() = default;

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
	static CMoonShot* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
