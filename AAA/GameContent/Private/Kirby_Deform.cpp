#include "Kirby_Deform.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"
#include "Kirby_Ability.h"

#include "Movement_Child.h"

CKirby_Deform::CKirby_Deform()
{
}

HRESULT CKirby_Deform::Initialize()
{   
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_AniInfos.resize(ETOUI(DEFORM_ANI::END));

    return S_OK;
}

void CKirby_Deform::Enter_DeformState_Deform(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    m_pGameInstance_Proxy->Play_SFX(L"HeroBasic_DeformingSwallow1.wav", 0.2f);

    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_GravityScale(0.f);
    pMovement->Set_Velocity(XMVectorSet(0.f, 0.f, 0.f, 0.f));
    pMovement->Clear_Forces();

    m_pGameInstance_Proxy->Set_TimeScale(0.f);

    KIRBY_ABILITY_CHANGED tDesc{};
    tDesc.bBegin = true;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Ability_Changed, &tDesc);

    pKirby->Get_Body()->Set_Active(false);

    CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(DeformContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
    pDeformModel_Demo->Set_Active(true);
    pDeformModel_Demo->Get_Animator()->Play("Deform", false, true, 0.1f, 1.8f);

    pKirby->Change_HatSocketMatrix(pKirby->Get_KirbyAbility()->Get_AbilityType(),
        pDeformModel_Demo->Get_HatBoneMatirx());
}

void CKirby_Deform::On_DumpSpitStart(CKirby* pKirby)
{
    pKirby->Get_Movement()->Add_Velocity(XMVectorSet(0.f, 22.f, 0.f, 0.f));
}

void CKirby_Deform::Play_DeformAni(CKirby* pKirby, DEFORM_ANI eDeformAni)
{
    CKirby_Deform_Model* pDeformModelMain =
        pKirby->Get_DeformPart_Model(pKirby->Get_KirbyDeform()->Get_DeformType(), KIRBY_DEFORM_MODEL_TYPE::MAIN);

    const KIRBY_TYPE_ANI_DESC& tDesc = m_AniInfos[ETOUI(eDeformAni)];
    CAnimator* pAnimator = pDeformModelMain->Get_Animator();

    pAnimator->Play(&tDesc.tBaseAniInfo);

    if (tDesc.ePlayType == ANI_PLAY_TYPE::OVERLAY)
        pAnimator->Apply_Overlay(tDesc.tLayerAniInfo);
}

void CKirby_Deform::Set_FullBodyAni(DEFORM_ANI eAni, const _string& strAniName, _bool bLoop, _bool bRestart, _float fBlend, _float fSpeed)
{
    KIRBY_TYPE_ANI_DESC& desc = m_AniInfos[ETOUI(eAni)];
    desc.ePlayType = ANI_PLAY_TYPE::FULL_BODY;

    desc.tBaseAniInfo.strAniName = strAniName;
    desc.tBaseAniInfo.bLoop = bLoop;
    desc.tBaseAniInfo.bRestart = bRestart;
    desc.tBaseAniInfo.fBlend = fBlend;
    desc.tBaseAniInfo.fSpeed = fSpeed;
}

void CKirby_Deform::Free()
{
    __super::Free();
}
