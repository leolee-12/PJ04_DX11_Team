#pragma once
#include "GameContent_Defines.h"
#include "Effect_Quad.h"

NS_BEGIN(Client)

class CQuadCommon final : public CEffect_Quad
{
	GENERATED_BODY(CQuadCommon)

	PROPERTY(_int, m_iRenderGroup, L"Render Group", L"Rendering");

public:
	struct QUAD_COMMON_DESC : public CEffect_Quad::EFFECT_QUAD_DESC
	{
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_QuadCommon";

private:
	CQuadCommon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQuadCommon(const CQuadCommon& Prototype);
	virtual ~CQuadCommon() = default;

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
	static CQuadCommon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
