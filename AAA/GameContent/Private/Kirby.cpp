#include "Kirby.h"

#include "GameInstance.h"

#include "PartObject.h"
#include "Monster.h"
#include "Controller.h"

#include "GameContent_const.h"
#include "Movement_Child.h"

// Parts
#include "Kirby_Body.h"
#include "Kirby_Sword.h"
#include "Kirby_SwordHat.h"

#include "Kirby_DeformCar_Demo.h"
#include "Kirby_DeformCar_Main.h"

#include "Kirby_InputManager.h"
#include "Kirby_Controller.h"
#include "Kirby_StateMachine.h"

// Ability
#include "Kirby_State.h"
#include "Kirby_Ability_Normal.h"
#include "Kirby_Ability_Sword.h"

// Deform
#include "Kirby_Deform_Car.h"

// Ladder
#include "LevelDesign_Ladder.h"

#include "Effect_Loader.h"

CKirby::CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CKirby::CKirby(const CKirby& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CKirby::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    if (FAILED(Ready_Ability()))
        return E_FAIL;

    if (FAILED(Ready_Deform()))
        return E_FAIL;

    if (FAILED(Ready_System()))
        return E_FAIL;

    SetUp_Collider_Callback();

    // Part 생성된 후
    if (FAILED(Ready_AnimEvents()))
        return E_FAIL;  

    return S_OK;
}

void CKirby::Priority_Update(_float fTimeDelta)
{
    fTimeDelta = Resolve_TimeDelta(fTimeDelta);
    __super::Priority_Update(fTimeDelta);
}

void CKirby::Update(_float fTimeDelta)
{
    fTimeDelta = Resolve_TimeDelta(fTimeDelta);

    XMStoreFloat3(&m_vWishDir, XMVectorZero());

    Update_Timer(fTimeDelta);

    m_pKirby_InputManager->Update_KirbyInput(fTimeDelta);
    m_pKirby_Controller->Update_KirbyController(fTimeDelta);
    m_pKirby_StateMachine->Update_StateMachine(fTimeDelta);

    if (m_pGameInstance_Proxy->Is_EditMode())
    {
        m_pMovement->Sync_To_Controller();
        return;
    }

    if(Has_MoveDir())
    {
        _vector vDir = XMLoadFloat3(&m_vWishDir);
        m_pMovement->Add_Acceleration(vDir * 120.f);
        if(!m_RotationLock)
            m_pMovement->Rotate_To_Direction(vDir, fTimeDelta);
    }

    m_pMovement->Update_RigidBody(fTimeDelta);

    __super::Update(fTimeDelta);

    if (m_fInvincibleTime > 0.f)
    {
        _float fElapsed = s_fInvincibleDuration - m_fInvincibleTime;
        if (fElapsed < 0.12f)
        {
            m_fHitFlashCur = 1.f;
        }
        else
        {
            const _float fBlinkHz = 8.f;
            _float fBlink = (fmodf(m_fInvincibleTime * fBlinkHz, 1.f) < 0.5f) ? 1.f : 0.f;
            m_fHitFlashCur = fBlink * 0.1f;
        }
    }
    else
        m_fHitFlashCur = 0.f;
}

void CKirby::Late_Update(_float fTimeDelta)
{
    fTimeDelta = Resolve_TimeDelta(fTimeDelta);

    __super::Late_Update(fTimeDelta);

    if (m_pTransformCom)
    {
        const auto WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

        for (auto* pCollider : m_KirbyColliders)
        {
            pCollider->Update(WorldMatrix);

#ifdef _DEBUG
            if (pCollider->Is_Enabled())
                m_pGameInstance_Proxy->Add_DebugComponent(pCollider);
#endif
        }

        // BlobShadow 갱신
        _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
        const _float H = 5.f, fSize = 3.5f;
        SHADOW_LIGHT_DESC d{};
        XMStoreFloat4(&d.vEye, vPos + XMVectorSet(0.f, H, 0.f, 0.f));
        XMStoreFloat4(&d.vAt, XMVectorSetW(vPos, 1.f));
        d.fWidth = d.fHeight = fSize;
        d.fNear = 0.1f; d.fFar = H + 3.f;
        m_pGameInstance_Proxy->Update_BlobShadow(d);
    }
}

HRESULT CKirby::Render()
{
    return S_OK;
}

void CKirby::On_Deserialized()
{
    if (m_pMovement)
        m_pMovement->Sync_To_Controller();
}

