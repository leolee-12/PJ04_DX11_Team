#pragma once
#include "Component.h"
#include "Geometry_Utils.h"

NS_BEGIN(Engine)
class CModel;

class ENGINE_DLL CCullingState final : public CComponent
{
public:
	enum class CHANNEL : _uint { MAIN, SHADOW, COUNT };

	enum CAUSE_FLAGS : _uint
	{
		CAUSE_NONE		= 0u,
		CAUSE_DISTANCE	= 1u << 0,
		CAUSE_FRUSTUM	= 1u << 1
	};

	struct CHANNEL_RESULT
	{
		_bool   bCulled = { false };
		_float  fDissolve = { 0.f };
		_uint   iCauseFlags = { CAUSE_NONE };
	};

	struct CULLING_CHANNEL_QUERY
	{
		CULLING_VIEW    eView = { CULLING_VIEW::MAIN_CAMERA };

		_bool   bUseDistance = { true };
		_bool   bUseFrustum = { true };
		_bool   bUseFrustumFade = { true };
	};

	struct CULLING_EVALUATION_INPUT
	{
		CULLING_CHANNEL_QUERY	Main = {};
		CULLING_CHANNEL_QUERY	Shadow = { CULLING_VIEW::SHADOW_DIR, true, true, true };

		_bool   bEvaluateMain = { true };
		_bool   bEvaluateShadow = { false };
	};

private:
	CCullingState(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCullingState(const CCullingState& Prototype);
	virtual ~CCullingState() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Set_LocalBounds(const BoundingBox& LocalBounds);
	HRESULT Set_LocalBoundsFromModel(CModel* pModel, _float fMargin = 0.f);

	void Set_RotationInvariant(_bool bEnable);
	void Mark_TransformDirty();

	void Refresh_WorldBounds(const _float4x4& WorldMatrix);
	void Evaluate(const CULLING_EVALUATION_INPUT& Desc);

	void Reset_AllResults();

	_bool Is_Culled(CHANNEL eChannel) const;
	_float Get_Dissolve(CHANNEL eChannel) const;

	const BoundingBox& Get_LocalBounds() const;
	const BoundingBox& Get_WorldBounds() const;

private:
	BoundingBox		m_LocalBounds = {};
	BoundingBox		m_WorldBounds = {};
	BoundingSphere	m_WorldSphere = {};

	_bool	m_bHasBounds = { false };
	_bool	m_bTransformDirty = { true };
	_bool	m_bRotationInvariant = { false };

	_bool	m_bForceEvaluation = { true };
	_uint	m_iEvaluationPhase = { 0u };

	CHANNEL_RESULT  m_Results[ETOUI(CHANNEL::COUNT)] = {};

private:
	void Evaluate_Channel(CHANNEL eChannel, const CULLING_CHANNEL_QUERY& Query, const CULLING_FADE_RESULT* pDistanceResult);

public:
	static CCullingState* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END