#include "Kirby.h"

#include "GameInstance.h"

#include "PartObject.h"
#include "Monster.h"
#include "Controller.h"

#include "GameContent_const.h"
#include "Effect_Loader.h"

#include "Movement_Child.h"

// Parts
#include "Kirby_Body.h"
#include "Kirby_BombHat.h"
#include "Kirby_CrashHat.h"
#include "Kirby_IceHat.h"
#include "Kirby_SleepHat.h"
#include "Kirby_Sword.h"
#include "Kirby_SwordHat.h"
#include "Kirby_MetaSword.h"
#include "Kirby_MetaHat.h"
#include "Kirby_ToyHammer.h"
#include "Kirby_ToyHat.h"

#include "Kirby_DeformCar_Demo.h"
#include "Kirby_DeformCar_Main.h"
#include "Kirby_DeformCylinder_Demo.h"
#include "Kirby_DeformCylinder_Main.h"
#include "Kirby_DeformRollerCoaster_Main.h"

#include "Kirby_InputManager.h"
#include "Kirby_Controller.h"
#include "Kirby_StateMachine.h"

// Ability
#include "Kirby_State.h"
#include "Kirby_Ability_Normal.h"
#include "Kirby_Ability_Sword.h"
#include "Kirby_Ability_MetaKnightSword.h"
#include "Kirby_Ability_Bomb.h"
#include "Kirby_Ability_Crash.h"
#include "Kirby_Ability_Ice.h"
#include "Kirby_Ability_Sleep.h"
#include "Kirby_Ability_ToyHammer.h"

// Deform
#include "Kirby_Deform_Car.h"
#include "Kirby_Deform_Cylinder.h"
#include "Kirby_Deform_RollerCoaster.h"

// Ladder
#include "LevelDesign_Ladder.h"

#include "EssenceBubble.h"
#include "LD_DeformObject.h"

// 임시
#include "DropStar_Manager.h"

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
    XMStoreFloat4x4(&m_RenderWorldMatrix, XMMatrixIdentity());

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

    if (FAILED(SetUp_Collider_Callback()))
        return E_FAIL;

    // Part 생성된 후
    if (FAILED(Ready_AnimEvents()))
        return E_FAIL;  
  
    m_fInvincibleDuration = 2.f;

    return S_OK;
}

void CKirby::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    if (m_pKirby_StateMachine->Ignore_TimeScale_StateMachine())
        fTimeDelta = m_pGameInstance_Proxy->Get_RawTimeDelta(L"Timer_60");

    __super::Priority_Update(fTimeDelta);
}

void CKirby::Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    if (m_pKirby_StateMachine->Ignore_TimeScale_StateMachine())
        fTimeDelta = m_pGameInstance_Proxy->Get_RawTimeDelta(L"Timer_60");

    XMStoreFloat3(&m_vWishDir, XMVectorZero());

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

    Update_RenderWorldMatrix(fTimeDelta);

    __super::Update(fTimeDelta);

    Update_InvincibilityHitFlash();

#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_TAB))
    {
        wchar_t szBuf[128] = {};
        swprintf_s(szBuf, L"[Kirby] LevelIndex=%u, Layer=%s, Active=%d\n",
            Get_LevelIndex(), m_strLayerTag.c_str(), Is_Active() ? 1 : 0);
        OutputDebugStringW(szBuf);
    }
#endif
}

