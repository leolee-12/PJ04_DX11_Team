#pragma once

#include "GameContent_Defines.h"

#include "Effect_MeshParticle.h"

NS_BEGIN(Client)

class CTestMeshParticle final : public CEffect_MeshParticle
{
	GENERATED_BODY(CEffect_MeshParticle)

public:
	struct TEST_MESHPARTICLE_DESC : public CEffect_MeshParticle::EFFECT_MESHPARTICLE_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TestMeshParticle";

private:
	CTestMeshParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestMeshParticle(const CTestMeshParticle& Prototype);
	virtual ~CTestMeshParticle() = default;

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
	static CTestMeshParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END