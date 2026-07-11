#pragma once
#include "GameContent_Defines.h"
#include "Effect_Mesh.h"

NS_BEGIN(Client)

class CMeshCommon final : public CEffect_Mesh
{
	GENERATED_BODY(CMeshCommon)

	PROPERTY(_int, m_iRenderGroup, L"Render Group", L"Rendering");

public:
	struct MESH_COMMON_DESC : public CEffect_Mesh::EFFECT_MESH_DESC
	{
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MeshCommon";

private:
	CMeshCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshCommon(const CMeshCommon& Prototype);
	virtual ~CMeshCommon() = default;

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
	static CMeshCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
