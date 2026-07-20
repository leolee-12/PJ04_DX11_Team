#pragma once
#include "LevelDesign_LoadTypes.h"
#include "Editable.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCullingState;
NS_END

NS_BEGIN(Client)
struct MESH_LAYER_BIND_CONTEXT;
enum class MESH_LAYER_PROFILE : _uint;

class CLevelDesignObject abstract : public CGameObject, public IEditable
{
	GENERATED_BODY_ABSTRACT(CLevelDesignObject)

protected:
	CLevelDesignObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesignObject(const CLevelDesignObject& Prototype);
	virtual ~CLevelDesignObject() = default;

	virtual		HRESULT Initialize_Prototype() override;
	virtual		HRESULT Initialize(void* pArg) override;
	virtual		HRESULT Validate_Initialized();
	_wstring	Make_LevelDesignObjectKey() const;

public:
	const LD_OBJECT_DESC& Get_LevelDesignDesc() const { return m_tLevelDesignDesc; }
	virtual _bool Is_CullingEnabled() const { return true; }
	virtual _bool Is_CullTransformDynamic() const { return false; }

#pragma region Editable
public:
	virtual _bool Get_EditDesc(EDITABLE_DESC* pOutDesc) const override;
	virtual HRESULT Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy) override;
	virtual HRESULT On_EditTransformChanged() override;
	virtual HRESULT Apply_EditCustomDesc(const EDIT_CUSTOM_DESC& /*Desc*/) override { return E_FAIL; }
	virtual const MESH_LAYER_IDX* Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const override;
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) override;
#pragma endregion

protected:
	LD_OBJECT_DESC  m_tLevelDesignDesc = {};
	CCullingState* m_pCullingState = { nullptr };

	_bool	m_bVisible = { true };
	_bool	m_bVisibleShadow = { false };
	_bool	m_bUseShadow = { false };

protected:
	virtual HRESULT	Ready_Events() override;
	virtual void	Build_EditCapabilities(_uint* pOutCaps, EDIT_OBJECT_POLICY* pOutPolicy) const;
	virtual void	Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const;
	virtual HRESULT	On_ApplyEditPolicy(const EDIT_OBJECT_POLICY& Policy);

	HRESULT Ready_CullingState(CModel* pModel, _float fBoundsMargin = 0.f, _bool bRotationInvariant = false);
	HRESULT Ready_CullingState(const BoundingBox& LocalBounds, _bool bRotationInvariant = false);
	void	Check_Visible();
	void    Submit_RenderGroups(RENDERID eMainID = RENDERID::NONBLEND);
	HRESULT Bind_ShadowTransforms(CShader* pShader, const _float4x4* pWorldOverride = nullptr) const;
	HRESULT Render_ShadowMesh(CShader* pShader, CModel* pModel, _uint iMeshIndex, MESH_LAYER_PROFILE eProfile) const;	// Anim 개별 구현용
	HRESULT Render_ShadowModel(CShader* pShader, CModel* pModel, MESH_LAYER_PROFILE eProfile, const _float4x4* pWorldOverride = nullptr) const;	// NonAnim 공통
	void	Add_EditModelSlot(vector<EDITABLE_MODEL_SLOT>* pOutSlots, const _tchar* pLabel, EDITABLE_MODEL_KIND eKind, CModel* pModel) const;
	_bool	Compute_EffectSpawnPosition(CModel* pModel, _float fHeightRatio, _float3* pOutPosition) const;

	void    Publish_LDEvent();
	virtual void On_LDEventReceived(const _wstring&) {}

protected:
	virtual void Free() override;
};

NS_END