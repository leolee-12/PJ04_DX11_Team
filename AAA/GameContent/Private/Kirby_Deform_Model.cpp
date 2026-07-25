#include "Kirby_Deform_Model.h"

#include "GameInstance.h"

#include "Effect_Container.h"
#include "Effect_Loader.h"
#include "Kirby.h"

CKirby_Deform_Model::CKirby_Deform_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject(pDevice, pContext)
{
}

CKirby_Deform_Model::CKirby_Deform_Model(const CKirby_Deform_Model& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CKirby_Deform_Model::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_Deform_Model::Initialize(void* pArg)
{
    if (pArg == nullptr)
    {
        MSG_BOX("pArg is nullptr: CKirby_Deform_Model");
        return E_FAIL;
    }

    KIRBY_DEFORM_MODEL_DESC* pDesc = static_cast<KIRBY_DEFORM_MODEL_DESC*>(pArg);

    m_pHitFlashIntensity = pDesc->pHitFlashIntensity;
    m_pHitFlashColor = pDesc->pHitFlashColor;

    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CKirby_Deform_Model::Update(_float fTimeDelta)
{
    if (m_bActive == false || m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_Deform_Model::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    CPartObject::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

const _float4x4* CKirby_Deform_Model::Get_BoneMatrixPtr(const _char* pBoneName) const
{
    if (m_pModelCom == nullptr)
    {
        MSG_BOX("m_pModelCom is nullptr: Kirby_Form");
        return nullptr;
    }

    return m_pModelCom->Get_BoneMatrixPtr(pBoneName);
}

const _float4x4* CKirby_Deform_Model::Get_HatBoneMatirx()
{
    return Get_BoneMatrixPtr("HatL");;
}

_bool CKirby_Deform_Model::Handle_AnimEventParent(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    if (Handle_AnimEventEye(e, ePhase) == true)
        return true;

    if (Handle_AnimEventSound(e, ePhase) == true)
        return true;

    if (Handle_AnimEventFx(pKirby, e, ePhase) == true)
        return true;

    return false;
}

_bool CKirby_Deform_Model::Handle_AnimEventFx(CKirby* pKirby, const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    // Begin, End는 Emitter 지속 이펙트용

    if (static_cast<EANIM_EVENT>(e.iEventType) != EANIM_EVENT::Fx)
        return false;

    if (e.strParam.empty())
        return true;

    const _wstring wstrEffectKey = StrToWstr(e.strParam);
    CEffect_Loader* pEffectLoader = CEffect_Loader::GetInstance();

    if (ePhase == ANIM_EVENT_PHASE::END)
    {
        auto iter = m_AnimEventEffects.find(wstrEffectKey);

        if (iter != m_AnimEventEffects.end())
        {
            FX_HANDLE& hEffect = iter->second;
            if (pEffectLoader->Is_Current(hEffect))
                hEffect.p->EffectContainer_StopAfterEmission();

            m_AnimEventEffects.erase(iter);
        }

        return true;
    }

    if (ePhase != ANIM_EVENT_PHASE::POINT && ePhase != ANIM_EVENT_PHASE::BEGIN)
        return true;

    if (ePhase == ANIM_EVENT_PHASE::BEGIN)
    {
        auto iter = m_AnimEventEffects.find(wstrEffectKey);
        if (iter != m_AnimEventEffects.end())
        {
            if (pEffectLoader->Is_Current(iter->second))
                return true;

            m_AnimEventEffects.erase(iter);
        }
    }

    CTransform* pKirbyTransform = pKirby->Get_Transform();

    const _vector vForward = XMVector3Normalize(XMVectorSetY(pKirbyTransform->Get_State(STATE::LOOK), 0.f));
    const _vector vRight = XMVector3Normalize(XMVectorSetY(pKirbyTransform->Get_State(STATE::RIGHT), 0.f));
    const _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vLocalLook = XMVectorZero();

    switch (static_cast<KIRBY_FX_DIRECTION>(e.iIntParam))
    {
        case KIRBY_FX_DIRECTION::NONE:
            break;
        case KIRBY_FX_DIRECTION::FORWARD:
            vLocalLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
            break;
        case KIRBY_FX_DIRECTION::BACKWARD:
            vLocalLook = XMVectorSet(0.f, 0.f, -1.f, 0.f);
            break;
        case KIRBY_FX_DIRECTION::RIGHT:
            vLocalLook = XMVectorSet(1.f, 0.f, 0.f, 0.f);
            break;
        case KIRBY_FX_DIRECTION::LEFT:
            vLocalLook = XMVectorSet(-1.f, 0.f, 0.f, 0.f);
            break;
        case KIRBY_FX_DIRECTION::BACKWARD_RIGHT:
            vLocalLook = XMVector3Normalize(XMVectorSet(1.f, 0.f, -1.f, 0.f));
            break;
        case KIRBY_FX_DIRECTION::BACKWARD_LEFT:
            vLocalLook = XMVector3Normalize(XMVectorSet(-1.f, 0.f, -1.f, 0.f));
            break;
    }

    if (ePhase == ANIM_EVENT_PHASE::POINT)
    {
        _float3 vPos{};
        XMStoreFloat3(&vPos, pKirbyTransform->Get_State(STATE::POSITION) +
            vRight * e.vOffset.x + vUp * e.vOffset.y + vForward * e.vOffset.z);

        _float3 vLook{};
        XMStoreFloat3(&vLook,
            vRight * XMVectorGetX(vLocalLook) +
            vUp * XMVectorGetY(vLocalLook) +
            vForward * XMVectorGetZ(vLocalLook));

        pEffectLoader->Spawn(wstrEffectKey, pKirby->Get_LevelIndex(), vPos, vLook);
        return true;
    }

    _float3 vLocalLookFloat{};
    XMStoreFloat3(&vLocalLookFloat, vLocalLook);

    FX_HANDLE hEffect{};
    const HRESULT hr = pEffectLoader->Spawn(wstrEffectKey, pKirby->Get_LevelIndex(),
        e.vOffset, vLocalLookFloat, _float3{}, pKirbyTransform->Get_WorldMatrixPtr(), nullptr, &hEffect);

    if (FAILED(hr) || !pEffectLoader->Is_Current(hEffect))
        return true;

    m_AnimEventEffects.emplace(wstrEffectKey, hEffect);

    return true;
}

_bool CKirby_Deform_Model::Handle_AnimEventEye(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    if (static_cast<EANIM_EVENT>(e.iEventType) != EANIM_EVENT::SetEye)
        return false;

    if (ePhase != ANIM_EVENT_PHASE::POINT)
        return true;

    switch (static_cast<KIRBY_EYE_STATE>(e.iIntParam))
    {
        case KIRBY_EYE_STATE::IDLE:      Set_KirbyEye(KIRBY_EYE_STATE::IDLE);      break;
        case KIRBY_EYE_STATE::DOUBT:     Set_KirbyEye(KIRBY_EYE_STATE::DOUBT);     break;
        case KIRBY_EYE_STATE::BLINK:     Set_KirbyEye(KIRBY_EYE_STATE::BLINK);     break;
        case KIRBY_EYE_STATE::CLOSE:     Set_KirbyEye(KIRBY_EYE_STATE::CLOSE);     break;
        case KIRBY_EYE_STATE::ANGRY:     Set_KirbyEye(KIRBY_EYE_STATE::ANGRY);     break;
        case KIRBY_EYE_STATE::SURPRISED: Set_KirbyEye(KIRBY_EYE_STATE::SURPRISED); break;
        case KIRBY_EYE_STATE::SADNESS:   Set_KirbyEye(KIRBY_EYE_STATE::SADNESS);   break;
    }

    return true;
}

_bool CKirby_Deform_Model::Handle_AnimEventSound(const ANIM_EVENT& tEvent, ANIM_EVENT_PHASE ePhase)
{
    if (static_cast<EANIM_EVENT>(tEvent.iEventType) != EANIM_EVENT::Sound)
        return false;

    if (tEvent.strParam.empty())
        return true;

    const _wstring wstrSoundKey = StrToWstr(tEvent.strParam);

    enum KIRBY_SOUND_TYPEP
    {
        ONCE, SECTION_LOOP,
    };

    switch (tEvent.iIntParam)
    {
        case KIRBY_SOUND_TYPEP::ONCE:
        {
            if (ePhase == ANIM_EVENT_PHASE::POINT)
            {
                m_pGameInstance_Proxy->Play_SFX(wstrSoundKey.c_str(), tEvent.vOffset.x);
            }
            else if (ePhase == ANIM_EVENT_PHASE::BEGIN)
            {
                auto iter = m_SoundHandles.find(wstrSoundKey);

                if (iter == m_SoundHandles.end())
                {
                    CSound_Handle pHandle = m_pGameInstance_Proxy->Play_SFX(wstrSoundKey.c_str(), tEvent.vOffset.x);
                    m_SoundHandles.emplace(wstrSoundKey, pHandle);
                }
            }
            else if (ePhase == ANIM_EVENT_PHASE::END)
            { 
                auto iter = m_SoundHandles.find(wstrSoundKey);

                if (iter != m_SoundHandles.end())
                {
                    iter->second.Stop();                    
                    m_SoundHandles.erase(iter);
                }
            }

            break;
        }
        case KIRBY_SOUND_TYPEP::SECTION_LOOP:
        {
            if (ePhase == ANIM_EVENT_PHASE::POINT)
            {
                auto iter = m_SoundHandles.find(wstrSoundKey);

                if (iter == m_SoundHandles.end())
                {
                    CSound_Handle pHandle = m_pGameInstance_Proxy->Play_SFX_Section_Loop(wstrSoundKey.c_str(), tEvent.vOffset.y, tEvent.vOffset.z, tEvent.vOffset.x);
                    m_SoundHandles.emplace(wstrSoundKey, pHandle);
                }
            }
            break;
        }
        return true;
    }

    return true;
}

void CKirby_Deform_Model::Stop_SoundHandle()
{
    for (auto pair : m_SoundHandles)
        pair.second.Stop();
    m_SoundHandles.clear();
}

HRESULT CKirby_Deform_Model::Bind_CommonShaderResources(CShader* pShader)
{
    if (pShader == nullptr)
    {
        MSG_BOX("pShader is nullptr: Kirby_Form");
        return E_FAIL;
    }

    if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;

    if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;
    if (FAILED(pShader->Bind_RawValue("g_iMaterialID", &m_iMaterialID, sizeof(_uint))))
        return E_FAIL;

    const _float fIntensity = m_pHitFlashIntensity ? *m_pHitFlashIntensity : 0.f;
    if (FAILED(pShader->Bind_RawValue("g_fHitFlash", &fIntensity, sizeof(fIntensity))))
        return E_FAIL;

    const _float3 vColor = m_pHitFlashColor ? *m_pHitFlashColor : _float3(1.f, 1.f, 1.f);
    if (FAILED(pShader->Bind_RawValue("g_vHitFlashColor", &vColor, sizeof(vColor))))
        return E_FAIL;

    return S_OK;
}

void CKirby_Deform_Model::Free()
{
    m_AnimEventEffects.clear();
    Stop_SoundHandle();

    __super::Free();
}
