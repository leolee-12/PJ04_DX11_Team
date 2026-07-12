#pragma once
#include "GameObject.h"
#include "GameContent_Defines.h"
#include "EnvObject_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)
class CEnvObject_Static;

class CEnv_InstanceBatch final : public CGameObject
{
public:
	struct ENV_INSTANCE_BATCH_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		ENV_INSTANCE_KEY tKey = {};
	};

	struct INSTANCE_SUBMIT_DATA
	{
		CEnvObject_Static* pObj = { nullptr };
		ENV_INSTANCE_DATA InstanceData = {};
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Env_InstanceBatch";

private:
	CEnv_InstanceBatch(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnv_InstanceBatch(const CEnv_InstanceBatch& Prototype);
	virtual ~CEnv_InstanceBatch() = default;

public:
	void	Submit(CEnvObject_Static* pObj, _uint64 iCurrentFrame);
	HRESULT	Reserve_InstanceBuffer(_uint iRequiredCount);
	HRESULT	Update_InstanceBuffer();
	virtual	HRESULT Render() override;
	virtual	HRESULT Render_Shadow() override;
	virtual HRESULT Render_Decal() override;
	virtual	void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	_bool	Is_RegisteredThisFrame() { return m_bRegisteredThisFrame; }
	void	Set_RegisteredThisFrame(_bool b) { m_bRegisteredThisFrame = b; }

	HRESULT Apply_MeshLayer(_uint iMesh, const MESH_LAYER_IDX& Layer);

private:
	ENV_INSTANCE_KEY m_tKey = {};

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	ID3D11Buffer* m_pInstanceBuffer = { nullptr };
	_uint m_iInstanceCapacity = { 0u };

	vector<INSTANCE_SUBMIT_DATA> m_Submitted;

	_bool m_bRegisteredThisFrame = { false };
	_uint64 m_iLastSubmitFrame = { 0u };

private:
	virtual HRESULT Initialize(void* pArg) override;
	HRESULT Ready_Component();

	void BeginFrame_IfNeeded(_uint64 iCurrentFrame);
	_bool Should_Instance() const;
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShadowShaderResources();
	void Clear_Submissions();

	HRESULT Render_Instanced();
	HRESULT Render_NotInstanced();
	HRESULT Render_Decal_Instanced();
	HRESULT Render_Decal_NotInstanced();
	HRESULT Render_Shadow_Instanced();
	HRESULT Render_Shadow_NotInstanced();

public:
	static CEnv_InstanceBatch* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END