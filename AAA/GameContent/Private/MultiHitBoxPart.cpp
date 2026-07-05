#include "MultiHitBoxPart.h"
#include "GameInstance.h"
#include "Collider.h"
#include "Transform.h"
#include "Damageable.h"

CMultiHitBoxPart::CMultiHitBoxPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterPart(pDevice, pContext) {
}

CMultiHitBoxPart::CMultiHitBoxPart(const CMultiHitBoxPart& Prototype)
    : CMonsterPart(Prototype) {
}

void CMultiHitBoxPart::SetUp_HitBox_Callback(_int iIndex)
{
    m_HitBoxes[iIndex].pCollider->Set_OnEnter([this, iIndex](CCollider* pOther)
        {
            if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
                return;

            CGameObject* pVictimObj = pOther->Get_Owner();
            HITBOX& hb = m_HitBoxes[iIndex];

            IDamageable* pVictim = dynamic_cast<IDamageable*>(pVictimObj);
            if (nullptr == pVictim) return;

            ATTACK_INFO atk{};
            atk.fDamage = hb.fDamage;
            atk.fKnockback = hb.fKnockback;
            atk.vAttackerPos = _float3(m_CombinedWorldMatrix._41, m_CombinedWorldMatrix._42, m_CombinedWorldMatrix._43);
            atk.pAttacker = this;
            pVictim->Damaged(atk);
        });
}

void CMultiHitBoxPart::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    for (auto& hb : m_HitBoxes)
    {
        if (nullptr == hb.pCollider || !hb.pCollider->Is_Enabled())
            continue;

        _matrix matWorld = (hb.pBoneMatrix)
            ? XMLoadFloat4x4(hb.pBoneMatrix) * XMLoadFloat4x4(&m_CombinedWorldMatrix)
            : XMLoadFloat4x4(&m_CombinedWorldMatrix);

        hb.pCollider->Update(matWorld);
#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(hb.pCollider);
#endif
    }
}

void CMultiHitBoxPart::Enable_HitBox(_int iIndex, _bool b)
{
    if (iIndex < 0 || iIndex >= (_int)m_HitBoxes.size()) return;
    m_HitBoxes[iIndex].pCollider->Set_Enabled(b);
    if (b) m_HitBoxes[iIndex].setHit.clear();    
}

void CMultiHitBoxPart::Enable_AllHitBoxes(_bool b)
{
    for (_int i = 0; i < (_int)m_HitBoxes.size(); ++i)
        Enable_HitBox(i, b);
}

HRESULT CMultiHitBoxPart::Add_HitBox(_int iIndex, const _char* szBone, COLLIDER eShape, _float fRadius, _float fHeight, _float fDamage, _float fKnockback, const _float3& vCenter, const _float3& vRadians)
{
    if (iIndex < 0) return E_FAIL;
    if (iIndex >= (_int)m_HitBoxes.size())
        m_HitBoxes.resize(iIndex + 1);     

    const COMPONENT_DESC& tProto = (eShape == COLLIDER::SPHERE) ? Collider_Sphere : Collider_Capsule;

    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.vCenter = vCenter;
    desc.fRadius = fRadius;
    desc.fHeight = fHeight;
    desc.vRadians = vRadians;

    _tchar szTag[64];
    swprintf_s(szTag, L"HitBox_%d", iIndex);

    CCollider* pCol = Add_Component<CCollider>(tProto.iLevelID, tProto.szProtoTag, szTag, &desc);
    if (nullptr == pCol) return E_FAIL;

    pCol->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(pCol, ETOUI(COLLISION_LAYER::MONSTER_HIT));

    HITBOX& hb = m_HitBoxes[iIndex];
    hb.pCollider = pCol;
    hb.pBoneMatrix = Get_BoneMatrixPtr(szBone);
    hb.fDamage = fDamage;
    hb.fKnockback = fKnockback;

#ifdef _DEBUG
    if (nullptr == hb.pBoneMatrix)
        OutputDebugStringA((std::string("[MultiHitBox] 본 못찾음: ") + szBone + "\n").c_str());
#endif

    SetUp_HitBox_Callback(iIndex);
    return S_OK;
}

void CMultiHitBoxPart::Free()
{
    __super::Free();
}