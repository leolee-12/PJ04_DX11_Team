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

#include "Kirby_InputManager.h"
#include "Kirby_Controller.h"
#include "Kirby_StateMachine.h"

// Ability
#include "Kirby_State.h"
#include "Kirby_Ability_Normal.h"
#include "Kirby_Ability_Sword.h"

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

    if (FAILED(Ready_System()))
        return E_FAIL;

    SetUp_Collider_Callback();
  
    return S_OK;
}

void CKirby::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CKirby::Update(_float fTimeDelta)
{
    XMStoreFloat3(&m_vWishDir, XMVectorZero());

    __super::Update(fTimeDelta);

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
}

void CKirby::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_KirbyColliders[KIRBY_COLLIDER::HURT_BOX] && m_pTransformCom)
    {
        m_KirbyColliders[KIRBY_COLLIDER::HURT_BOX]->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(m_KirbyColliders[KIRBY_COLLIDER::HURT_BOX]);
#endif
    }

    if (m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX] && m_pTransformCom)
    {
        m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX]->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
        if (m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX]->Is_Enabled())
            m_pGameInstance_Proxy->Add_DebugComponent(m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX]);
#endif
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

void CKirby::OnOffParts(COPY_ABILITY_TYPE eAbilityType, _bool fOn)
{
    auto OnOffPart = [this](const wchar_t* PartTag, _bool bOn)->void
        {
            auto iter = m_PartObjects.find(PartTag);
            if (iter != m_PartObjects.end())
            {
                static_cast<CKirby_OnOffPart*>(iter->second)->PartOnOff(bOn);
            }
        };

    switch (eAbilityType)
    {
        case COPY_ABILITY_TYPE::SWORD:
            OnOffPart(CKirby_Sword::Kirby_PartTag, fOn);
            OnOffPart(CKirby_SwordHat::Kirby_PartTag, fOn);
            break;
    }
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
    InhaleDesc.vCenter = _float3(0.f, s_fInhaleUp, s_fInhaleFwd);
    InhaleDesc.vSize = s_vInhaleSize;
    InhaleDesc.vRadians = _float3(0.f, 0.f, 0.f);

    m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX] = Add_Component<CCollider>(Collider_OBB.iLevelID, Collider_OBB.szProtoTag,
        TEXT("InhaleBox_Com"), &InhaleDesc);
    if (m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX] == nullptr)
        return E_FAIL;

    m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX]->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_KirbyColliders[KIRBY_COLLIDER::INHALE_BOX], ETOUI(COLLISION_LAYER::PLAYER_INHALE));

    //임시
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_INHALE), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_HIT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_PROJECTILE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::MONSTER_D_RANGE));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HURT), ETOUI(COLLISION_LAYER::ENV_TRIGGER));

    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_HIT),        ETOUI(COLLISION_LAYER::MONSTER_HURT));
    m_pGameInstance_Proxy->Add_CollisionPool(ETOUI(COLLISION_LAYER::PLAYER_PROJECTILE), ETOUI(COLLISION_LAYER::MONSTER_HURT));

    return S_OK;
}

void CKirby::SetUp_Collider_Callback()
{
    if (m_KirbyColliders[HURT_BOX])
    {
        m_KirbyColliders[HURT_BOX]->Set_OnEnter(
            [this](CCollider* pOther)
            {
                if (ETOUI(COLLISION_LAYER::MONSTER_HURT) == pOther->Get_RegisteredGroup())
                {
                    _vector vAtkPos = pOther->Get_Owner()->Get_Transform()->Get_State(STATE::POSITION);
                    ATTACK_INFO atk{};
                    atk.fDamage = 1.f;
                    atk.fKnockback = 6.f;                     
                    XMStoreFloat3(&atk.vAttackerPos, vAtkPos);
                    atk.pAttacker = pOther->Get_Owner();
                    Damaged(atk);
#ifdef _DEBUG
                    char szBuf[128];
                    sprintf_s(szBuf, "[Kirby] Hurt! HP %.0f/%.0f\n", m_fCurHP, m_fMaxHP);
                    OutputDebugStringA(szBuf);
#endif
                }
            });
        
        //m_KirbyColliders[HURT_BOX]->Set_OnStay([this](CCollider* pOther) {
        //      여기에 콜백을
        //    });
        //
        //m_KirbyColliders[HURT_BOX]->Set_OnExit([this](CCollider* pOther) {
        //      넣으시오
        //    });
    }
}

HRESULT CKirby::Ready_PartObjects()
{
    // Body
    CKirby_Body::KIRBY_BODY_DESC BodyDesc{};
    BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_Body::PROTOTYPE_TAG,
        TEXT("Body"), &BodyDesc)))
        return E_FAIL;

    m_pBody = dynamic_cast<CKirby_Body*>(m_PartObjects[TEXT("Body")]);

    // Sword
    CKirby_Sword::KIRBY_SWORD_DESC SwordDesc{};
    SwordDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    SwordDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("RHaveL");

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_Sword::PROTOTYPE_TAG,
        CKirby_Sword::Kirby_PartTag, &SwordDesc)))
        return E_FAIL;

    // SwordHat
    CKirby_SwordHat::KIRBY_SWORDHAT_DESC SwordHatDesc{};
    SwordHatDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    SwordHatDesc.pSocketBoneMatrix = m_pBody->Get_BoneMatrixPtr("HatL");

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CKirby_SwordHat::PROTOTYPE_TAG,
        CKirby_SwordHat::Kirby_PartTag, &SwordHatDesc)))
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

    return S_OK;
}

_bool CKirby::Block_Hit(const ATTACK_INFO& tInfo) 
{ 
    return m_fInvincibleTime > 0.f; 
}

void  CKirby::On_Damaged(const ATTACK_INFO& tInfo)
{
    m_fInvincibleTime = s_fInvincibleDur;

    m_pMovement->Apply_Knockback(tInfo.vAttackerPos, tInfo.fKnockback * 5.f, tInfo.fKnockback * 1.5f);
    m_pKirby_StateMachine->On_Damaged_KirbyStateMachine(tInfo);
}

void CKirby::Update_Timer(_float fTimeDelta)
{
    if (m_fInvincibleTime > 0.f)
        m_fInvincibleTime -= fTimeDelta;
}

void CKirby::Spit_Inhalable()
{
    if (m_pCapturedInhalable == nullptr)
        return;

    _vector vMouth =
        m_pTransformCom->Get_State(STATE::POSITION)
        + m_pTransformCom->Get_State(STATE::LOOK) * s_fInhaleFwd
        + m_pTransformCom->Get_State(STATE::UP) * s_fInhaleUp;
    _vector vDir = m_pTransformCom->Get_State(STATE::LOOK);

    m_pCapturedInhalable->Be_Spat(vMouth, vDir, s_fSpitSpeed);

    m_pCapturedInhalable = nullptr;
}

CCollider* CKirby::Get_Collider(KIRBY_COLLIDER eKirbyCollider)
{
    return m_KirbyColliders[eKirbyCollider];
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

    m_KirbyColliders.clear();

    Safe_Release(m_pKirby_InputManager);
    Safe_Release(m_pKirby_Controller);
    Safe_Release(m_pKirby_StateMachine);

    __super::Free();
}