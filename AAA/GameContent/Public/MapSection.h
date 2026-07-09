#pragma once
#include "MapObject.h"
#include "Editable.h"
#include "Map_Defines.h"

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CMapSection final : public CMapObject, public IEditable
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

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
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
	void	Set_RenderID(RENDERID eRenderID);
	void	Set_Culling(_bool bEnableCulling) { m_bEnableCulling = bEnableCulling; }
	void	Set_UseCollMesh(_bool bUseCollMesh);
	void	Set_Renderable(_bool bRenderable) { m_bRenderable = bRenderable; }
	_bool	Has_CollMesh() const { return m_bHasCollMesh; }
	_bool	Is_UseCollMesh() const { return m_bUseCollMesh; }

	const MAP_SECTION_DESC&	Get_Desc() const { return m_tDesc; }
	const BoundingBox&		Get_WorldBounds() const { return m_WorldBounds; }
	MAP_SECTION_TYPE		Get_SectionType() const { return m_eSectionType; }
	RENDERID				Get_RenderID() const { return m_eRenderID; }
	_bool					Is_Culling() const { return m_bEnableCulling; }
	_bool					Is_Renderable() const { return m_bRenderable; }
	const _wstring&			Get_SectionName() const { return m_strSectionName; }
	const _wstring&			Get_StageName() const { return m_strStageName; }
	void					Set_StageName(const _wstring& strStageName) { m_strStageName = strStageName; }

#pragma region Editable
	virtual _bool Get_EditDesc(EDITABLE_DESC* pOutDesc) const override;
	virtual HRESULT Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy) override;
	virtual const MESH_LAYER_IDX* Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const override;
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) override;
#pragma endregion

#ifdef _DEBUG
	const MAP_SECTION_PROFILE& Get_Profile() const { return m_Profile; }

	void	Set_EditorSoloMeshIndex(_int iMeshIndex);
	void	Clear_EditorSoloMesh();
	_int	Get_EditorSoloMeshIndex() const { return m_iEditorSoloMeshIndex; }
	_bool	Is_EditorSoloMeshEnabled() const { return m_iEditorSoloMeshIndex >= 0; }
#endif

private:
	_wstring			m_strSectionName;
	_wstring			m_strStageName;
	_wstring			m_strModelProtoTag;
	_uint				m_iModelProtoLevel = {};
	MAP_SECTION_TYPE	m_eSectionType = { MAP_SECTION_TYPE::UNKNOWN };
	RENDERID			m_eRenderID = { RENDERID::NONBLEND };
	BoundingBox			m_LocalBounds = {};
	BoundingBox			m_WorldBounds = {};
	const _float4x4*	m_pParentMatrix = {};
	_float4x4			m_CombinedWorldMatrix = {};
	MAP_SECTION_DESC	m_tDesc = {};
	_bool				m_bHasCollMesh = { false };
	_bool				m_bUseCollMesh = { true };

	physx::PxRigidStatic* m_pColliderActor = { nullptr };

#ifdef _DEBUG
	MAP_SECTION_PROFILE	m_Profile = {};

	_int m_iEditorSoloMeshIndex = -1;
	
private:
	virtual _bool	Should_RenderMesh(_uint iMesh) const override;
#endif

private:
	virtual const _tchar*	Get_ModelProtoTag() const override;
	virtual _uint			Get_ModelProtoLevel() const override;
	virtual HRESULT			Bind_WorldMatrix() override;
	void					Update_LocalBounds();
	void					Refresh_ColliderPose();
	void					Rebuild_ColliderActor();

public:
	static CMapSection* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
