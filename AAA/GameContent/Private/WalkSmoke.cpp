#include "WalkSmoke.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeSphereOriginal.h"
#include "SmokeLowPoly.h"
#include "SmokeTail.h"

#include "TestParticle.h"
#include "TestMeshParticle.h"
#include "TestMeshEmitter.h"

CWalkSmoke::CWalkSmoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
    Init_PropertyValue();
}

CWalkSmoke::CWalkSmoke(const CWalkSmoke& Prototype)
    : CEffect_Container(Prototype)
{
    Init_PropertyValue();
}

HRESULT CWalkSmoke::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CWalkSmoke::Initialize(void* pArg)
{
    WALK_SMOKE_DESC* pDesc = static_cast<WALK_SMOKE_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CWalkSmoke::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CWalkSmoke::Update(_float fTimeDelta)
{
    Update_Move(fTimeDelta);
    __super::Update(fTimeDelta);
}

void CWalkSmoke::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CWalkSmoke::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CWalkSmoke::Ready_EffectPartObjects()
{
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeSphereOriginal::PROTOTYPE_TAG, TEXT("Proto_SmokeSphereOriginal"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeLowPoly::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeLowPoly_1"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeLowPoly::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeLowPoly_2"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeLowPoly::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeLowPoly_3"))))
        return E_FAIL;
    if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CSmokeTail::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeTail"))))
        return E_FAIL;

    //if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CTestMeshEmitter::PROTOTYPE_TAG, TEXT("Test_MeshEmitter"))))
    //    return E_FAIL;
    //if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CTestParticle::PROTOTYPE_TAG, TEXT("Test_Paticle"))))
    //    return E_FAIL;
    //if (FAILED(Add_Effect_PartObject(m_iPrototypeLevel, CTestMeshParticle::PROTOTYPE_TAG, TEXT("Test_MeshPaticle"))))
    //    return E_FAIL;

    return S_OK;
}

void CWalkSmoke::Update_Move(_float fTimeDelta)
{
    if (m_bIsPlay == false || m_fDuration <= Helper::fEpsilon ||
        fabsf(m_fWalkSmokeMoveSpeed) <= Helper::fEpsilon || fTimeDelta <= 0.f)
        return;

    _float fStartRatio = std::clamp(m_fWalkSmokeMoveStartRatio, 0.f, 1.f);
    _float fEndRatio = std::clamp(m_fWalkSmokeMoveEndRatio, 0.f, 1.f);

    if (fStartRatio > fEndRatio)
        std::swap(fStartRatio, fEndRatio);

    const _float fMoveStartTime = fStartRatio * m_fDuration;
    const _float fMoveEndTime = fEndRatio * m_fDuration;
    const _float fFrameStartTime = std::clamp(m_fAccTime, 0.f, m_fDuration);
    const _float fFrameEndTime = (std::min)(fFrameStartTime + fTimeDelta, m_fDuration);

    const _float fActiveStartTime = (std::max)(fFrameStartTime, fMoveStartTime);
    const _float fActiveEndTime = (std::min)(fFrameEndTime, fMoveEndTime);
    const _float fActiveDelta = (std::max)(0.f, fActiveEndTime - fActiveStartTime);

    if (fActiveDelta <= 0.f)
        return;

    const _float fMoveDuration = fMoveEndTime - fMoveStartTime;
    if (fMoveDuration <= Helper::fEpsilon)
        return;

    const _float fLocalStartRatio = (fActiveStartTime - fMoveStartTime) / fMoveDuration;
    const _float fLocalEndRatio = (fActiveEndTime - fMoveStartTime) / fMoveDuration;
    const _float fLocalMidRatio = (fLocalStartRatio + fLocalEndRatio) * 0.5f;

    const _float fStartSpeedScale = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fLocalStartRatio);
    const _float fMidSpeedScale = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fLocalMidRatio);
    const _float fEndSpeedScale = 1.f - Helper::FloatSmoothStep(0.f, 1.f, fLocalEndRatio);
    const _float fSmoothedDelta = fActiveDelta *
        (fStartSpeedScale + 4.f * fMidSpeedScale + fEndSpeedScale) / 6.f;

    _vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
    if (XMVectorGetX(XMVector3LengthSq(vLook)) <= Helper::fEpsilon)
        return;

    _vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
    vPosition += XMVector3Normalize(vLook) * m_fWalkSmokeMoveSpeed * fSmoothedDelta;
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vPosition, 1.f));
}

void CWalkSmoke::Init_PropertyValue()
{
    m_fWalkSmokeMoveSpeed = 0.f;
    m_fWalkSmokeMoveStartRatio = 0.f;
    m_fWalkSmokeMoveEndRatio = 1.f;
}

CWalkSmoke* CWalkSmoke::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CWalkSmoke* pInstance = new CWalkSmoke(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CWalkSmoke");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CWalkSmoke::Clone(void* pArg)
{
    CWalkSmoke* pInstance = new CWalkSmoke(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CWalkSmoke");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CWalkSmoke::Free()
{
    __super::Free();
}
