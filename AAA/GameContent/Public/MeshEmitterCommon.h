#pragma once
#include "GameContent_Defines.h"
#include "Effect_MeshEmitter.h"

NS_BEGIN(Client)

class CMeshEmitterCommon final : public CEffect_MeshEmitter
{
	GENERATED_BODY(CMeshEmitterCommon)

	PROPERTY(_int, m_iRenderGroup, L"Render Group", L"Rendering");

public:
	struct MESH_EMITTER_COMMON_DESC : public CEffect_MeshEmitter::EFFECT_MESHEMITTER_DESC
	{
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MeshEmitterCommon";

private:
	CMeshEmitterCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshEmitterCommon(const CMeshEmitterCommon& Prototype);
	virtual ~CMeshEmitterCommon() = default;

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
	static CMeshEmitterCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END
