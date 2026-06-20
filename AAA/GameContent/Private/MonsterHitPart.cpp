#include "MonsterHitPart.h"
#include "GameInstance.h"
#include "Collider.h"
#include "Transform.h"
#include "Damageable.h"

CMonsterHitPart::CMonsterHitPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}

CMonsterHitPart::CMonsterHitPart(const CMonsterHitPart& Prototype)
    : CMonsterPart(Prototype) {
}

void CMonsterHitPart::SetUp_HitBox_Callback()
{
    m_pHitBox->Set_OnEnter([this](CCollider* pOther)
        {
            if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
                return;

            IDamageable* pVictim = dynamic_cast<IDamageable*>(pOther->Get_Owner());
            if (nullptr == pVictim) return;

            ATTACK_INFO atk{};
            atk.fDamage = m_fDamage;
            atk.fKnockback = m_fKnockback;
            XMStoreFloat3(&atk.vAttackerPos, m_pTransformCom->Get_State(STATE::POSITION));
            atk.pAttacker = this;
            pVictim->Damaged(atk);
#ifdef _DEBUG
            OutputDebugStringA("[MonsterHitPart] HIT player!\n");
#endif
        });
}

void CMonsterHitPart::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);    

    if (m_pHitBox && m_pHitBox->Is_Enabled())
    {
        m_pHitBox->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));
#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(m_pHitBox);
#endif
    }
}

void CMonsterHitPart::Set_Drawn(_bool bDrawn)
{
    Set_Active(bDrawn);            
    if (m_pHitBox) m_pHitBox->Set_Enabled(bDrawn);
}

HRESULT CMonsterHitPart::Ready_HitBox(const CAPSULE_DESC& Desc, _float fDamage, _float fKnockback)
{
    m_fDamage = fDamage;        
    m_fKnockback = fKnockback;

    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.vCenter = Desc.vCenter;
    desc.fRadius = Desc.fRadius;
    desc.fHeight = Desc.fHeight;
    desc.vRadians = Desc.vRadians;
    m_pHitBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        TEXT("HitBox_Com"), &desc);
    if (nullptr == m_pHitBox) return E_FAIL;

    SetUp_HitBox_Callback();                                                 
    m_pHitBox->Set_Enabled(false);                                           
    m_pGameInstance_Proxy->Register_Collider(m_pHitBox,                      
        ETOUI(COLLISION_LAYER::MONSTER_HIT));

    return S_OK;
}

void CMonsterHitPart::Free()
{
    __super::Free();                   
}