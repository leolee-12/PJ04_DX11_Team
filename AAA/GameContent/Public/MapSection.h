#pragma once
#include "MapObject.h"
#include "Map_Defines.h"

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CMapSection final : public CMapObject
{
	GENERATED_BODY(CMapSection)

	PROPERTY(_bool, m_bRenderable, L"Renderable", L"MapSection");
	PROPERTY(_bool, m_bEnableCulling, L"Enable Culling", L"MapSection");

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MapSection";

private:
	CMapSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapSection(const CMapSection& Prototype);
	virtual ~CMapSection() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render_Shadow() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	void	Refresh_WorldBounds();
	void	Set_ParentMatrix(const _float4x4* pParentMatrix);
	void	Refresh_CombinedWorldMatrix();
	void	Notify_EditTransformChanged();

#ifdef _DEBUG
	void	Reset_FrameProfile();
#endif

public:
	json	Serialize_SectionState() const;
	void	Deserialize_SectionState(const json& j);
	void	Set_RenderID(RENDERID eRenderID) { m_eRenderID = eRenderID; }
	void	Set_Culling(_bool bEnableCulling) { m_bEnableCulling = bEnableCulling; }
	void	Set_CollisionActorEnabled(_bool bEnable) { m_bCreateCollisionActor = bEnable; }
	void	Set_Renderable(_bool bRenderable) { m_bRenderable = bRenderable; }
	void	Set_RuntimeCollisionActorEnabled(_bool bEnable);
	_bool	Is_CollisionActorEnabled() const { return m_bCreateCollisionActor; }

	const MAP_SECTION_DESC&	Get_Desc() const { return m_tDesc; }
	const BoundingBox&		Get_WorldBounds() const { return m_WorldBounds; }
	MAP_SECTION_TYPE		Get_SectionType() const { return m_eSectionType; }
	RENDERID				Get_RenderID() const { return m_eRenderID; }
	_bool					Is_Culling() const { return m_bEnableCulling; }
	_bool					Is_Renderable() const { return m_bRenderable; }
	const _wstring&			Get_SectionName() const { return m_strSectionName; }

#ifdef _DEBUG
	const MAP_SECTION_PROFILE& Get_Profile() const { return m_Profile; }

	void	Set_EditorSoloMeshIndex(_int iMeshIndex);
	void	Clear_EditorSoloMesh();
	_int	Get_EditorSoloMeshIndex() const { return m_iEditorSoloMeshIndex; }
	_bool	Is_EditorSoloMeshEnabled() const { return m_iEditorSoloMeshIndex >= 0; }

private:
	virtual _bool	Should_RenderMesh(_uint iMesh) const override;
#endif

private:
	virtual const _tchar*	Get_ModelProtoTag() const override;
	virtual _uint			Get_ModelProtoLevel() const override;
	virtual HRESULT			Ready_Events() override { return S_OK; }
	virtual HRESULT			Bind_WorldMatrix() override;
	void					Update_LocalBounds();
	void					Refresh_ColliderActor();

private:
	_wstring			m_strProtoTag = { PROTOTYPE_TAG };
	_wstring			m_strSectionName;
	_wstring			m_strModelProtoTag;
	_wstring			m_strModelPath;
	_uint				m_iModelProtoLevel = {};
	MAP_SECTION_TYPE	m_eSectionType = { MAP_SECTION_TYPE::UNKNOWN };
	RENDERID			m_eRenderID = { RENDERID::NONBLEND };
	BoundingBox			m_LocalBounds = {};
	BoundingBox			m_WorldBounds = {};
	const _float4x4*	m_pParentMatrix = {};
	_float4x4			m_CombinedWorldMatrix = {};
	MAP_SECTION_DESC	m_tDesc = {};
	_bool				m_bCreateCollisionActor = { true };

	physx::PxRigidStatic* m_pColliderActor = { nullptr };

#ifdef _DEBUG
	MAP_SECTION_PROFILE	m_Profile = {};

	_int m_iEditorSoloMeshIndex = -1;
#endif

public:
	static CMapSection* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
