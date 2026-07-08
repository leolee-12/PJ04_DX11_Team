#include "Kirby_Deform_Model.h"

#include "GameInstance.h"

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

    KIRBY_FORM_DESC* pDesc = static_cast<KIRBY_FORM_DESC*>(pArg);

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

_bool CKirby_Deform_Model::Handle_AnimEventParent(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    if (Handle_AnimEventEye(e, ePhase) == true)
        return true;

    if (Handle_AnimEventSound(e, ePhase) == true)
        return true;

    return false;
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

_bool CKirby_Deform_Model::Handle_AnimEventSound(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    if (static_cast<EANIM_EVENT>(e.iEventType) != EANIM_EVENT::Sound)
        return false;

    if (ePhase != ANIM_EVENT_PHASE::POINT)
        return true;

    if (e.strParam.empty())
        return true;

    enum KIRBY_SOUND_TYPEP { DEFAULT, SECTION_LOOP, SECTION_LOOP_STOP };

    const _wstring wstrSoundKey = StrToWstr(e.strParam);

    switch(e.iIntParam)
    {
        case KIRBY_SOUND_TYPEP::DEFAULT:
        {
            m_pGameInstance_Proxy->Play_SFX(wstrSoundKey.c_str(), e.vOffset.x);
            break;
        }
        case KIRBY_SOUND_TYPEP::SECTION_LOOP:
        {
            auto iter = m_SoundHandles.find(wstrSoundKey);

            if (iter == m_SoundHandles.end())
            {
                CSound_Handle pHandle = m_pGameInstance_Proxy->Play_SFX_Section_Loop(wstrSoundKey.c_str(), e.vOffset.y, e.vOffset.z, e.vOffset.x);
                m_SoundHandles.emplace(wstrSoundKey, pHandle);
            }
            break;
        }
        case KIRBY_SOUND_TYPEP::SECTION_LOOP_STOP:
        {
            auto iter = m_SoundHandles.find(wstrSoundKey);

            if (iter != m_SoundHandles.end())
            {
                iter->second.Stop();
                m_SoundHandles.erase(iter);
            }
            break;
        }
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
    Stop_SoundHandle();

    __super::Free();
}