void CKirby::Set_AbilityPartsActive(COPY_ABILITY_TYPE eAbilityType, _bool bOn, _bool bOnlyWeapon)
{
    CKirby_OnOffPart* pWeapon = Find_WeaponPart(eAbilityType);
    if (pWeapon != nullptr)
        pWeapon->PartOnOff(bOn);

    if (!bOnlyWeapon)
    {
        CKirby_OnOffPart* pHat = Find_HatPart(eAbilityType);
        if (pHat != nullptr)
            pHat->PartOnOff(bOn);
    }
}

void CKirby::Change_HatSocketMatrix(COPY_ABILITY_TYPE eAbilityType, const _float4x4* pBoneMatrix)
{
    CKirby_OnOffPart* pHat = Find_HatPart(eAbilityType);

    if (pHat != nullptr)
        pHat->Set_SocketBoneMatrix(pBoneMatrix);
}

CKirby_OnOffPart* CKirby::Find_OnOffPart(const wchar_t* PartTag)
{
    auto iter = m_PartObjects.find(PartTag);
    if (iter != m_PartObjects.end())
    {
        return dynamic_cast<CKirby_OnOffPart*>(iter->second);
    }

    return nullptr;
}

CKirby_OnOffPart* CKirby::Find_WeaponPart(COPY_ABILITY_TYPE eType)
{
    switch (eType)
    {
    case COPY_ABILITY_TYPE::SWORD:
        return Find_OnOffPart(CKirby_Sword::Kirby_PartTag);
    }

    return nullptr;
}

CKirby_OnOffPart* CKirby::Find_HatPart(COPY_ABILITY_TYPE eType)
{
    switch (eType)
    {
    case COPY_ABILITY_TYPE::SWORD:
        return Find_OnOffPart(CKirby_SwordHat::Kirby_PartTag);
    }

    return nullptr;
}

CKirby_Deform_Model* CKirby::Get_DeformPart_Model(DEFORM_TYPE eDeformType, KIRBY_DEFORM_MODEL_TYPE eDeformModelType)
{
    switch (eDeformType)
    {
        case DEFORM_TYPE::NONE:
            return m_pBody;

        case DEFORM_TYPE::CAR:
            switch (eDeformModelType)
            {
            case KIRBY_DEFORM_MODEL_TYPE::DEMO:
                return Find_DeformModel(CKirby_DeformCar_Demo::Kirby_PartTag);

            case KIRBY_DEFORM_MODEL_TYPE::MAIN:
                return Find_DeformModel(CKirby_DeformCar_Main::Kirby_PartTag);
            }
            break;
    }

    return nullptr;
}

CKirby_Deform_Model* CKirby::Find_DeformModel(const wchar_t* pPartTag)
{
    auto iter = m_PartObjects.find(pPartTag);

    if (iter == m_PartObjects.end())
        return nullptr;

    return dynamic_cast<CKirby_Deform_Model*>(iter->second);
}

void CKirby::Add_MoveDir(const _float3& vWishDir)
{
    XMStoreFloat3(&m_vWishDir,
        XMVectorAdd(XMLoadFloat3(&vWishDir), XMLoadFloat3(&m_vWishDir)));   
}

_bool CKirby::Has_MoveDir()
{
    _vector vWishDir = XMLoadFloat3(&m_vWishDir);

    if (XMVector3Equal(vWishDir, XMVectorZero()))
        return false;

    return true;
}

void CKirby::Excute_Command(CKirby_Command* pCommand)
{
    m_pKirby_StateMachine->Handle_Command(pCommand);
}

void CKirby::Change_State(KIRBY_STATE_TYPE eNewState)
{
    m_pKirby_StateMachine->Change_State(eNewState);
}

CKirby_Ability* CKirby::Get_KirbyAbility()
{
    return m_pKirby_Ability;
}

void CKirby::Set_KirbyAbility(COPY_ABILITY_TYPE eAbilityState)
{
    auto iter = m_Abilities.find(eAbilityState);
    if (iter == m_Abilities.end())
    {
        MSG_BOX("KirbyAbility Bug");
        return;
    }

    m_pKirby_Ability = iter->second;
}

void CKirby::Request_ChangeKirbyAbility(COPY_ABILITY_TYPE eAbilityState)
{
    if(m_pKirby_Ability == nullptr)
        return;

    if (m_pKirby_Ability->Get_AbilityType() == eAbilityState)
        return;

    m_bReqChangeAbility = true;
    
    m_eNextAbilityType = eAbilityState;
}

