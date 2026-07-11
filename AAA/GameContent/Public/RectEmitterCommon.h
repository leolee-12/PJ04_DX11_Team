#pragma once
#include "GameContent_Defines.h"
#include "Effect_RectEmitter.h"

NS_BEGIN(Client)

class CRectEmitterCommon final : public CEffect_RectEmitter
{
	GENERATED_BODY(CRectEmitterCommon)

	PROPERTY(_int, m_iRenderGroup, L"Render Group", L"Rendering");

public:
	struct RECT_EMITTER_COMMON_DESC : public CEffect_RectEmitter::EFFECT_RECTEMITTER_DESC
	{
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_RectEmitterCommon";

private:
	CRectEmitterCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRectEmitterCommon(const CRectEmitterCommon& Prototype);
	virtual ~CRectEmitterCommon() = default;

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
	static CRectEmitterCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
