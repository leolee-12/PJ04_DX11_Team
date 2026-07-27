#pragma once
#include "LevelDesignObject.h"
#include "BlendRenderable.h"
#include "MeshLayer_Binder.h"

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;
class CWorld_BlendCollector;

class CLD_BattleBoundary final
	: public CLevelDesignObject
	, public IBlendRenderable
{
	GENERATED_BODY(CLD_BattleBoundary)

public:
	static constexpr const _tchar* OBJECT_NAME = L"BattleBoundary";
	static constexpr const _tchar* OBJECT_NAME_CYLINDRICAL = L"BattleBoundaryCylindrical";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_BattleBoundary";
	static constexpr const _tchar* PROTOTYPE_TAG_CYLINDRICAL = L"Proto_LevelDesign_BattleBoundaryCylindrical";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_BattleBoundary";
	static constexpr const _tchar* MODEL_PROTO_TAG_CYLINDRICAL = L"Proto_Component_Model_BattleBoundaryCylindrical";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

	static constexpr const _char* MODEL_PATH = "../../Resources/Map/Gimmick/NonAnim/BattleBoundary/BattleBoundary.ysh";
	static constexpr const _char* MODEL_PATH_CYLINDRICAL = "../../Resources/Map/Gimmick/NonAnim/BattleBoundary/BattleBoundaryCylindrical.ysh";

private:
	CLD_BattleBoundary(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_BattleBoundary(const CLD_BattleBoundary& Prototype);
	virtual ~CLD_BattleBoundary() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_BlendMesh(_uint iMeshIndex) override;
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;
	virtual _bool   Is_CullingEnabled() const override { return false; }
	virtual void    Deserialize_Internal(const json& j) override;
	virtual HRESULT On_EditTransformChanged() override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static _bool Make_DefaultDesc(const LD_OBJECT_DESC& CommonDesc, _uint iModelProtoLevel, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual HRESULT Ready_Events() override;

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CWorld_BlendCollector* m_pBlendCollector = { nullptr };
	physx::PxRigidStatic* m_pRigidStatic = { nullptr };

	LD_BATTLE_BOUNDARY_DESC	m_tBattleBoundaryDesc = {};

	vector<_uint>	m_BlendMeshIndices;
	_float			m_fPlacedPositionY = { 0.f };
	_float			m_fActivationRatio = { 0.f };
	_float			m_fTargetActivationRatio = { 0.f };

private:
	HRESULT Ready_RenderComponents();
	HRESULT Bind_ShaderResources();
	HRESULT Render_Mesh(_uint iMeshIndex, MESH_LAYER_RENDER_KIND eKind);
	void	Cache_BlendMeshIndices();
	void	Submit_BlendMeshes();
	HRESULT Ready_PhysicsActor();
	void    Rebuild_PhysicsActor();
	void    Sync_PhysicsActor();
	void    Release_PhysicsActor();

public:
	static CLD_BattleBoundary* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END