void CKirby::Apply_ChangeKirbyAbility()
{
    if (m_bReqChangeAbility == false)
        return;

    m_bReqChangeAbility = false;
    Set_KirbyAbility(m_eNextAbilityType);
}

void CKirby::Update_AbilityDumpCool(_float fTimeDelta)
{
    if (m_bDecreaseAbilityDumpCool == true)
    {
        m_fAccAbilityDumpCoolTime -= fTimeDelta;

        m_bDecreaseAbilityDumpCool = false;
    }
    else
    {
        m_fAccAbilityDumpCoolTime += fTimeDelta;
    }

    Helper::FloatClamp(m_fAccAbilityDumpCoolTime, 0.f, m_fMaxAbilityDumpCoolTime);
}

void CKirby::Reset_AbilityDumpCool()
{
    m_fAccAbilityDumpCoolTime = m_fMaxAbilityDumpCoolTime;
}

_bool CKirby::Can_AbilityDump()
{
    if (m_fAccAbilityDumpCoolTime <= 0.f)
        return true;

    return false;
}

HRESULT CKirby::Ready_Components()
{
    // Controller
    _float3 vFootPos;
    XMStoreFloat3(&vFootPos, m_pTransformCom->Get_State(STATE::POSITION));

    m_pController = Add_Component<CController>(TEXT("Com_Controller"),
        CController::Create(m_pDevice, m_pContext));
    if (m_pController == nullptr)
        return E_FAIL;

    CController::CONTROLLER_DESC ctrlDesc{};
    ctrlDesc.vFootPos = vFootPos;
    ctrlDesc.fRadius = s_fCCT_Radius;
    ctrlDesc.fHeight = s_fCCT_Height;
    ctrlDesc.pOwner = this;
    if (FAILED(m_pController->Initialize(&ctrlDesc)))
        return E_FAIL;


    // Movement
    m_pMovement = Add_Component<CMovement_Child>(TEXT("Com_Movement"), CMovement_Child::Create(m_pDevice, m_pContext));
    if (m_pMovement == nullptr) return E_FAIL;

    m_pMovement->Set_Refs(m_pTransformCom, m_pController->Get_Raw());

    // Cllider
    m_KirbyColliders.resize(COLLIDER_END);

    // Collider HurtBox
    CCollider::COLLIDER_DESC ColliderDesc{};
    ColliderDesc.pOwner = this;
    ColliderDesc.vCenter = _float3(vFootPos.x, vFootPos.y + (s_fCCT_Radius + 0.1f), vFootPos.z);
    ColliderDesc.fRadius = s_fCCT_Radius + 0.1f;

    m_KirbyColliders[KIRBY_COLLIDER::HURT_BOX] = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
        TEXT("HurtBox_Com"), &ColliderDesc);
    if (m_KirbyColliders[KIRBY_COLLIDER::HURT_BOX] == nullptr)
        return E_FAIL;

    m_pGameInstance_Proxy->Register_Collider(m_KirbyColliders[KIRBY_COLLIDER::HURT_BOX], ETOUI(COLLISION_LAYER::PLAYER_HURT));


    // Collider Inhale
    CCollider::COLLIDER_DESC InhaleDesc{};
    InhaleDesc.pOwner = this;
    InhaleDesc.vCenter = _float3(0.f, s_fInhaleUp, 0.f);
    InhaleDesc.fRadius = s_fInhaleRadius;
    InhaleDesc.fHeight = s_fInhaleLength;
    InhaleDesc.vRadians = _float3(XMConvertToRadians(90.f), 0.f, 0.f);

    m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX] = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        TEXT("InhaleBox_Com"), &InhaleDesc);
    if (m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX] == nullptr)
        return E_FAIL;

    m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX]->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX], ETOUI(COLLISION_LAYER::PLAYER_INHALE));

    // Wall Breaker Collider
    CCollider::COLLIDER_DESC WallBreakerDesc{};
    WallBreakerDesc.pOwner = this;
    WallBreakerDesc.vCenter = _float3(0.f, 1.5f, 1.3f);
    WallBreakerDesc.fRadius = 2.f;

    m_KirbyColliders[KIRBY_COLLIDER::CAR_BOOST_COLLIDER] = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
        TEXT("WallBreakerCollider_Com"), &WallBreakerDesc);
    if (m_KirbyColliders[KIRBY_COLLIDER::CAR_BOOST_COLLIDER] == nullptr)
        return E_FAIL;

    m_KirbyColliders[KIRBY_COLLIDER::CAR_BOOST_COLLIDER]->Set_Enabled(false);
     m_pGameInstance_Proxy->Register_Collider(m_KirbyColliders[KIRBY_COLLIDER::CAR_BOOST_COLLIDER], ETOUI(COLLISION_LAYER::CAR_BOOST));

    //임시
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::MONSTER_PROJECTILE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_HIT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_PROJECTILE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_D_RANGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_TRIGGER));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_LADDER));

    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HIT),        ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE), ETOUI(COLLISION_LAYER::MONSTER_HURT));

    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE),     ETOUI(COLLISION_LAYER::ENV_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HIT),        ETOUI(COLLISION_LAYER::ENV_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE), ETOUI(COLLISION_LAYER::ENV_HURT));

    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::CAR_BOOST), ETOUI(COLLISION_LAYER::ENV_TRIGGER));

    return S_OK;
}

