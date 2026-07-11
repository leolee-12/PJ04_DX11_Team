#pragma once
#include "GameContent_Defines.h"
#include "Effect_MeshParticle.h"

NS_BEGIN(Client)

class CMeshParticleCommon final : public CEffect_MeshParticle
{
	GENERATED_BODY(CMeshParticleCommon)

	PROPERTY(_int, m_iRenderGroup, L"Render Group", L"Rendering");

public:
	struct MESH_PARTICLE_COMMON_DESC : public CEffect_MeshParticle::EFFECT_MESHPARTICLE_DESC
	{
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MeshParticleCommon";

private:
	CMeshParticleCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshParticleCommon(const CMeshParticleCommon& Prototype);
	virtual ~CMeshParticleCommon() = default;

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
	static CMeshParticleCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
