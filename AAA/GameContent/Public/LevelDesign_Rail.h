#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)
class CRailTrack;

class CLevelDesign_Rail final : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Rail)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Rail";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Rail";
	static constexpr const _tchar* OBJECT_NAME = L"Rail";
	static constexpr const _tchar* COASTER_RAIL_MODEL_PROTO_TAG = L"Proto_Component_Model_CoasterRail";

	enum class RAIL_VISUAL_TYPE : _uint { NONE, COASTER, END };

	struct COASTER_RAIL_INSTANCE_DATA
	{
		_float4x4 matWorld = {};
		_float4 vDissolveParams = {};
	};

	static_assert(sizeof(COASTER_RAIL_INSTANCE_DATA) == 80, "COASTER_RAIL_INSTANCE_DATA must match the instance input layout.");

private:
	CLevelDesign_Rail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Rail(const CLevelDesign_Rail& Prototype);
	virtual ~CLevelDesign_Rail() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	const LD_RAIL_DESC& Get_RailDesc() const { return m_tRailDesc; }
	const CRailTrack* Get_RailTrack() const { return m_pRailTrack; }
	
	HRESULT Enable_Visual(RAIL_VISUAL_TYPE eType, _uint iModelPrototypeLevel);

	static CLevelDesign_Rail* Find_ByUid(CGameInstance_Proxy* pProxy, _uint iLevelIndex, _uint iRailUid);
	static _uint Get_SegmentCount(const LD_RAIL_DESC& RailDesc);
	static _bool Evaluate_Segment(const LD_RAIL_DESC& RailDesc, _uint iSegmentIndex, _float fT, _float3* pOutPosition, _float3* pOutTangent = nullptr);

private:
	LD_RAIL_DESC m_tRailDesc = {};
	CRailTrack* m_pRailTrack = nullptr;

	RAIL_VISUAL_TYPE m_eVisualType = RAIL_VISUAL_TYPE::NONE;
	_bool m_bVisualReady = false;
	CShader* m_pShaderCom = nullptr;
	CModel* m_pModelCom = nullptr;
	ID3D11Buffer* m_pInstanceBuffer = nullptr;
	vector<COASTER_RAIL_INSTANCE_DATA> m_Instances;

private:
	virtual HRESULT Validate_Initialized() override;
	HRESULT Ready_RenderComponents(_uint iModelPrototypeLevel);
	HRESULT Ready_InstanceData();
	HRESULT Ready_InstanceBuffer();
	HRESULT Ready_VisualCullBounds();
	HRESULT Bind_ShaderResources();
	HRESULT Render_Model();

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