void CKirby::Late_Update(_float fTimeDelta)
{
    if (!m_bActive)
        return;

    if (m_pKirby_StateMachine->Ignore_TimeScale_StateMachine())
        fTimeDelta = m_pGameInstance_Proxy->Get_RawTimeDelta(L"Timer_60");

    __super::Late_Update(fTimeDelta);

    const _matrix WorldMatrix = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

    for (auto* pCollider : m_KirbyColliders)
    {
        pCollider->Update(WorldMatrix);

#ifdef _DEBUG
        if (pCollider->Is_Enabled())
            m_pGameInstance_Proxy->Add_DebugComponent(pCollider);
#endif
    }

    //Update_BlobShadow();
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

_bool CKirby::Dispatch_BodyAnimEventToAbility(const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
{
    return m_pKirby_Ability->Handle_BodyAnimEvent(this, e, ePhase);
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
        case COPY_ABILITY_TYPE::METAKNIGHT_SWORD:
            return Find_OnOffPart(CKirby_MetaSword::Kirby_PartTag);
        case COPY_ABILITY_TYPE::TOY_HAMMER:
            return Find_OnOffPart(CKirby_ToyHammer::Kirby_PartTag);
    }

    return nullptr;
}

CKirby_OnOffPart* CKirby::Find_HatPart(COPY_ABILITY_TYPE eType)
{
    switch (eType)
    {
        case COPY_ABILITY_TYPE::BOMB:
            return Find_OnOffPart(CKirby_BombHat::Kirby_PartTag);
        case COPY_ABILITY_TYPE::CRASH:
            return Find_OnOffPart(CKirby_CrashHat::Kirby_PartTag);
        case COPY_ABILITY_TYPE::ICE:
            return Find_OnOffPart(CKirby_IceHat::Kirby_PartTag);
        case COPY_ABILITY_TYPE::SLEEP:
            return Find_OnOffPart(CKirby_SleepHat::Kirby_PartTag);
        case COPY_ABILITY_TYPE::SWORD:
            return Find_OnOffPart(CKirby_SwordHat::Kirby_PartTag);
        case COPY_ABILITY_TYPE::METAKNIGHT_SWORD:
            return Find_OnOffPart(CKirby_MetaHat::Kirby_PartTag);
        case COPY_ABILITY_TYPE::TOY_HAMMER:
            return Find_OnOffPart(CKirby_ToyHat::Kirby_PartTag);
    }

    return nullptr;
}

void CKirby::Set_OnOffPartMode(KIRBY_PART_MODE ePartMode)
{
    COPY_ABILITY_TYPE eAbilityType = m_pKirby_Ability->Get_AbilityType();

    CKirby_OnOffPart* pWeapon = Find_WeaponPart(eAbilityType);
    if (pWeapon == nullptr)
        return;

    CKirby_OnOffPart* pHat = Find_HatPart(eAbilityType);
    if (pHat == nullptr)
        return;

    pWeapon->Set_PartMode(this, ePartMode);
    pHat->Set_PartMode(this, ePartMode);
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

        case DEFORM_TYPE::CYLINDER:
            switch (eDeformModelType)
            {
            case KIRBY_DEFORM_MODEL_TYPE::DEMO:
                return Find_DeformModel(CKirby_DeformCylinder_Demo::Kirby_PartTag);

            case KIRBY_DEFORM_MODEL_TYPE::MAIN:
                return Find_DeformModel(CKirby_DeformCylinder_Main::Kirby_PartTag);
            }
            break;

        case DEFORM_TYPE::COASTER:
            if (eDeformModelType == KIRBY_DEFORM_MODEL_TYPE::MAIN)
                return Find_DeformModel(CKirby_DeformRollerCoaster_Main::Kirby_PartTag);
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

void CKirby::Set_CCTSize(_float fCCTRadius, _float fCCTHeight)
{
    m_pController->Set_CapsuleSize(fCCTRadius, fCCTHeight);
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

void CKirby::Change_State(KIRBY_STATE_TYPE eNewState, _int iFlag)
{
    m_pKirby_StateMachine->Change_State(eNewState, iFlag);
}

CKirby_AttackMode* CKirby::Get_ActiveAttackMode()
{
    if (m_pKirby_Deform != nullptr)
        return m_pKirby_Deform;
    else
        return m_pKirby_Ability;
}

CKirby_Deform_Model* CKirby::Get_CurrentDeformModel()
{
    if (Has_Deform())
        return Get_DeformPart_Model(m_pKirby_Deform->Get_DeformType());
    else
        return m_pBody;
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

void CKirby::Update_DumpCool(_float fTimeDelta)
{
    if (m_bCurDecreaseDumpCool != m_bPreDecreaseDumpCool)
    {
        if (m_bCurDecreaseDumpCool)
        {
            ABILITY_DISCARD_BIND_DESC tDesc{};
            tDesc.pCoolTime = &m_fAccDumpCoolTime;
            tDesc.fMaxCoolTime = m_fMaxDumpCoolTime;

            if(Has_Deform())
                tDesc.bIsAbility = false;

            m_pGameInstance_Proxy->Publish(EventTag::AbilityDiscardUI_Bind, &tDesc);
        }

        m_bPreDecreaseDumpCool = m_bCurDecreaseDumpCool;
    }

    if (m_bCurDecreaseDumpCool == true)
    {
        m_fAccDumpCoolTime -= fTimeDelta;
        m_bCurDecreaseDumpCool = false;
    }
    else
    {
        m_fAccDumpCoolTime += fTimeDelta;
    }

    Helper::FloatClamp(m_fAccDumpCoolTime, 0.f, m_fMaxDumpCoolTime);
}

void CKirby::Reset_DumpCool()
{
    m_fAccDumpCoolTime = m_fMaxDumpCoolTime;
}

_bool CKirby::Can_Dump()
{
    if (m_fAccDumpCoolTime <= 0.f)
        return true;

    return false;
}

void CKirby::Add_Coin(_int iCoin)
{
    if (m_iCoin == 0 && iCoin < 0)
        return;

    if(iCoin != 0)
    {
        // 이벤트
        m_iCoin += iCoin;
    }
}

HRESULT CKirby::Ready_Components()
{
    // Controller
    m_pController = Add_Component<CController>(TEXT("Com_Controller"), CController::Create(m_pDevice, m_pContext));
    if (m_pController == nullptr)
        return E_FAIL;

    CController::CONTROLLER_DESC ctrlDesc{};
    ctrlDesc.vFootPos = { 0.f ,0.f, 0.f };
    ctrlDesc.fRadius = s_fCCT_Radius;
    ctrlDesc.fHeight = s_fCCT_Height;
    ctrlDesc.pOwner = this;
    if (FAILED(m_pController->Initialize(&ctrlDesc)))
        return E_FAIL;

    // Movement
    m_pMovement = Add_Component<CMovement_Child>(TEXT("Com_Movement"), CMovement_Child::Create(m_pDevice, m_pContext));
    if (m_pMovement == nullptr)
        return E_FAIL;

    m_pMovement->Set_Refs(m_pTransformCom, m_pController->Get_Raw());

    // Collider
    m_KirbyColliders.resize(COLLIDER_END);

    // Collider HurtBox
    CCollider::COLLIDER_DESC tColliderDesc{};
    tColliderDesc.pOwner = this;
    tColliderDesc.vCenter = _float3(0.f, 0.f, 0.f); // 발 위치임
    tColliderDesc.fRadius = s_fCCT_Radius + s_fHurtBoxRadiusPadding;
    tColliderDesc.fHeight = s_fCCT_Height;

    m_KirbyColliders[KIRBY_COLLIDER::HURT_BOX] = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        TEXT("HurtBox_Com"), &tColliderDesc);
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


    // Collider Slide
    CCollider::COLLIDER_DESC SlideDesc{};
    SlideDesc.pOwner = this;
    SlideDesc.vCenter = _float3(0.f, 0.f, 0.8f);
    SlideDesc.fRadius = 1.f;

    m_KirbyColliders[KIRBY_COLLIDER::SLIDE_COLLIDER] = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
        TEXT("SlideCollider_Com"), &SlideDesc);
    if (m_KirbyColliders[KIRBY_COLLIDER::SLIDE_COLLIDER] == nullptr)
        return E_FAIL;

    m_KirbyColliders[KIRBY_COLLIDER::SLIDE_COLLIDER]->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_KirbyColliders[KIRBY_COLLIDER::SLIDE_COLLIDER], ETOUI(COLLISION_LAYER::PLAYER_HIT));


    // Collider Breakerable HitBox
    CCollider::COLLIDER_DESC BreakerableHitDesc{};
    BreakerableHitDesc.pOwner = this;
    BreakerableHitDesc.vSize = _float3(1.f, 1.f, 1.f);
    BreakerableHitDesc.fRadius = 1.f;
    BreakerableHitDesc.fHeight = 1.f;

    m_KirbyColliders[KIRBY_COLLIDER::BREAKERABLE_HITBOX] = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        TEXT("BreakerableHitBox_Com"), &BreakerableHitDesc);
    if (m_KirbyColliders[KIRBY_COLLIDER::BREAKERABLE_HITBOX] == nullptr)
        return E_FAIL;

    m_KirbyColliders[KIRBY_COLLIDER::BREAKERABLE_HITBOX]->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_KirbyColliders[KIRBY_COLLIDER::BREAKERABLE_HITBOX], ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE));


    //임시
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::MONSTER_PROJECTILE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::ENV_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::ENV_FOLIAGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::DROPPED_BUBBLE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::DROP_STAR));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::LD_ITEM));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::ENV_INTERACT_KICKPROP));

    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_HIT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_PROJECTILE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_D_RANGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_TRIGGER));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_LADDER));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ESSENCE_BUBBLE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::DROPPED_BUBBLE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_FOLIAGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::DEFORM_OBJECT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::LD_ITEM));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::DEFORM_RELEASE_AREA));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::EXCALIBUR));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_INTERACT_KICKPROP));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_PROJECTILE));

    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HIT), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HIT), ETOUI(COLLISION_LAYER::ENV_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HIT), ETOUI(COLLISION_LAYER::ENV_FOLIAGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HIT), ETOUI(COLLISION_LAYER::ENV_INTERACT_KICKPROP));


    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE), ETOUI(COLLISION_LAYER::ENV_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE), ETOUI(COLLISION_LAYER::ENV_FOLIAGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE), ETOUI(COLLISION_LAYER::ENV_INTERACT_KICKPROP));

    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BOMB), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BOMB), ETOUI(COLLISION_LAYER::ENV_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BOMB), ETOUI(COLLISION_LAYER::ENV_FOLIAGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BOMB), ETOUI(COLLISION_LAYER::PLAYER_BOMB));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BOMB), ETOUI(COLLISION_LAYER::ENV_INTERACT_KICKPROP));

    // Kirby_DeformCar_Main
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE), ETOUI(COLLISION_LAYER::ENV_TRIGGER));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE), ETOUI(COLLISION_LAYER::ENV_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE), ETOUI(COLLISION_LAYER::ENV_FOLIAGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_BREAKERABLE), ETOUI(COLLISION_LAYER::ENV_INTERACT_KICKPROP));

    // 호준 추가
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::ENV_PROJECTILE), ETOUI(COLLISION_LAYER::ENV_TRIGGER));

    return S_OK;
}

