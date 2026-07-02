#pragma once

#include "PartObject.h"

#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CTexture;
NS_END

NS_BEGIN(Client)

class CKirby;
enum class KIRBY_EYE_STATE { IDLE, DOUBT, BLINK, CLOSE, ANGRY, SURPRISED, SADNESS, END };

class CKirby_Deform_Model abstract : public CPartObject
{
	GENERATED_BODY_ABSTRACT(CKirby_Deform_Model)

public:
	struct KIRBY_FORM_DESC : public CPartObject::PARTOBJECT_DESC
	{
		const _float* pHitFlash = { nullptr };
		const _float3* pHitFlashColor = { nullptr };
	};

protected:
	CKirby_Deform_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_Deform_Model(const CKirby_Deform_Model& Prototype);
	virtual ~CKirby_Deform_Model() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual HRESULT Ready_AnimEvents(CKirby* pKirby) = 0;

public:
	const _float4x4* Get_BoneMatrixPtr(const _char* pBoneName) const;

	CAnimator* Get_Animator() { return m_pAnimatorCom; }

public:
	void Set_KirbyEye(KIRBY_EYE_STATE eState) { m_eEye = eState; }
	KIRBY_EYE_STATE Get_KirbyEye() const { return m_eEye; }

public:
	virtual const _float4x4* Get_HatBoneMatirx();

protected:
	_bool Handle_AnimEventEye(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase);

	HRESULT Bind_ShaderResources(CShader* pShader);

	virtual HRESULT Render_PBRMesh(_uint iMeshIndex);
	virtual HRESULT Render_KirbyMesh(_uint iMeshIndex) = 0;

protected:
	CShader* m_pKirbyShaderCom{};
	CShader* m_pPBRShaderCom = { nullptr };

	CModel* m_pModelCom{};

	CAnimator* m_pAnimatorCom{};

	CTexture* m_pEyeTextureCom{};
	CTexture* m_pEyeMaskTextureCom{};

	KIRBY_EYE_STATE m_eEye{};

	const _float* m_pHitFlash = { nullptr };
	const _float3* m_pHitFlashColor = { nullptr };

protected:
	_float4 m_vBodyColor = { 1.f, 0.1882353f, 0.3764706f, 1.f };
	_float4 m_vFootColor = { 0.67f, 0.f, 0.f, 1.f };
	_float4 m_vBlushColor = { 1.f, 0.05f, 0.12f, 1.f };

protected:
	virtual void Free();
};

NS_END
