#pragma once

#include "Base.h"
#include "Culling_Util.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCulling_Context final : public CBase
{
public:
	enum class ECullingViewType : _ubyte
	{
		UNKNOWN = 0,
		GAME_CAMERA,
		SHADOW_LIGHT,
		PICKING,
		//EDITOR_VIEWPORT,	// 필요할 경우에 추가
	};

	struct CULLING_CONTEXT_DESC
	{
		const _float4x4*	pView = { nullptr };
		const _float4x4*	pProj = { nullptr };
		const _float4x4*	pInvView = { nullptr };
		const _float4*		pEyePos = { nullptr };
		_float				fNearZ = { 0.f };
		_float				fFarZ = { 0.f };
		_float				fCullMargin = { 0.f };
		ECullingViewType	eViewType = { ECullingViewType::UNKNOWN };
		const _tchar*		pDebugName = { TEXT("UnnamedCullingContext") };
	};

private:
	CCulling_Context() = default;
	virtual ~CCulling_Context() = default;

public:
	_bool Intersects(const BoundingBox& WorldBounds) const;
	_bool Intersects(const BoundingSphere& WorldBounds) const;

	_bool Should_CullAABB(_bool bEnableCulling, const BoundingBox& WorldBounds) const;
	_bool Should_CullSphere(_bool bEnableCulling, const BoundingSphere& WorldBounds) const;

public:
	_bool Has_Frustum() const { return m_bHasFrustum; }
	_bool Is_Valid() const { return m_bHasFrustum; }

	const BoundingFrustum& Get_WorldFrustum() const { return m_WorldFrustum; }
	const _float4& Get_EyePosition() const { return m_vEyePos; }

	_float Get_NearZ() const { return m_fNearZ; }
	_float Get_FarZ() const { return m_fFarZ; }
	_float Get_CullMargin() const { return m_fCullMargin; }

	ECullingViewType Get_ViewType() const { return m_eViewType; }
	const _tchar* Get_DebugName() const;

private:
	static _bool Try_BuildWorldFrustum(const CULLING_CONTEXT_DESC& tDesc, BoundingFrustum* pOutWorldFrustum);

private:
	BoundingFrustum		m_WorldFrustum = {};
	_float4				m_vEyePos = {};
	_float				m_fNearZ = { 0.f };
	_float				m_fFarZ = { 0.f };
	_float				m_fCullMargin = { 0.f };
	ECullingViewType	m_eViewType = { ECullingViewType::UNKNOWN };
	_bool				m_bHasFrustum = { false };
	wstring				m_strDebugName;

public:
	static CCulling_Context Create(const CULLING_CONTEXT_DESC& tDesc);

private:
	virtual void Free() override;
};

NS_END