HRESULT CKirby::Ready_PartObjects()
{
    // Body
    CKirby_Body::KIRBY_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = &m_RenderWorldMatrix;

    BodyDesc.pHitFlashIntensity = Get_HitFlashPtr();       
    BodyDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_Body::PROTOTYPE_TAG, CKirby_Body::Kirby_PartTag, &BodyDesc)))
        return E_FAIL;

    m_pBody = dynamic_cast<CKirby_Body*>(m_PartObjects[CKirby_Body::Kirby_PartTag]);

    // DeformCar_Demo
    CKirby_DeformCar_Demo::KIRBY_DEFORMCAR_DEMO_DESC DeformCar_Demo_Desc{};
    DeformCar_Demo_Desc.pParentMatrix = &m_RenderWorldMatrix;
    DeformCar_Demo_Desc.pHitFlashIntensity = Get_HitFlashPtr();
    DeformCar_Demo_Desc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_DeformCar_Demo::PROTOTYPE_TAG, CKirby_DeformCar_Demo::Kirby_PartTag, &DeformCar_Demo_Desc)))
        return E_FAIL;

    // DeformCar_Main
    CKirby_DeformCar_Main::KIRBY_DEFORMCAR_MAIN_DESC DeformCar_Main_Desc{};
    DeformCar_Main_Desc.pParentMatrix = &m_RenderWorldMatrix;
    DeformCar_Main_Desc.pHitFlashIntensity = Get_HitFlashPtr();
    DeformCar_Main_Desc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_DeformCar_Main::PROTOTYPE_TAG, CKirby_DeformCar_Main::Kirby_PartTag, &DeformCar_Main_Desc)))
        return E_FAIL;

    // DeformCylinder_Demo
    CKirby_DeformCylinder_Demo::KIRBY_DEFORMCYLINDER_DEMO_DESC DeformCylinder_Demo_Desc{};
    DeformCylinder_Demo_Desc.pParentMatrix = &m_RenderWorldMatrix;
    DeformCylinder_Demo_Desc.pHitFlashIntensity = Get_HitFlashPtr();
    DeformCylinder_Demo_Desc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_DeformCylinder_Demo::PROTOTYPE_TAG,
        CKirby_DeformCylinder_Demo::Kirby_PartTag, &DeformCylinder_Demo_Desc)))
        return E_FAIL;

    // DeformCylinder_Main
    CKirby_DeformCylinder_Main::KIRBY_DEFORMCYLINDER_MAIN_DESC DeformCylinder_Main_Desc{};
    DeformCylinder_Main_Desc.pParentMatrix = &m_RenderWorldMatrix;
    DeformCylinder_Main_Desc.pHitFlashIntensity = Get_HitFlashPtr();
    DeformCylinder_Main_Desc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_DeformCylinder_Main::PROTOTYPE_TAG,
        CKirby_DeformCylinder_Main::Kirby_PartTag, &DeformCylinder_Main_Desc)))
        return E_FAIL;

    // DeformRollerCoaster_Main
    CKirby_DeformRollerCoaster_Main::KIRBY_DEFORMROLLERCOASTER_MAIN_DESC DeformRollerCoaster_Main_Desc{};
    DeformRollerCoaster_Main_Desc.pParentMatrix = &m_RenderWorldMatrix;
    DeformRollerCoaster_Main_Desc.pHitFlashIntensity = Get_HitFlashPtr();
    DeformRollerCoaster_Main_Desc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_DeformRollerCoaster_Main::PROTOTYPE_TAG,
        CKirby_DeformRollerCoaster_Main::Kirby_PartTag, &DeformRollerCoaster_Main_Desc)))
        return E_FAIL;

    // Sword
    CKirby_Sword::KIRBY_SWORD_DESC SwordDesc{};
    SwordDesc.pParentMatrix = &m_RenderWorldMatrix;
    SwordDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");
    SwordDesc.pHitFlashIntensity = Get_HitFlashPtr();
    SwordDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_Sword::PROTOTYPE_TAG, CKirby_Sword::Kirby_PartTag, &SwordDesc)))
        return E_FAIL;

    // SwordHat
    CKirby_SwordHat::KIRBY_SWORDHAT_DESC SwordHatDesc{};
    SwordHatDesc.pParentMatrix = &m_RenderWorldMatrix;
    SwordHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    SwordHatDesc.pHitFlashIntensity = Get_HitFlashPtr();
    SwordHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_SwordHat::PROTOTYPE_TAG, CKirby_SwordHat::Kirby_PartTag, &SwordHatDesc)))
        return E_FAIL;

    // MetaSword
    CKirby_MetaSword::KIRBY_METASWORD_DESC MetaSwordDesc{};
    MetaSwordDesc.pParentMatrix = &m_RenderWorldMatrix;
    MetaSwordDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");
    MetaSwordDesc.pHitFlashIntensity = Get_HitFlashPtr();
    MetaSwordDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_MetaSword::PROTOTYPE_TAG, CKirby_MetaSword::Kirby_PartTag, &MetaSwordDesc)))
        return E_FAIL;

    // MetaHat
    CKirby_MetaHat::KIRBY_METAHAT_DESC MetaHatDesc{};
    MetaHatDesc.pParentMatrix = &m_RenderWorldMatrix;
    MetaHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    MetaHatDesc.pHitFlashIntensity = Get_HitFlashPtr();
    MetaHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_MetaHat::PROTOTYPE_TAG, CKirby_MetaHat::Kirby_PartTag, &MetaHatDesc)))
        return E_FAIL;

    // ToyHammer
    CKirby_ToyHammer::KIRBY_TOYHAMMER_DESC ToyHammerDesc{};
    ToyHammerDesc.pParentMatrix = &m_RenderWorldMatrix;
    ToyHammerDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");
    ToyHammerDesc.pHitFlashIntensity = Get_HitFlashPtr();
    ToyHammerDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_ToyHammer::PROTOTYPE_TAG, CKirby_ToyHammer::Kirby_PartTag, &ToyHammerDesc)))
        return E_FAIL;

    // ToyHat
    CKirby_ToyHat::KIRBY_TOYHAT_DESC ToyHatDesc{};
    ToyHatDesc.pParentMatrix = &m_RenderWorldMatrix;
    ToyHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    ToyHatDesc.pHitFlashIntensity = Get_HitFlashPtr();
    ToyHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_ToyHat::PROTOTYPE_TAG, CKirby_ToyHat::Kirby_PartTag, &ToyHatDesc)))
        return E_FAIL;

    // BombHat
    CKirby_BombHat::KIRBY_BOMB_HAT_DESC BombHatDesc{};
    BombHatDesc.pParentMatrix = &m_RenderWorldMatrix;
    BombHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    BombHatDesc.pHitFlashIntensity = Get_HitFlashPtr();
    BombHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_BombHat::PROTOTYPE_TAG, CKirby_BombHat::Kirby_PartTag, &BombHatDesc)))
        return E_FAIL;

    // CrashHat
    CKirby_CrashHat::KIRBY_CRASH_HAT_DESC CrashHatDesc{};
    CrashHatDesc.pParentMatrix = &m_RenderWorldMatrix;
    CrashHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    CrashHatDesc.pHitFlashIntensity = Get_HitFlashPtr();
    CrashHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_CrashHat::PROTOTYPE_TAG, CKirby_CrashHat::Kirby_PartTag, &CrashHatDesc)))
        return E_FAIL;

    // IceHat
    CKirby_IceHat::KIRBY_ICE_HAT_DESC IceHatDesc{};
    IceHatDesc.pParentMatrix = &m_RenderWorldMatrix;
    IceHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    IceHatDesc.pHitFlashIntensity = Get_HitFlashPtr();
    IceHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_IceHat::PROTOTYPE_TAG, CKirby_IceHat::Kirby_PartTag, &IceHatDesc)))
        return E_FAIL;

    // SleepHat
    CKirby_SleepHat::KIRBY_SLEEP_HAT_DESC SleepHatDesc{};
    SleepHatDesc.pParentMatrix = &m_RenderWorldMatrix;
    SleepHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");
    SleepHatDesc.pHitFlashIntensity = Get_HitFlashPtr();
    SleepHatDesc.pHitFlashColor = Get_HitFlashColorPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_SleepHat::PROTOTYPE_TAG, CKirby_SleepHat::Kirby_PartTag, &SleepHatDesc)))
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
    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::METAKNIGHT_SWORD, CKirby_Ability_MetaKnightSword::Create()))) return E_FAIL;
    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::BOMB, CKirby_Ability_Bomb::Create())))     return E_FAIL;
    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::CRASH, CKirby_Ability_Crash::Create())))   return E_FAIL;
    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::ICE, CKirby_Ability_Ice::Create())))       return E_FAIL;
    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::SLEEP, CKirby_Ability_Sleep::Create())))   return E_FAIL;
    if (FAILED(Register_Ability(COPY_ABILITY_TYPE::TOY_HAMMER, CKirby_Ability_ToyHammer::Create()))) return E_FAIL;

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

    if (FAILED(Register_Deform(DEFORM_TYPE::CAR, CKirby_Deform_Car::Create())))                         return E_FAIL;
    if (FAILED(Register_Deform(DEFORM_TYPE::CYLINDER, CKirby_Deform_Cylinder::Create())))               return E_FAIL;
    if (FAILED(Register_Deform(DEFORM_TYPE::COASTER, CKirby_Deform_RollerCoaster::Create())))           return E_FAIL;

    return S_OK;
}

