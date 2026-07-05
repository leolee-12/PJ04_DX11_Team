#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Client)

class CLevelDesign_Rail final : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Rail)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Rail";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Rail";
	static constexpr const _tchar* OBJECT_NAME = L"Rail";

private:
	CLevelDesign_Rail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Rail(const CLevelDesign_Rail& Prototype);
	virtual ~CLevelDesign_Rail() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	const LD_RAIL_DESC& Get_RailDesc() const { return m_tRailDesc; }
	static const CLevelDesign_Rail* Find_ByUid(CGameInstance_Proxy* pProxy, _uint iLevelIndex, _uint iRailUid);
	static _uint Get_SegmentCount(const LD_RAIL_DESC& RailDesc);
	static _bool Evaluate_Segment(const LD_RAIL_DESC& RailDesc, _uint iSegmentIndex, _float fT, _float3* pOutPosition, _float3* pOutTangent = nullptr);

private:
	LD_RAIL_DESC m_tRailDesc = {};

private:
	virtual HRESULT Validate_Initialized() override;

#ifdef _DEBUG
private:
	PrimitiveBatch<VertexPositionColor>* m_pBatch = nullptr;
	BasicEffect* m_pEffect = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;

private:
	HRESULT Ready_DebugResources();
	HRESULT Render_Rail();
#endif

public:
	static	CLevelDesign_Rail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END