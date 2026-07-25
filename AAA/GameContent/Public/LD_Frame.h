#pragma once
#include "LevelDesignObject.h"
#include "BlendRenderable.h"
#include "MeshLayer_Binder.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;
class CWorld_BlendCollector;

class CLD_Frame final
	: public CLevelDesignObject
	, public IBlendRenderable
{
	GENERATED_BODY(CLD_Frame)

public:
	static constexpr const _tchar* OBJECT_NAME = L"CreditKirbyHouseFrame";
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Frame";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_Frame";
	static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";
	static constexpr const _char* MODEL_PATH = "../../Resources/Map/Gimmick/NonAnim/CreditKirbyHouseFrame/CreditKirbyHouseFrame.ysh";

private:
	CLD_Frame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_Frame(const CLD_Frame& Prototype);
	virtual ~CLD_Frame() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	virtual HRESULT Render_BlendMesh(_uint iMeshIndex) override;
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CWorld_BlendCollector* m_pBlendCollector = { nullptr };

	LD_STATIC_MODEL_DESC m_tStaticModelDesc = {};
	vector<_uint> m_BlendMeshIndices;

private:
	HRESULT Ready_RenderComponents();
	HRESULT Bind_ShaderResources();
	HRESULT Render_Mesh(_uint iMeshIndex, MESH_LAYER_RENDER_KIND eKind);
	void Cache_BlendMeshIndices();
	void Submit_BlendMeshes();

public:
	static CLD_Frame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END