HRESULT CKirby::Bind_ShaderResources()
{
    return S_OK;
}

HRESULT CKirby::Ready_Events()
{
    Subscribe_Event(EventTag::Query_Player,
        [this](void* pData)
        {
            static_cast<PLAYER_QUERY*>(pData)->pPlayer = this;
        }
    );

    // Attach
    Subscribe_Event(EventTag::Kirby_AttachmentBegin,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<KIRBY_ATTACHMENT_BEGIN_DESC*>(pData);
            Set_CutsceneAttachTarget(pDesc);
            m_pKirby_StateMachine->Request_Attachment_StateMachine(pDesc);
        }
    );

    Subscribe_Event(EventTag::Kirby_AttachmentEnd,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<KIRBY_ATTACHMENT_END_DESC*>(pData);
            Clear_CutsceneAttachTarget();
            m_pKirby_StateMachine->Request_Attachment_End_StateMachine(pDesc);
        }
    );

    // Position Sync
#pragma region Position Sync
    Subscribe_Event(EventTag::Kirby_PositionSyncBegin,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<KIRBY_POSITION_SYNC_BEGIN_DESC*>(pData);
            m_pKirby_StateMachine->Request_PositionSync_StateMachine(pDesc);
        }
    );

    Subscribe_Event(EventTag::Kirby_PositionSyncEnd,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<KIRBY_POSITION_SYNC_END_DESC*>(pData);
            m_pKirby_StateMachine->Request_PositionSync_End_StateMachine(pDesc);
        }
    );