void CKirby::SetUp_Collider_Callback()
{
    if (m_KirbyColliders[HURT_BOX])
    {
        m_KirbyColliders[HURT_BOX]->Set_OnEnter(
            [this](CCollider* pOther)
            {
                const _uint iGroup = pOther->Get_RegisteredGroup();

                if (iGroup == ETOUI(COLLISION_LAYER::MONSTER_HURT))
                {
                    CMonster* pMon = dynamic_cast<CMonster*>(pOther->Get_Owner());
                    if (pMon && !pMon->Is_Touch_Harmful())
                        return;

                    _vector vAtkPos = pOther->Get_Owner()->Get_Transform()->Get_State(STATE::POSITION);
                    ATTACK_INFO atk{};
                    atk.fDamage = 1.f;
                    atk.fKnockback = 2.f;                     
                    XMStoreFloat3(&atk.vAttackerPos, vAtkPos);
                    atk.pAttacker = pOther->Get_Owner();
                    Damaged(atk);
#ifdef _DEBUG
                    char szBuf[128];
                    sprintf_s(szBuf, "[Kirby] Hurt! HP %.0f/%.0f\n", m_fCurHP, m_fMaxHP);
                    OutputDebugStringA(szBuf);
#endif
                }

                else if (iGroup == ETOUI(COLLISION_LAYER::ENV_LADDER))
                {
                    CLevelDesign_Ladder* pLadder = dynamic_cast<CLevelDesign_Ladder*>(pOther->Get_Owner());
                    if (pLadder == nullptr)
                        return;

                    Set_Ladder(pLadder);
                }
            }
        );
    }

    m_KirbyColliders[HURT_BOX]->Set_OnExit(
        [this](CCollider* pOther)
        {
            const _uint iGroup = pOther->Get_RegisteredGroup();

            if (iGroup == ETOUI(COLLISION_LAYER::ENV_LADDER))
            {
                Clear_Ladder();
                return;
            }
        });
}

