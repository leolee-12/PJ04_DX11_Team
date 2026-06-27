#pragma once

#include "EnvObject_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CEnvObject abstract : public CGameObject
{
	GENERATED_BODY_ABSTRACT(CEnvObject)

	PROPERTY(_bool, m_bRenderable,		L"Renderable",				L"EnvObject")
	PROPERTY(_bool, m_bCastShadow,		L"Cast Shadow",				L"EnvObject")
	PROPERTY(_bool, m_bUseCullDistance,	L"Use Distance Culling",	L"EnvObject")
	PROPERTY(_bool, m_bUseCullFrustum,	L"Use Frustum Culling",		L"EnvObject")

	PROPERTY(_bool, m_Is,	L"Use Frustum Culling",		L"EnvObject")
	PROPERTY(_float, m_fDecalAlpha,		L"DecalAlpha(0 ~ 1)",		L"Decal")

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_EnvObject";

protected:
	CEnvObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject(const CEnvObject& Prototype);
	virtual ~CEnvObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	//virtual HRESULT Render_Decal() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	_bool   Is_ProfileRenderable() const { return m_bRenderable && Has_RenderModel(); }
	_bool   Is_ShadowCaster() const { return m_bCastShadow; }
	_bool   Is_Visible_Main() const { return m_bVisible; }
	_bool   Is_Visible_Shadow() const { return m_bVisibleShadow; }

	const ENV_OBJECT_DESC& Get_Desc() const { return m_tDesc; }
	const BoundingBox& Get_WorldBounds() const { return m_WorldBounds; }
	_float Get_Dissolve() const { return m_fDissolve; }

	_bool Pick_Marb1e(_fvector vRayOrigin, _fvector vRayDir, _float3* pOutHit, _float* fOutDistance);

protected:
	HRESULT Ready_RenderComponents(_uint iModelProtoLevel, const wstring& wstrModelProtoTag);
	HRESULT Ready_PhysicsActor();
	HRESULT Ready_PhysicsActor_ModelMesh();
	void	Release_PhysicsActor();

	_bool	Should_CreatePhysicsActor() const;

	HRESULT Bind_ShaderResources();
	void	Update_LocalBounds();
	void	Refresh_WorldBounds();
	void	Check_Visible();
	_bool	Has_RenderModel() const { return nullptr != m_pModelCom; }

protected:
	ENV_OBJECT_DESC	m_tDesc = {};
	wstring			m_strProtoTag = { PROTOTYPE_TAG };
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	physx::PxRigidStatic* m_pPhysicsActor = { nullptr };

	BoundingBox		m_LocalBounds = {};
	BoundingBox		m_WorldBounds = {};
	_bool			m_bTransformDirty = { true };
	_bool			m_bVisible = { false };
	_bool			m_bVisibleShadow = { false };
	_bool			m_bDebugDraw = { false };

	// 디더링관련
	_bool m_bUseCameraDither = { false }; // 객체 디더 사용 여부
	_float m_fDitherNear = { 8.f };		  // 이보다 가까우면 완전투명 (1)
	_float m_fDitherFar = { 12.f };		  // 이보다 멀면 디더 없음 (0)
	_float m_fDissolve = { 0.f };		  // 계산된 디졸브 값 (0~1)

private:
	void Apply_TransformFromDesc();
	void Apply_DescDefaults();

protected:
	virtual void Free() override;
};

NS_END