#pragma endregion


    // Clear
    Subscribe_Event(EventTag::Cutscene_StageClear,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<CUTSCENE_STAGECLEAR*>(pData);
            m_pKirby_StateMachine->Request_ClearStage_StateMachine(pDesc);
        }
    );

    // SequenceLock
    Subscribe_Event(EventTag::Kirby_LevelSpawn,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<KIRBY_LEVEL_SPAWN_DESC*>(pData);
            m_pLastSpawner = pDesc->pSpawner;

            Set_OwnerLevelLayer(pDesc->iLevelIndex, m_strLayerTag);
            m_pKirby_StateMachine->Request_SequenceLock_End_StateMachine(pDesc);
            Set_Active(true);
        }
    );

    Subscribe_Event(EventTag::Kirby_LevelSleep,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<KIRBY_LEVEL_SLEEP_DESC*>(pData);

            if (pDesc->pSpawner != m_pLastSpawner)
                return;

            m_pKirby_StateMachine->Request_SequenceLock_StateMachine(pDesc);
            Set_Active(false);

            //Clear_CutsceneAttachTarget();
            Clear_Ladder();
            Set_TriggerDeformObj(nullptr);
        }
    );

    Subscribe_Event(EventTag::Kirby_HUD_Refresh,
        [this](void*)
        {
            Republish_HUDState();
        }
    );

    // Dialogue
#pragma region Dialogue
    // 위치 이동
    Subscribe_Event(EventTag::Sequence_KirbyWarp,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<SEQUENCE_KIRBY_WARP_DESC*>(pData);
            if (pDesc == nullptr)
                return;

            m_pKirby_StateMachine->Request_Dialogue_StateMachine(pDesc);
        }
    );

    // 대화 애니메이션 시작
    Subscribe_Event(EventTag::Sequence_KirbyAnim,
        [this](void* pData)
        {
            const auto* pDesc = static_cast<SEQUENCE_KIRBY_ANIM_DESC*>(pData);
            if (pDesc == nullptr)
                return;

            m_pKirby_StateMachine->Request_DialogueAnim_StateMachine(pDesc);
        }
    );