HRESULT CKirby::Ready_PartObjects()
{
    // Body
    CKirby_Body::KIRBY_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    BodyDesc.pHitFlash = Get_HitFlashPtr();       
    BodyDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_Body::PROTOTYPE_TAG, CKirby_Body::Kirby_PartTag, &BodyDesc)))
        return E_FAIL;

    m_pBody = dynamic_cast<CKirby_Body*>(m_PartObjects[CKirby_Body::Kirby_PartTag]);

    // DeformCar_Demo
    CKirby_DeformCar_Demo::KIRBY_DEFORMCAR_DEMO_DESC DeformCar_Demo_Desc{};
    DeformCar_Demo_Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    DeformCar_Demo_Desc.pHitFlash = Get_HitFlashPtr();
    DeformCar_Demo_Desc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_DeformCar_Demo::PROTOTYPE_TAG, CKirby_DeformCar_Demo::Kirby_PartTag, &DeformCar_Demo_Desc)))
        return E_FAIL;

    // DeformCar_Main
    CKirby_DeformCar_Main::KIRBY_DEFORMCAR_MAIN_DESC DeformCar_Main_Desc{};
    DeformCar_Main_Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    DeformCar_Main_Desc.pHitFlash = Get_HitFlashPtr();
    DeformCar_Main_Desc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_DeformCar_Main::PROTOTYPE_TAG, CKirby_DeformCar_Main::Kirby_PartTag, &DeformCar_Main_Desc)))
        return E_FAIL;


    // Sword
    CKirby_Sword::KIRBY_SWORD_DESC SwordDesc{};
    SwordDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    SwordDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");
    SwordDesc.pHitFlash = Get_HitFlashPtr();
    SwordDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_Sword::PROTOTYPE_TAG, CKirby_Sword::Kirby_PartTag, &SwordDesc)))
        return E_FAIL;

    // SwordHat
    CKirby_SwordHat::KIRBY_SWORDHAT_DESC SwordHatDesc{};
    SwordHatDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    SwordHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    SwordHatDesc.pHitFlash = Get_HitFlashPtr();
    SwordHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_SwordHat::PROTOTYPE_TAG, CKirby_SwordHat::Kirby_PartTag, &SwordHatDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby::Ready_System()
{
    m_pKirby_StateMachine = CKirby_StateMachine::Create(this);
    if (m_pKirby_StateMachine == nullptr)
        return E_FAIL;

    m_pKirby_Controller = CKirby_Controller::Create(this);
    if (m_pKirby_Controller == nullptr)
        return E_FAIL;

    m_pKirby_InputManager =  CKirby_InputManager::Create(this, m_pKirby_Controller);
    if (m_pKirby_InputManager == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby::Ready_Ability()
{
    auto Register_Ability = [this](COPY_ABILITY_TYPE eType, CKirby_Ability* pNewAbility) -> HRESULT
        {
            if (pNewAbility == nullptr)
                return E_FAIL;

            m_Abilities[eType] = pNewAbility;

            return S_OK;
        };

    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::NORMAL, CKirby_Ability_Normal::Create()))) return E_FAIL;
    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::SWORD, CKirby_Ability_Sword::Create())))   return E_FAIL;

    auto iter = m_Abilities.find(COPY_ABILITY_TYPE::NORMAL);
    if (iter == m_Abilities.end())
        return E_FAIL;

    m_pKirby_Ability = iter->second;

    return S_OK;
}

HRESULT CKirby::Ready_Deform()
{
    auto Register_Deform = [this](DEFORM_TYPE eType, CKirby_Deform* pNewDeform) -> HRESULT
        {
            if (pNewDeform == nullptr)
                return E_FAIL;

            m_Deformations[eType] = pNewDeform;

            return S_OK;
        };

    if (FAILED(Register_Deform(DEFORM_TYPE::CAR, CKirby_Deform_Car::Create()))) return E_FAIL;

    return S_OK;
}

HRESULT CKirby::Bind_ShaderResources()
{
    return S_OK;
}

HRESULT CKirby::Ready_Events()
{
    Subscribe_Event(EVT_QUERY_PLAYER,
        [this](void* pData)
        {
            static_cast<PLAYER_QUERY*>(pData)->pPlayer = this;
        }
    );

    Subscribe_Event(EventTag::Cutscene_GrabKirby,
        [this](void* pData)
        {
            CUTSCENE_GRAB_DESC* pDesc = static_cast<CUTSCENE_GRAB_DESC*>(pData);
            Set_CutsceneGrabTarget(pDesc);
            m_pKirby_StateMachine->Request_GrabState_StateMachine(pDesc->eType);
        });

    Subscribe_Event(EventTag::Cutscene_GorillaHandoff,
        [this](void*)
        {
            Clear_CutsceneGrabTarget();
            m_pKirby_StateMachine->Request_ReleaseGrabState_StateMachine();
        });

    Subscribe_Event(EventTag::Cutscene_ReleaseKirby,
        [this](void*)
        {
            Clear_CutsceneGrabTarget();
            m_pKirby_StateMachine->Request_ReleaseGrabState_StateMachine();
        });

    return S_OK;
}

HRESULT CKirby::Ready_AnimEvents()
{
    if(FAILED(m_pBody->Ready_AnimEvents(this)))
        return E_FAIL;

    if (FAILED(Get_DeformPart_Model(DEFORM_TYPE::CAR, KIRBY_DEFORM_MODEL_TYPE::DEMO)->Ready_AnimEvents(this)))
        return E_FAIL;

    if (FAILED(Get_DeformPart_Model(DEFORM_TYPE::CAR, KIRBY_DEFORM_MODEL_TYPE::MAIN)->Ready_AnimEvents(this)))
        return E_FAIL;

    return S_OK;
}

