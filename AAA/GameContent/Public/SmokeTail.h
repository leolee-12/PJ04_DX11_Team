#pragma once

#include "GameContent_Defines.h"

#include "Effect_Mesh.h"

NS_BEGIN(Client)

class CSmokeTail final : public CEffect_Mesh
{
	GENERATED_BODY(CSmokeTail)

public:
	struct SMOKE_TAIL_DESC : public CEffect_Mesh::EFFECT_MESH_DESC
	{

	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_SmokeTail";

private:
	CSmokeTail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSmokeTail(const CSmokeTail& Prototype);
	virtual ~CSmokeTail() = default;

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
	static CSmokeTail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END