#pragma endregion

    // MetaKnight
    Subscribe_Event(EventTag::Metaknight_ParryBegin,
        [this](void*)
        {
            m_pKirby_StateMachine->Request_MetaKnight_ParryBegin_StateMachine();
        }
    );
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

    if (FAILED(Get_DeformPart_Model(DEFORM_TYPE::CYLINDER, KIRBY_DEFORM_MODEL_TYPE::DEMO)->Ready_AnimEvents(this)))
        return E_FAIL;

    if (FAILED(Get_DeformPart_Model(DEFORM_TYPE::CYLINDER, KIRBY_DEFORM_MODEL_TYPE::MAIN)->Ready_AnimEvents(this)))
        return E_FAIL;

    if (FAILED(Get_DeformPart_Model(DEFORM_TYPE::COASTER, KIRBY_DEFORM_MODEL_TYPE::MAIN)->Ready_AnimEvents(this)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby::SetUp_Collider_Callback()
{
#pragma region Hurt Box
    m_KirbyColliders[HURT_BOX]->Set_OnEnter(
        [this](CCollider* pOther)
        {
            const _uint iGroup = pOther->Get_RegisteredGroup();
            CGameObject* pGameObject = pOther->Get_Owner();

            if (iGroup == ETOUI(COLLISION_LAYER::MONSTER_HURT))
            {
                CMonster* pMonster = dynamic_cast<CMonster*>(pGameObject);
                if (pMonster == nullptr)
                    return;

                if(pMonster->IsDead())
                    return;

                if (!pMonster->Is_Touch_Harmful())
                    return;

                ATTACK_INFO tAttackDesc{};
                tAttackDesc.eHitType = HIT_TYPE::BODY_CONTACT;
                tAttackDesc.pAttacker = pMonster;
                XMStoreFloat3(&tAttackDesc.vAttackerPos, pMonster->Get_Transform()->Get_State(STATE::POSITION));
                tAttackDesc.fDamage = 10.f;
                tAttackDesc.fKnockback = 2.f;
                Damaged(tAttackDesc);
            }
            else if (iGroup == ETOUI(COLLISION_LAYER::ESSENCE_BUBBLE))
            {
                CEssenceBubble* pEssenceBubble = dynamic_cast<CEssenceBubble*>(pGameObject);
                if (pEssenceBubble == nullptr)
                    return;

                if (pEssenceBubble->Is_Available())
                    m_pKirby_StateMachine->Get_EssenceBubble(pEssenceBubble->Get_Ability());
            }
            else if (iGroup == ETOUI(COLLISION_LAYER::DEFORM_OBJECT))
            {
                Set_TriggerDeformObj(static_cast<CLD_DeformObject*>(pGameObject));
            }
            else if (iGroup == ETOUI(COLLISION_LAYER::DEFORM_RELEASE_AREA))
            {
                Set_DeformEndTrigger(true);
            }
            else if (iGroup == ETOUI(COLLISION_LAYER::EXCALIBUR))
            {
                IInhalable* pExcalibur = dynamic_cast<IInhalable*>(pGameObject);
                if (pExcalibur == nullptr)
                    return;

                m_pKirby_StateMachine->Get_EssenceBubble(pExcalibur->Get_CopyAbility());
            }
        }
    );

    m_KirbyColliders[HURT_BOX]->Set_OnStay
    (
        [this](CCollider* pOther)
        {
            const _uint iGroup = pOther->Get_RegisteredGroup();

            if (iGroup == ETOUI(COLLISION_LAYER::ENV_LADDER))
            {
                CLevelDesign_Ladder* pLadder = dynamic_cast<CLevelDesign_Ladder*>(pOther->Get_Owner());
                if (pLadder == nullptr)
                    return;

                Set_Ladder(pLadder);
            }
        }
    );

    m_KirbyColliders[HURT_BOX]->Set_OnExit
    (
        [this](CCollider* pOther)
        {
            const _uint iGroup = pOther->Get_RegisteredGroup();

            if (iGroup == ETOUI(COLLISION_LAYER::ENV_LADDER))
            {
                Clear_Ladder();
                return;
            }
            else if (iGroup == ETOUI(COLLISION_LAYER::DEFORM_OBJECT))
            {
                Set_TriggerDeformObj(nullptr);
            }
            else if (iGroup == ETOUI(COLLISION_LAYER::DEFORM_RELEASE_AREA))
            {
                Set_DeformEndTrigger(false);
            }
        }
    );
#pragma endregion

#pragma region Breakable HitBox
    m_KirbyColliders[BREAKERABLE_HITBOX]->Set_OnEnter(
        [this](CCollider* pOther)
        {
            if (pOther->Get_RegisteredGroup() == ETOUI(COLLISION_LAYER::MONSTER_HURT))
            { 
                CMonster* pMonster = dynamic_cast<CMonster*>(pOther->Get_Owner());
                if (pMonster == nullptr)
                    return;

                ATTACK_INFO tAttackInfo{};
                tAttackInfo.eHitType = HIT_TYPE::BREAKERABLE_HIT;
                tAttackInfo.pAttacker = this;
                XMStoreFloat3(&tAttackInfo.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
                tAttackInfo.fDamage = 0.f;
                tAttackInfo.fKnockback = 0.f;
                pMonster->Damaged(tAttackInfo);
                return;
            }
            else if (pOther->Get_RegisteredGroup() == ETOUI(COLLISION_LAYER::ENV_HURT))
            {
                if (auto* pDamageable = dynamic_cast<IDamageable*>(pOther->Get_Owner()))
                {
                    ATTACK_INFO tAttackDesc{};
                    tAttackDesc.eHitType = HIT_TYPE::BREAKERABLE_HIT;
                    tAttackDesc.pAttacker = this;
                    XMStoreFloat3(&tAttackDesc.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
                    tAttackDesc.fDamage = 0.f;
                    tAttackDesc.fKnockback = 0.f;
                    pDamageable->Damaged(tAttackDesc);
                    return;
                }
            }
        }
    );
#pragma endregion

    Route_CollisionToState(KIRBY_COLLIDER::SLIDE_COLLIDER);

    return S_OK;
}

void CKirby::Route_CollisionToState(KIRBY_COLLIDER eCollider)
{
    CCollider* pCollider = m_KirbyColliders[eCollider];

    pCollider->Set_OnEnter(
        [this, eCollider](CCollider* pOther)
        {
            m_pKirby_StateMachine->On_KirbyCollisionEnter_StateMachine(ETOUI(eCollider), pOther);
        }
    );

    pCollider->Set_OnStay(
        [this, eCollider](CCollider* pOther)
        {
            m_pKirby_StateMachine->On_KirbyCollisionStay_StateMachine(ETOUI(eCollider), pOther);
        }
    );

    pCollider->Set_OnExit(
        [this, eCollider](CCollider* pOther)
        {
            m_pKirby_StateMachine->On_KirbyCollisionExit_StateMachine(ETOUI(eCollider), pOther);
        }
    );
}

_bool CKirby::Block_Hit(const ATTACK_INFO& tInfo) 
{ 
    return Is_Invincible();
}

void  CKirby::On_Damaged(const ATTACK_INFO& tInfo)
{
    m_pKirby_StateMachine->On_Damaged_KirbyStateMachine(tInfo);
}

void CKirby::Set_CutsceneAttachTarget(const KIRBY_ATTACHMENT_BEGIN_DESC* pAttachDesc)
{
    m_pAttachBone = pAttachDesc->pBoneMatrix;
    m_pAttachOwnerWorld = pAttachDesc->pSourceWorld;
    m_vPreAttachScale = Get_Transform()->Get_Scaled();

    m_bIsAttach = true;
}

void CKirby::Clear_CutsceneAttachTarget()
{
    m_pAttachBone = nullptr;
    m_pAttachOwnerWorld = nullptr;

    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    _vector vLook = XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vLook)) < Helper::fEpsilon)
        vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);

    vLook = XMVector3Normalize(vLook);

    _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
    vLook = XMVector3Normalize(XMVector3Cross(vRight, vUp));

    _matrix matNewWorld = XMMatrixIdentity();
    matNewWorld.r[0] = XMVectorSetW(vRight * m_vPreAttachScale.x, 0.f);
    matNewWorld.r[1] = XMVectorSetW(vUp * m_vPreAttachScale.y, 0.f);
    matNewWorld.r[2] = XMVectorSetW(vLook * m_vPreAttachScale.z, 0.f);
    matNewWorld.r[3] = XMVectorSetW(vPos, 1.f);
   
    m_pTransformCom->Set_WorldMatrix(matNewWorld);
    m_pMovement->Sync_To_Controller();

    XMStoreFloat4x4(&m_RenderWorldMatrix, matNewWorld);
    XMStoreFloat3(&m_vRenderGroundNormal, vUp);

    m_vPreAttachScale = { 1.f,1.f,1.f };
    m_bIsAttach = false;
}

