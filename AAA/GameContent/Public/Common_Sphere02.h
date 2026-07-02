#pragma once

#include "GameContent_Defines.h"

#include "Effect_MeshParticle.h"

NS_BEGIN(Client)

class CCommon_Sphere02 final : public CEffect_MeshParticle
{
	GENERATED_BODY(CCommon_Sphere02)

public:
	struct COMMON_SPHERE_DESC : public CEffect_MeshParticle::EFFECT_MESHPARTICLE_DESC
	{

	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CommonSphere";

private:
	CCommon_Sphere02(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCommon_Sphere02(const CCommon_Sphere02& Prototype);
	virtual ~CCommon_Sphere02() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	static CCommon_Sphere02* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END