#include "Boss_Cage.h"
#include "Boss_Cage_Body.h"
#include "Cage_WaddleDee.h"

#include "GameInstance.h"

CBoss_Cage::CBoss_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{
}

CBoss_Cage::CBoss_Cage(const CBoss_Cage& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CBoss_Cage::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoss_Cage::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    XMStoreFloat4x4(&m_AttachOffset, XMMatrixIdentity());

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    if (FAILED(Ready_BreakTrigger()))
        return E_FAIL;

    return S_OK;
}

void CBoss_Cage::Priority_Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    __super::Priority_Update(fTimeDelta);
}

void CBoss_Cage::Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    __super::Update(fTimeDelta);

    switch (m_eState)
    {
        case CAGE_STATE::HANGING:
            if (!m_bAnimPrimed)
            {
                m_pBody->Get_Animator()->Pause();
                m_bAnimPrimed = true;
            }
            break;

        case CAGE_STATE::DESCEND:
        {
            _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
            _float  fNewY = XMVectorGetY(vPos) - m_fDescendSpeed * fTimeDelta;

            if (fNewY <= m_fFloatHeight)
            {
                fNewY = m_fFloatHeight;
                m_eState = CAGE_STATE::FLOAT_IDLE;
            }
            m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetY(vPos, fNewY));
            break;
        }

        case CAGE_STATE::FLOAT_IDLE:
            break;

        case CAGE_STATE::BREAKING:
            if (m_pBody->Get_Animator()->Is_Finished())
            {
                m_pBody->Set_Active(false);
                Fire_CutsceneCamera(L"ClearDanceLong_Camera", Get_DeeAnimator());

                CUTSCENE_STAGECLEAR Desc{};
                Desc.AnchorWorld = *(m_pTransformCom->Get_WorldMatrixPtr());
                Desc.fAnimSpeed = 1.5f;
                Desc.fBlendDuration = 0.f;
                Desc.eAnim = STAGECLEAR_ANIM::DANCE;
                m_pGameInstance_Proxy->Publish(EventTag::Cutscene_StageClear, &Desc);

                m_eState = CAGE_STATE::BROKEN;
            }
            break;

        case CAGE_STATE::BROKEN:
            // 임시로 박음 나중에 ClearUI로 옮기삼
            if (!m_bHeadTurnFired)
            {
                CAnimator* pDeeAnim = Get_DeeAnimator();
                if (pDeeAnim && pDeeAnim->Is_Finished())
                {
                    m_bHeadTurnFired = true;
                    m_pGameInstance_Proxy->Publish(EventTag::StageClear_UIStarted, nullptr);
                }
            }
            break;
    }
}

void CBoss_Cage::Late_Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    // Late_Update에서 합성해야 소유자 본 갱신(Update 단계)이 끝난 뒤가 보장됨
    if (m_pAttachBone && m_pAttachOwnerWorld)
    {
        _matrix World = XMLoadFloat4x4(&m_AttachOffset)
            * XMLoadFloat4x4(m_pAttachBone)
            * XMLoadFloat4x4(m_pAttachOwnerWorld);
        m_pTransformCom->Set_WorldMatrix(World);
    }

    __super::Late_Update(fTimeDelta);

    if (CAGE_STATE::FLOAT_IDLE == m_eState && m_pBreakTrigger && m_pBreakTrigger->Is_Enabled())
    {
        m_pBreakTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(m_pBreakTrigger);
#endif
    }
}

HRESULT CBoss_Cage::Ready_PartObjects()
{
    CMonsterPart::MONSTERPART_DESC Desc{};
    Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();   // 루트가 될 행렬을 합성

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CBoss_Cage_Body::PROTOTYPE_TAG,
        CBoss_Cage_Body::PART_TAG, &Desc)))
        return E_FAIL;

    m_pBody = static_cast<CBoss_Cage_Body*>(m_PartObjects.find(CBoss_Cage_Body::PART_TAG)->second);
    if (!m_pBody)
        return E_FAIL;

    const _char* szSocketBones[3] = { "DeeFrontL", "DeeLeftL", "DeeRightL" };
    const _tchar* szPartTags[3] = { L"Part_DeeFront", L"Part_DeeLeft", L"Part_DeeRight" };
    const _float3 vOffsets[3] = { { 0.f, 0.f, 0.5f }, { -0.5f, 0.f, -0.3f }, { 0.5f, 0.f, -0.3f } };

    for (_uint i = 0; i < 3; ++i)
    {
        CCage_WaddleDee::WADDLEDEE_DESC DeeDesc{};
        DeeDesc.ePos = static_cast<CCage_WaddleDee::DEE_POS>(i);
        DeeDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
        DeeDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr(szSocketBones[i]);
#ifdef _DEBUG
        if (nullptr == DeeDesc.pSocketBoneMatrix)
            MSG_BOX("Cage: Dee socket bone not found");
#endif
        XMStoreFloat4x4(&DeeDesc.InitialLocal, XMMatrixRotationY(XMConvertToRadians(180)));

        if (FAILED(Add_PartObject(m_iPrototypeLevel, CCage_WaddleDee::PROTOTYPE_TAG,
            szPartTags[i], &DeeDesc)))
            return E_FAIL;

        m_WaddleDees[i] = static_cast<CCage_WaddleDee*>(m_PartObjects.find(szPartTags[i])->second);
    }

    return S_OK;
}

