#pragma once
#include "GameContent_Defines.h"
#include "GameContent_const.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader; class CModel; class CAnimator;
NS_END

NS_BEGIN(Client)

class CCutsceneActor abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CCutsceneActor)

protected:
    struct VISUAL_SETUP
    {
        SHADER_DESC   tShader;                   
        const _tchar* szModelProtoTag = nullptr;
        const _tchar* szAnimEventFile = nullptr; 
    };

    struct ATTACH_INFO
    {
        const _float4x4* pBoneMatrix = { nullptr };  // 소스 모델 로컬공간 본 행렬
        const _float4x4* pSourceWorld = { nullptr }; // 소스 액터 월드행렬(스케일 포함)
    };

protected:
    CCutsceneActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCutsceneActor(const CCutsceneActor& Prototype);
    virtual ~CCutsceneActor() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

public:
    CAnimator* Get_Animator() const { return m_pAnimatorCom; }
    CModel* Get_Model()    const { return m_pModelCom; }
    const _float4x4* Get_BoneMatrixPtr(const _char* pBoneName) const;

    void Play(const _char* szAnim, _bool bLoop = false, _float fBlend = 0.1f);

    void Attach_To(const ATTACH_INFO& tInfo) { m_tAttach = tInfo; m_bAttached = true; }
    void Detach() { m_bAttached = false; }

protected:
    HRESULT Ready_Visual(const VISUAL_SETUP& tSetup);
    virtual HRESULT Ready_Components() = 0;    
    virtual HRESULT Bind_ShaderResources();

    virtual void On_AnimEvent(const ANIM_EVENT& tEvent, ANIM_EVENT_PHASE ePhase) {}

protected:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };

    _bool       m_bAttached = { false };
    ATTACH_INFO m_tAttach = {};
    _float4x4   m_RenderWorld = {};     

protected:
    virtual void Free() override;
};

NS_END