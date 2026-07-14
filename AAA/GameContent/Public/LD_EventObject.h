#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)

class CLD_EventObject abstract : public CLevelDesignObject
{
	GENERATED_BODY_ABSTRACT(CLD_EventObject)

protected:
	CLD_EventObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLD_EventObject(const CLD_EventObject& Prototype);
	virtual ~CLD_EventObject() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

	virtual _bool Is_CullingEnabled() const override { return false; }

protected:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };
	physx::PxRigidStatic* m_pRigidStatic = { nullptr };

	LD_EVENTOBJECT_DESC m_tEventObjectDesc = {};

	vector<_bool> m_MeshVisible;
	_bool m_bAnimationActive = { false };

	struct LD_ANIM_PLAY_DESC
	{
		_string strAnimName;
		_bool bLoop = { false };
		_float fSpeed = { 1.f };
	};

	vector<LD_ANIM_PLAY_DESC> m_AnimPlayDescs;

protected:
	virtual HRESULT Ready_Components();
	HRESULT Ready_RenderComponents();

	HRESULT Ready_AnimPlayDescs(const LD_ANIM_PLAY_DESC* pAnimDescs, _uint iCount);
	const LD_ANIM_PLAY_DESC* Find_AnimPlayDesc(const _string& strAnimName) const;
	HRESULT Set_AnimPose(const _string& strAnimName, _float fProgress);
	void Play_Anim(const _string& strAnimName);

	HRESULT Ready_AnimEvents();
	HRESULT Ready_RigidStatic();
	void    Release_RigidStatic();

	_int Find_MeshIndex_ByName(const _string& strMeshName) const;

	virtual void On_AnimEvent(const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase);

	void Set_AllMeshesVisible(_bool bVisible);
	void Set_MeshVisible(_uint iMeshIndex, _bool bVisible);

	virtual _bool Should_RenderMesh(_uint iMeshIndex) const;
	virtual _uint Resolve_RenderPass(_uint iMeshIndex) const;
	virtual HRESULT Bind_BoneMatrices(_uint iMeshIndex);

	HRESULT Bind_ShaderResources();
	HRESULT Render_Mesh(_uint iMeshIndex);
	HRESULT Render_Mesh(_uint iMeshIndex, _uint iAnimPassIndex);

protected:
	virtual void Free() override;
};

NS_END