void CKirby::Update_RenderWorldMatrix(_float fTimeDelta)
{
    _matrix matBaseWorld = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
    _vector vBaseUp = XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP));

    // 누군가에게 부착되어 있을때 적용 x
    if (m_bIsAttach || !m_bUseRenderGroundAlign)
    {
        XMStoreFloat4x4(&m_RenderWorldMatrix, matBaseWorld);
        XMStoreFloat3(&m_vRenderGroundNormal, vBaseUp);
        return;
    }    

    // Normal 못 구하면 적용 x
    _float3 vTargetGroundNormal = { 0.f, 1.f, 0.f };
    if (Cal_GroundNormal(vTargetGroundNormal) == false)
    {
        XMStoreFloat4x4(&m_RenderWorldMatrix, matBaseWorld);
        XMStoreFloat3(&m_vRenderGroundNormal, vBaseUp);
        return;
    }

    _vector vTargetNormal = XMVector3Normalize(XMLoadFloat3(&vTargetGroundNormal));

    constexpr _float fSpeed = 12.f;
    _float fRatio = 1.f - expf(-fSpeed * fTimeDelta);
    _vector vCurNormal = XMVector3Normalize(XMLoadFloat3(&m_vRenderGroundNormal));
    _vector vFinalUp = XMVector3Normalize(XMVectorLerp(vCurNormal, vTargetNormal, fRatio));

    _vector vLook = XMVector3Normalize(
        XMVectorSetW(m_pTransformCom->Get_State(STATE::LOOK), 0.f));

   vLook = vLook - vFinalUp * XMVector3Dot(vLook, vFinalUp);

   // 같은 방향이거나 반대 방향이면
   if (XMVectorGetX(XMVector3LengthSq(vLook)) < Helper::fEpsilon)
   {
       // 그냥 원래 방향으로 세팅./
       XMStoreFloat4x4(&m_RenderWorldMatrix, matBaseWorld);
       XMStoreFloat3(&m_vRenderGroundNormal, vBaseUp);
       return;
   }

   vLook = XMVector3Normalize(vLook);
   _vector vRight = XMVector3Normalize(XMVector3Cross(vFinalUp, vLook));
   vLook = XMVector3Normalize(XMVector3Cross(vRight, vFinalUp));

   _float3 vScale = m_pTransformCom->Get_Scaled();
   matBaseWorld.r[0] = XMVectorSetW(vRight * vScale.x, 0.f);
   matBaseWorld.r[1] = XMVectorSetW(vFinalUp * vScale.y, 0.f);
   matBaseWorld.r[2] = XMVectorSetW(vLook * vScale.z, 0.f);

   XMStoreFloat4x4(&m_RenderWorldMatrix, matBaseWorld);

   XMStoreFloat3(&m_vRenderGroundNormal, vFinalUp);
}

