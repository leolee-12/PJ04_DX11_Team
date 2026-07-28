#include "Kabu_State_WarpIn.h"
#include "Kabu.h"
#include "Monster_RailMovement.h"
#include "MonsterPart.h"
#include "Effect_Loader.h"

HRESULT CKabu_State_WarpIn::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CKabu_State_WarpIn::Get_StateType()
{
	return MONSTER_STATE_TYPE::WARPIN;
}

void CKabu_State_WarpIn::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner)
		return;

    if (m_pMovement)
        static_cast<CMonster_RailMovement*>(m_pMovement)->Warp_ToRandomPoint();

    static_cast<CKabu*>(m_pOwner)->Set_Visible(true);

    if (m_pAnimator && !m_PlayInfo.strAniName.empty())
        m_pAnimator->Play(&m_PlayInfo);

	CKabu* pKabu = static_cast<CKabu*>(m_pOwner);
	CTransform* pTransform = pKabu->Get_Transform();

	if (!pKabu->Is_RenderCulled())
	{
		_float3 vPos{};
		XMStoreFloat3(&vPos, pTransform->Get_State(STATE::POSITION));
		vPos.y += 0.5f;

		const auto& Parts = pKabu->Get_PartObjects();
		auto it = Parts.find(TEXT("Body"));
		if (it != Parts.end())
		{
			CMonsterPart* pBody = static_cast<CMonsterPart*>(it->second);
			const _float4x4* pBone =
				nullptr != pBody ? pBody->Get_BoneMatrixPtr("CenterL") : nullptr;
			if (nullptr != pBone)
			{
				_matrix matBoneWorld =
					XMLoadFloat4x4(pBone) * XMLoadFloat4x4(pTransform->Get_WorldMatrixPtr());
				XMStoreFloat3(&vPos, matBoneWorld.r[3]);
			}
		}

		CEffect_Loader::GetInstance()->Spawn(
			TEXT("WarpInEffect"), pKabu->Get_LevelIndex(), vPos);
	}
}

void CKabu_State_WarpIn::Update(_float fTimeDelta)
{
    if (nullptr == m_pOwner || nullptr == m_pAnimator)
        return;

    if (!m_pAnimator->Is_Finished())
        return;

    if (m_pAnimator->Is_Finished())
        m_pOwner->Change_State(MONSTER_STATE_TYPE::IDLE);
}

void CKabu_State_WarpIn::Exit(MONSTER_STATE_TYPE eNextState)
{
    if (nullptr == m_pOwner)
        return;
}

CKabu_State_WarpIn* CKabu_State_WarpIn::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
    CKabu_State_WarpIn* pInstance = new CKabu_State_WarpIn();
    if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
    {
        MSG_BOX("Failed to Created : CKabu_State_WarpIn");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CKabu_State_WarpIn::Free()
{
    __super::Free();
}