HRESULT CBoss_Cage::Ready_BreakTrigger()
{
    CCollider::COLLIDER_DESC Desc{};
    Desc.pOwner = this;
    Desc.fRadius = 1.5f;                      
    Desc.vCenter = _float3(0.f, 1.5f, 0.f);    

    m_pBreakTrigger = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
        TEXT("Com_BreakTrigger"), &Desc);
    if (nullptr == m_pBreakTrigger)
        return E_FAIL;

    m_pGameInstance_Proxy->Register_Collider(m_pBreakTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

    m_pBreakTrigger->Set_OnEnter([this](CCollider* pOther)
        {
            if (nullptr == pOther)
                return;
            if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
                return;

            Break();
        });

    return S_OK;
}

void CBoss_Cage::Fire_CutsceneCamera(const _tchar* szTrack, CAnimator* pProgress)
{
    CUTSCENE_CAMERA_DESC cam{};
    cam.eCam = ECutsceneCam::Cutscene;
    cam.szTrack = szTrack;
    cam.pProgress = pProgress;                               
    cam.pAnchorWorld = m_pTransformCom->Get_WorldMatrixPtr();
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

CAnimator* CBoss_Cage::Get_DeeAnimator() const
{
    for (auto pDee : m_WaddleDees)
    {
        if (pDee)
            return pDee->Get_Animator();
    }
    return nullptr;
}

void CBoss_Cage::Rescue_WaddleDees()
{
    for (auto pDee : m_WaddleDees)
    {
        if (pDee)
            pDee->Rescue();
    }
}

_bool CBoss_Cage::Is_RescueDone() const
{
    for (auto pDee : m_WaddleDees)
    {
        if (pDee && !pDee->Is_Done())
            return false;
    }
    return true;
}

void CBoss_Cage::Start_Descend(_fvector vLookTarget, _float fSpawnHeightOffset)
{
    if (m_pAttachOwnerWorld)
        m_fFloatHeight = m_pAttachOwnerWorld->m[3][1];
    else
        m_fFloatHeight = XMVectorGetY(m_pTransformCom->Get_State(STATE::POSITION));

    Detach();

    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    vPos = XMVectorSetY(vPos, m_fFloatHeight + fSpawnHeightOffset);
    m_pTransformCom->Set_WorldMatrix(XMMatrixTranslationFromVector(vPos));

    _vector vFlatTarget = XMVectorSetY(vLookTarget, XMVectorGetY(vPos));
    vFlatTarget = XMVectorSetW(vFlatTarget, 1.f);
    m_pTransformCom->LookAt(vFlatTarget);

    m_pBody->Set_RenderBird(true);
    m_pBody->Get_Animator()->Resume();
    m_eState = CAGE_STATE::DESCEND;
}

void CBoss_Cage::Break()
{
    if (CAGE_STATE::FLOAT_IDLE != m_eState)
        return;

    m_pBreakTrigger->Set_Enabled(false);                    

    m_pBody->Get_Animator()->Play("Cut1", false, true, 0.f, 1.5f);     
    Rescue_WaddleDees();                                    
    m_eState = CAGE_STATE::BREAKING;
    Fire_CutsceneCamera(L"Cut1Rescue_Camera", m_pBody->Get_Animator());

    CUTSCENE_STAGECLEAR Desc{};
    Desc.AnchorWorld = *(m_pTransformCom->Get_WorldMatrixPtr());
    Desc.fAnimSpeed = 1.5f;
    Desc.fBlendDuration = 0.f;
    Desc.eAnim = STAGECLEAR_ANIM::CUT1;
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_StageClear, &Desc);

    _bool bShow = false;
    m_pGameInstance_Proxy->Publish(EventTag::HUD_SetVisible, &bShow);
}

void CBoss_Cage::Attach_To_Bone(const _float4x4* pBoneMatrix, const _float4x4* pOwnerWorld, _fmatrix OffsetMatrix)
{
    m_pAttachBone = pBoneMatrix;
    m_pAttachOwnerWorld = pOwnerWorld;
    XMStoreFloat4x4(&m_AttachOffset, OffsetMatrix);
}

void CBoss_Cage::Detach()
{
    m_pAttachBone = nullptr;
    m_pAttachOwnerWorld = nullptr;
}

CBoss_Cage* CBoss_Cage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Cage* pInstance = new CBoss_Cage(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBoss_Cage");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBoss_Cage::Clone(void* pArg)
{
    CBoss_Cage* pInstance = new CBoss_Cage(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBoss_Cage");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Cage::Free()
{
    __super::Free();
}