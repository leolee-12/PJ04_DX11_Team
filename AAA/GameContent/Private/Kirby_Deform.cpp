#include "Kirby_Deform.h"

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

void CKirby_Deform::Enter_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    // Model 교체
    CKirby_Deform_Model* pDeformModel_Demo = pKirby->Get_DeformPart_Model(DeformContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::DEMO);
    pDeformModel_Demo->Set_Active(false);
    CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(DeformContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
    pDeformModel_Main->Set_Active(true);

    // Animation
    pDeformModel_Main->Get_Animator()->Play("DemoEndFirst", false, true, 0.f, 3.5f);

    // 회전
    CMovement_Child* pMovement = pKirby->Get_Movement();
    constexpr _float fRotSpeed = 560.f;
    pMovement->Set_RotationSpeed(fRotSpeed);
    Set_RotationDir(pKirby);
}

_bool CKirby_Deform::Update_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext, _float fTimeDelta)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Rotate_To_Direction(XMLoadFloat3(&m_vRotationDir), fTimeDelta);

    CKirby_Deform_Model* pDeformModel_Main = pKirby->Get_DeformPart_Model(DeformContext.eDeformType, KIRBY_DEFORM_MODEL_TYPE::MAIN);
    CAnimator* pMainAnimator = pDeformModel_Main->Get_Animator();

    if (pMainAnimator->Is_Finished())
        return true;

    return false;
}

void CKirby_Deform::Exit_DeformState_Deform_End(CKirby* pKirby, const POST_DEFORM_END_CONTEXT& DeformContext)
{
    CMovement_Child* pMovement = pKirby->Get_Movement();
    pMovement->Set_RotationSpeed(CKirby::s_fRot_Speed_Degree);
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

void CKirby_Deform::Set_RotationDir(CKirby* pKirby)
{
    _vector vCamPos = XMLoadFloat4(m_pGameInstance_Proxy->Get_CamPosition());
    _vector vPlayerPos = pKirby->Get_Transform()->Get_State(STATE::POSITION);
    _vector vDir = XMVectorSetY(vCamPos - vPlayerPos, 0.f);
    vDir = XMVector3Normalize(vDir);

    _float fRadian = XMConvertToRadians(15.f);

    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vLeftDir = XMVector3Rotate(vDir, XMQuaternionRotationAxis(vUp, -fRadian));
    _vector vRightDir = XMVector3Rotate(vDir, XMQuaternionRotationAxis(vUp, fRadian));

    _vector vCurLook = XMVectorSetY(pKirby->Get_Transform()->Get_State(STATE::LOOK), 0.f);
    vCurLook = XMVector3Normalize(vCurLook);

    _float fLeftDot = XMVectorGetX(XMVector3Dot(vCurLook, vLeftDir));
    _float fRightDot = XMVectorGetX(XMVector3Dot(vCurLook, vRightDir));

    if (fLeftDot > fRightDot)
        XMStoreFloat3(&m_vRotationDir, vLeftDir);
    else
        XMStoreFloat3(&m_vRotationDir, vRightDir);
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
