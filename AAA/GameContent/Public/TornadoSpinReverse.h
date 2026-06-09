#pragma once

#include "GameContent_Defines.h"

#include "Effect_Mesh.h"

NS_BEGIN(Client)

class CTornadoSpinReverse final : public CEffect_Mesh
{
	GENERATED_BODY(CTornadoSpinReverse)

public:
	struct TORNADO_SPIN_REVERSE_DESC : public CEffect_Mesh::EFFECT_MESH_DESC
	{

	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_TornadoSpinReverse";

private:
	CTornadoSpinReverse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTornadoSpinReverse(const CTornadoSpinReverse& Prototype);
	virtual ~CTornadoSpinReverse() = default;

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
	static CTornadoSpinReverse* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END