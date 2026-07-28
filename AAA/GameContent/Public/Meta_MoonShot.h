#pragma once

#include "Effect_Container.h"

NS_BEGIN(Client)

class CMeta_MoonShot final : public CEffect_Container
{
	GENERATED_BODY(CMeta_MoonShot)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Meta_MoonShot";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Meta_MoonShot";
	static constexpr const _tchar* MODEL_PROTO_TAG_TOP = L"Prototype_Component_Model_Meta_MoonShotTop";

private:
	CMeta_MoonShot(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeta_MoonShot(const CMeta_MoonShot& Prototype);
	virtual ~CMeta_MoonShot() = default;

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
	static CMeta_MoonShot* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