_bool CKirby::Cal_GroundNormal(_float3& vGroundNormal)
{
    _vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

    _float3 vCenter{};
    XMStoreFloat3(&vCenter, vPos + XMVectorSet(0.f, 0.35f, 0.f, 0.f));

    const _float3 vHalfExtents = { 0.2f, 0.15f, 0.2f };
    const _float4 vRot = { 0.f, 0.f, 0.f, 1.f };
    const _float3 vDir = { 0.f, -1.f, 0.f };
    const _float fMaxDist = 1.2f;

    _float fHitDist = 0.f;
    _float3 vHitNormal{ 0.f, 1.f, 0.f };

    if (!m_pGameInstance_Proxy->Sweep_Box(vCenter, vHalfExtents,
        vRot, vDir, fMaxDist, &vHitNormal, &fHitDist,
        true, false))
    {
        vGroundNormal = { 0.f, 1.f, 0.f };
        return false;
    }

    if (vHitNormal.y < 0.5f)
    {
        vGroundNormal = { 0.f, 1.f, 0.f };
        return false;
    }

    vGroundNormal = vHitNormal;
    return true;
}

void CKirby::Republish_HUDState()
{
    KIRBY_HP_UPDATED tHP{};
    tHP.fCurrHp = m_fCurHP;
    tHP.fMaxHP = m_fMaxHP;
    tHP.bSnap = true;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_HP_Updated, &tHP);

    KIRBY_NAME_UPDATED tName{};
    tName.strAtkModeName = Get_ActiveAttackMode()->Get_AttackModeName();
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_Name_Updated, &tName);
}

void CKirby::Update_BlobShadow()
{
    constexpr _float fLightHeight = 5.f;
    constexpr _float fProjectionSize = 3.5f;
    constexpr _float fNearPlane = 0.1f;
    constexpr _float fFarPlanePadding = 3.f;

    const _vector vKirbyPosition = m_pTransformCom->Get_State(STATE::POSITION);
    const _vector vLightPosition = vKirbyPosition + XMVectorSet(0.f, fLightHeight, 0.f, 0.f);

    SHADOW_LIGHT_DESC ShadowDesc{};

    XMStoreFloat4(&ShadowDesc.vEye, vLightPosition);
    XMStoreFloat4(&ShadowDesc.vAt, XMVectorSetW(vKirbyPosition, 1.f));

    ShadowDesc.fWidth = fProjectionSize;
    ShadowDesc.fHeight = fProjectionSize;
    ShadowDesc.fNear = fNearPlane;
    ShadowDesc.fFar = fLightHeight + fFarPlanePadding;

    m_pGameInstance_Proxy->Update_BlobShadow(ShadowDesc);
}

void CKirby::Update_InvincibilityHitFlash()
{
    if (m_fInvincibleTime <= 0.f)
    {
        m_fHitFlashCur = 0.f;
        return;
    }

    const _float fInvincibilityElapsedTime = m_fInvincibleDuration - m_fInvincibleTime;

    constexpr _float fInitialFlashDuration = 0.12f;
    if (fInvincibilityElapsedTime < fInitialFlashDuration)
    {
        m_fHitFlashCur = 1.f;
        return;
    }

    constexpr _float fBlinkFrequency = 8.f;
    const _float fBlinkPhase = fmodf(m_fInvincibleTime * fBlinkFrequency, 1.f);

    constexpr _float fBlinkIntensity = 0.1f;
    m_fHitFlashCur = fBlinkPhase < 0.5f ? fBlinkIntensity : 0.f;
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

CKirby_Deform* CKirby::Find_KirbyDeform(DEFORM_TYPE eType)
{
    auto iter = m_Deformations.find(eType);

    if (iter == m_Deformations.end())
        return nullptr;

    return iter->second;
}

CCollider* CKirby::Get_Collider(KIRBY_COLLIDER eKirbyCollider)
{
    return m_KirbyColliders[eKirbyCollider];
}

void CKirby::Set_ColliderDesc(KIRBY_COLLIDER eKirbyCollider, const CCollider::COLLIDER_DESC& Desc)
{
    CCollider* pCollider = m_KirbyColliders[eKirbyCollider];
    if (pCollider == nullptr)
        return;

    pCollider->Reset_Bounding(Desc);
    pCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CKirby::Set_ColliderEnabled(KIRBY_COLLIDER eKirbyCollider, _bool bEnabled)
{
    CCollider* pCollider = m_KirbyColliders[eKirbyCollider];
    if (pCollider)
        pCollider->Set_Enabled(bEnabled);
}

void CKirby::Update_CutsceneAttachTransform()
{
    if (m_pAttachBone == nullptr || m_pAttachOwnerWorld == nullptr)
        return;

    _matrix matGrabTargetWorld = XMMatrixRotationY(XMConvertToRadians(-180.f))
        * XMLoadFloat4x4(m_pAttachBone) * XMLoadFloat4x4(m_pAttachOwnerWorld);
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

    KIRBY_HP_UPDATED tDesc{};
    tDesc.fCurrHp = m_fCurHP;
    tDesc.fMaxHP = m_fMaxHP;
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_HP_Updated, &tDesc);
}

void CKirby::Start_DamageInvincibility()
{
    Start_Invincibility(m_fInvincibleDuration);
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
