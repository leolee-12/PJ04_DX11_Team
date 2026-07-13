#pragma once

#include "GameContent_Defines.h"
#include "Effect_Trail.h"

NS_BEGIN(Client)

class CTrailCommon final : public CEffect_Trail
{
	GENERATED_BODY(CTrailCommon)

	PROPERTY(_int, m_iRenderGroup, L"Render Group", L"Rendering");

public:
	struct TRAIL_COMMON_DESC : public CEffect_Trail::EFFECT_TRAIL_DESC
	{
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_TrailCommon";

private:
	CTrailCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrailCommon(const CTrailCommon& Prototype);
	virtual ~CTrailCommon() = default;

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
	static CTrailCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
