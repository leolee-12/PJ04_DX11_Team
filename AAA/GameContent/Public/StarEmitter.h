#pragma once

#include "GameContent_Defines.h"

#include "Effect_MeshEmitter.h"

NS_BEGIN(Client)

class CStarEmitter final : public CEffect_MeshEmitter
{
	GENERATED_BODY(CStarEmitter)

public:
	struct STAR_EMITTER_DESC : public CEffect_MeshEmitter::EFFECT_MESHEMITTER_DESC
	{

	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_StarEmitter";

private:
	CStarEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStarEmitter(const CStarEmitter& Prototype);
	virtual ~CStarEmitter() = default;

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
	static CStarEmitter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END