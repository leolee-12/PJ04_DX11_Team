#include "Kirby_Deform.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Deform::CKirby_Deform()
{
}

HRESULT CKirby_Deform::Initialize()
{   
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    m_tDeformAniInfos.resize(ETOUI(DEFORM_ANI::END));

    return S_OK;
}

void CKirby_Deform::Play_DeformAni(CKirby* pKirby, DEFORM_ANI eDeformAni)
{
    CKirby_Deform_Model* pDeformModelMain =
        pKirby->Get_DeformPart_Model(pKirby->Get_KirbyDeform()->Get_DeformType(),KIRBY_DEFORM_MODEL_TYPE::MAIN);

    const DEFORM_ANI_DESC& tDesc = m_tDeformAniInfos[ETOUI(eDeformAni)];
    pDeformModelMain->Get_Animator()->Play(&tDesc.tBaseAniInfo);

    // Overlay 있으면 추가
}

void CKirby_Deform::Set_FullBodyAni(DEFORM_ANI eAni, const _string& strAniName, _bool bLoop, _bool bRestart, _float fBlend, _float fSpeed)
{
    DEFORM_ANI_DESC& desc = m_tDeformAniInfos[ETOUI(eAni)];
    desc.ePlayType = DEFORM_ANI_PLAY_TYPE::FULL_BODY;

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