_bool CKirby::Block_Hit(const ATTACK_INFO& tInfo) 
{ 
    return m_fInvincibleTime > 0.f; 
}

void  CKirby::On_Damaged(const ATTACK_INFO& tInfo)
{
    m_pKirby_StateMachine->On_Damaged_KirbyStateMachine(tInfo);
}

void CKirby::Update_Timer(_float fTimeDelta)
{
    if (m_fInvincibleTime > 0.f)
        m_fInvincibleTime -= fTimeDelta;
}

void CKirby::Set_CutsceneGrabTarget(CUTSCENE_GRAB_DESC* pGrabDesc)
{
    m_pGrabBone = pGrabDesc->pBoneMatrix;
    m_pGrabOwnerWorld = pGrabDesc->pSourceWorld;
}

void CKirby::Clear_CutsceneGrabTarget()
{
    m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 0.f);

    m_pGrabBone = nullptr;
    m_pGrabOwnerWorld = nullptr;
}

_float CKirby::Resolve_TimeDelta(_float fTimeDelta)
{
    KIRBY_STATE_TYPE eType = m_pKirby_StateMachine->Get_StateType();
    if (eType == KIRBY_STATE_TYPE::GET_ABILITY)
        return m_pGameInstance_Proxy->Get_RawTimeDelta(L"Timer_60");
    else
        return fTimeDelta;
}

void CKirby::Change_KirbyDeform(DEFORM_TYPE eDeformType)
{
    if (eDeformType == DEFORM_TYPE::NONE)
    {
        m_pKirby_Deform = nullptr;
        return;
    }

    auto iter = m_Deformations.find(eDeformType);
    if (iter == m_Deformations.end())
    {
        MSG_BOX("KirbyDeform Bug");
        return;
    }

    m_pKirby_Deform = iter->second;
    m_pKirby_Deform->Enter_Deform(this);
}

void CKirby::Reset_KirbyDeform()
{
    if(m_pKirby_Deform == nullptr)
    {
        MSG_BOX("m_pKirby_Deform is nullptr");
        return;
    }

    m_pKirby_Deform->Exit_Deform(this);
    m_pKirby_Deform = nullptr;
}

CCollider* CKirby::Get_Collider(KIRBY_COLLIDER eKirbyCollider)
{
    return m_KirbyColliders[eKirbyCollider];
}

void CKirby::Update_CutsceneGrabTransform()
{
    if (m_pGrabBone == nullptr || m_pGrabOwnerWorld == nullptr)
        return;

    _matrix matGrabTargetWorld = XMMatrixRotationY(XMConvertToRadians(-180.f)) * XMLoadFloat4x4(m_pGrabBone) * XMLoadFloat4x4(m_pGrabOwnerWorld);
    Get_Transform()->Set_WorldMatrix(matGrabTargetWorld);

    m_pMovement->Sync_To_Controller();
}

void CKirby::Damaged(const ATTACK_INFO& tInfo)
{
    if (!Is_Active())
        return;

    if (Block_Hit(tInfo))
        return;

    On_Damaged(tInfo);

    if (m_fCurHP <= 0.f)
    {
        m_fCurHP = 0.f;
        On_Death(tInfo);
    }
}

void CKirby::Add_HP(_float fHP)
{
    m_fCurHP += fHP;

    Helper::FloatClamp(m_fCurHP, 0.f, m_fMaxHP);

    KIRBY_HP_UPDATED eDesc{};
    eDesc.fCurrHp = m_fCurHP;
    eDesc.fMaxHP = m_fMaxHP;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_HP_Updated, &eDesc);
}

CKirby* CKirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby* pInstance = new CKirby(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby::Clone(void* pArg)
{
    CKirby* pInstance = new CKirby(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CKirby");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby::Free()
{
    m_pKirby_Ability = nullptr;
    for (auto pair : m_Abilities)
        Safe_Release(pair.second);
    m_Abilities.clear();

    m_pKirby_Deform = nullptr;
    for (auto pair : m_Deformations)
        Safe_Release(pair.second);
    m_Deformations.clear();

    m_KirbyColliders.clear();

    Safe_Release(m_pKirby_InputManager);
    Safe_Release(m_pKirby_Controller);
    Safe_Release(m_pKirby_StateMachine);

    __super::Free();
}