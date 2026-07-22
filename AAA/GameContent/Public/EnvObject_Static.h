#pragma once
#include "EnvObject.h"
#include "BlendRenderable.h"

NS_BEGIN(Client)
class CEnv_InstanceController;
class CWorld_BlendCollector;

class CEnvObject_Static final
	: public CEnvObject
	, public IBlendRenderable
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvObject_Static";

private:
	CEnvObject_Static(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvObject_Static(const CEnvObject_Static& Prototype);
	virtual ~CEnvObject_Static() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render_BlendMesh(_uint iMeshIndex) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

#pragma region Editable
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) override;
#pragma endregion

public: // Editor preview
	void    Set_EditorForceMainPassNonInstanced(_bool bEnable) { m_bEditorForceMainPassNonInstanced = bEnable; }

public:	// Instance
	void	Set_InstanceController(CEnv_InstanceController* pCtrl);
	_bool	Can_RenderInstance() const;

private:
	CEnv_InstanceController* m_pInstanceController = { nullptr };
	ENV_INSTANCE_BATCH_HANDLE m_InstanceBatchHandle = {};

	CWorld_BlendCollector* m_pBlendCollector = { nullptr };
	vector<_uint> m_BlendMeshIndices;

	_bool m_bEditorForceMainPassNonInstanced = { false };

private:
	void Submit_RenderGroups();
	_bool Should_BypassMainInstance() const;
	void Cache_BlendMeshIndices();
	void Submit_BlendMeshes();

public:
	static CEnvObject_Static* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END