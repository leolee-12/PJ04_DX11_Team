#pragma once

#include "PartObject.h"

#include "GameContent_const.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CTexture;

class CSound_Handle;
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
		const _float* pHitFlashIntensity{};
		const _float3* pHitFlashColor{};
	};

protected:
	CKirby_Deform_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_Deform_Model(const CKirby_Deform_Model& Prototype);
	virtual ~CKirby_Deform_Model() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

public:
	const _float4x4* Get_BoneMatrixPtr(const _char* pBoneName) const;
	virtual const _float4x4* Get_HatBoneMatirx();

	CAnimator* Get_Animator() { return m_pAnimatorCom; }

	virtual HRESULT Ready_AnimEvents(CKirby* pKirby) = 0;

public:
	void Set_KirbyEye(KIRBY_EYE_STATE eState) { m_eEye = eState; }
	void Stop_SoundHandle();

protected:
	_bool Handle_AnimEventEye(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase);
	_bool Handle_AnimEventSound(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase);

	HRESULT Bind_CommonShaderResources(CShader* pShader);

protected:
	CShader* m_pKirbyShaderCom{};

	CModel* m_pModelCom{};

	CAnimator* m_pAnimatorCom{};

	CTexture* m_pEyeTextureCom{};
	CTexture* m_pEyeMaskTextureCom{};

	KIRBY_EYE_STATE m_eEye{};

	const _float* m_pHitFlashIntensity{};
	const _float3* m_pHitFlashColor{};

	// Sound
	unordered_map<_wstring, CSound_Handle> m_SoundHandles;

protected:
	static constexpr _float4 s_vBodyColor{ 1.f, 0.1882353f, 0.3764706f, 1.f };
	static constexpr _float4 s_vFootColor{ 0.67f, 0.f, 0.f, 1.f };
	static constexpr _float4 s_vBlushColor{ 1.f, 0.05f, 0.12f, 1.f };

protected:
	virtual void Free();
};

NS_END
