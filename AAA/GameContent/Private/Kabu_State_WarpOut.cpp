#include "Kabu_State_WarpOut.h"
#include "Kabu.h"
#include "MonsterPart.h"
#include "Effect_Loader.h"

HRESULT CKabu_State_WarpOut::Initialize(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	if (FAILED(__super::Initialize(tInfo, fSpeed)))
		return E_FAIL;

	return S_OK;
}

MONSTER_STATE_TYPE CKabu_State_WarpOut::Get_StateType()
{
	return MONSTER_STATE_TYPE::WARPOUT;
}

void CKabu_State_WarpOut::Enter(MONSTER_STATE_TYPE ePrevState)
{
	if (nullptr == m_pOwner)
		return;

	m_fTimer = 0.f;
	m_bStartFxPlayed = false;
	m_bEndFxPlayed = false;

	CTransform* pTransform = m_pOwner->Get_Transform();
	XMStoreFloat4(&m_vSavedRight, pTransform->Get_State(STATE::RIGHT));
	XMStoreFloat4(&m_vSavedUp, pTransform->Get_State(STATE::UP));
	XMStoreFloat4(&m_vSavedLook, pTransform->Get_State(STATE::LOOK));

	if (m_pAnimator && !m_PlayInfo.strAniName.empty())
		m_pAnimator->Play(&m_PlayInfo);
}

void CKabu_State_WarpOut::Update(_float fTimeDelta)
{
	if (nullptr == m_pOwner)
		return;

	CKabu* pKabu = static_cast<CKabu*>(m_pOwner);
	CTransform* pTransform = pKabu->Get_Transform();
	const _float fPrevTime = m_fTimer;
	m_fTimer += fTimeDelta;

	if (fPrevTime < s_fSpinDuration)
	{
		const _float fSpinBeginTime = fPrevTime > 0.f ? fPrevTime : 0.f;
		const _float fSpinEndTime = m_fTimer < s_fSpinDuration ? m_fTimer : s_fSpinDuration;
		if (fSpinEndTime > fSpinBeginTime)
		{
			const _float fBeginSpeed = s_fSpinMaxDegPerSec * (fSpinBeginTime / s_fSpinDuration);
			const _float fEndSpeed = s_fSpinMaxDegPerSec * (fSpinEndTime / s_fSpinDuration);
			const _float fSpinDegrees =
				0.5f * (fBeginSpeed + fEndSpeed) * (fSpinEndTime - fSpinBeginTime);
			pTransform->Rotate(XMQuaternionRotationAxis(
				XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fSpinDegrees)));
		}
	}

	if (!m_bStartFxPlayed && m_fTimer >= s_fSpinDuration - s_fStartLead)
	{
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
				TEXT("WarpOutStart"), pKabu->Get_LevelIndex(), vPos);
		}
		m_bStartFxPlayed = true;
	}

	if (!m_bEndFxPlayed && m_fTimer >= s_fSpinDuration)
	{
		pKabu->Set_Visible(false);

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
				TEXT("WarpOutEnd"), pKabu->Get_LevelIndex(), vPos);
		}
		m_bEndFxPlayed = true;
	}

	if (m_bEndFxPlayed && m_fTimer >= s_fSpinDuration + s_fWarpInvisibleTime)
		m_pOwner->Change_State(MONSTER_STATE_TYPE::WARPIN);

}

void CKabu_State_WarpOut::Exit(MONSTER_STATE_TYPE eNextState)
{
	if (nullptr == m_pOwner)
		return;

	CTransform* pTransform = m_pOwner->Get_Transform();
	pTransform->Set_State(STATE::RIGHT, XMLoadFloat4(&m_vSavedRight));
	pTransform->Set_State(STATE::UP, XMLoadFloat4(&m_vSavedUp));
	pTransform->Set_State(STATE::LOOK, XMLoadFloat4(&m_vSavedLook));
}

CKabu_State_WarpOut* CKabu_State_WarpOut::Create(const ANI_PLAY_INFO& tInfo, _float fSpeed)
{
	CKabu_State_WarpOut* pInstance = new CKabu_State_WarpOut();
	if (FAILED(pInstance->Initialize(tInfo, fSpeed)))
	{
		MSG_BOX("Failed to Created : CKabu_State_WarpOut");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CKabu_State_WarpOut::Free()
{
	__super::Free();
}
