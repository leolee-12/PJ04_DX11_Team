#include "Gigatzo.h"
#include "GameInstance.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"
#include "LevelDesign_LoadTypes.h"
#include "Parsing_Utils.h"

#include "Gigatzo_Brain.h"
#include "Gigatzo_Body.h"
#include "Monster_Movement.h"

#include "Monster_StateMachine.h"
#include "Monster_State_Idle.h"
#include "Gigatzo_State_Attack.h"

#include "Projectile_Manager.h"
#include "GigatzoBullet.h"

namespace
{
	_int Lv_Num(const _wstring& s)              // "Lv5" -> 5
	{
		return (s.size() > 2) ? _wtoi(s.c_str() + 2) : 0;
	}

	_float Level_To_Speed(const _wstring& s)    // Æ©´×: Lv2~16 Lv3~19 Lv5~25
	{
		return 10.f + Lv_Num(s) * 3.f;
	}

	_float Level_To_Damage(const _wstring& s)   // Æ©´×: Lv3=3 Lv5=5 Lv6=6
	{
		return static_cast<_float>(max(1, Lv_Num(s)));
	}
}

CGigatzo::CGigatzo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CGigatzo::CGigatzo(const CGigatzo& Prototype)
	: CMonster(Prototype)
{
}

HRESULT CGigatzo::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CGigatzo::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_TraitFlags = MT_NONE;
	m_eCopyAbility = COPY_ABILITY_TYPE::NONE;
	m_fCullDist = 95.f;
	Read_FireParams(pArg);

	return S_OK;
}

CAnimator* CGigatzo::Get_BodyAnimator() const
{
	return m_pBody ? m_pBody->Get_Animator() : nullptr ;
}

CMonsterBrain* CGigatzo::Create_Brain()
{
	return CGigatzo_Brain::Create(this);
}

HRESULT CGigatzo::Create_Movement()
{
    auto pMove = Add_Component<CMonster_Movement>(TEXT("Com_Movement"),
        CMonster_Movement::Create(m_pDevice, m_pContext));
    if (nullptr == pMove)
        return E_FAIL;

    pMove->Set_GravityEnabled(false);   // °íÁ¤ Æ÷Å¾(BodyMapColl None -> Áß·Â ÄÑ¸é ³«ÇÏ)
    m_pMovement = pMove;
    return S_OK;
}

void CGigatzo::Read_FireParams(void* pArg)
{
    auto pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);
    if (nullptr == pDesc)
        return;

    const json& j = pDesc->jRaw;
    const _string strBase = "Chara.Gigatzo.Variation.";

    _wstring strAtk, strSpd, strVert;
    _uint iLife = 270;
    _uint iInit = 0;

    JsonUtils::Try_ReadString(j, strBase + "AttackLevel",   &strAtk);
    JsonUtils::Try_ReadString(j, strBase + "SpeedLevel",    &strSpd);
    JsonUtils::Try_ReadString(j, strBase + "VerticalAngle", &strVert);
    JsonUtils::Try_ReadUInt(j, strBase + "LifeFrame",       &iLife);
    JsonUtils::Try_ReadUInt(j, strBase + "InitWaitFrame",   &iInit);

    m_fBulletSpeed   = Level_To_Speed(strSpd);
    m_fBulletDamage  = Level_To_Damage(strAtk);
    m_fBulletLife    = iLife / 60.f;     // ÇÁ·¹ÀÓ->ÃÊ(60fps °¡Á¤)
    m_fInitWaitDelay = iInit / 60.f;

    if (strVert == L"Vertical")   m_ePitch = PITCH::VERTICAL;
    else if (strVert == L"Tilt")  m_ePitch = PITCH::TILT;
    else                          m_ePitch = PITCH::HORIZONTAL;
}

HRESULT CGigatzo::Ready_State()
{
	if (m_pStateMachine == nullptr)
		return E_FAIL;

	CAnimator::ANI_PLAY_INFO Info{};

	// IDLE
	Info.strAniName = "Wait";
	Info.bLoop = true;
	Info.fSpeed = 1.f;
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::IDLE, CMonster_State_Idle::Create(Info))))
		return E_FAIL;

	// ATTACK
	if (FAILED(m_pStateMachine->Register_State(MONSTER_STATE_TYPE::ATTACK, CGigatzo_State_Attack::Create())))
		return E_FAIL;

	return S_OK;
}

HRESULT CGigatzo::Ready_PartObjects()
{
	m_pBody = Add_MonsterPart<CGigatzo_Body>(CGigatzo_Body::PROTOTYPE_TAG, TEXT("Body"));
	if (nullptr == m_pBody)
		return E_FAIL;

	return S_OK;
}

HRESULT CGigatzo::Ready_AnimEvents()
{
	CAnimator* pAnimator = Get_BodyAnimator();
	if (nullptr == pAnimator)
		return E_FAIL;

	pAnimator->Set_EventCallback([this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
		{
			if (Handle_SoundAnimEvent(e, phase))
				return;
			if (Handle_FxAnimEvent(e, phase))
				return;

			if (phase != ANIM_EVENT_PHASE::POINT)
				return;

			switch (static_cast<EANIM_EVENT>(e.iEventType))
			{
				case EANIM_EVENT::Projectile:
				{
					if (phase == ANIM_EVENT_PHASE::POINT)
						Fire_Bullet();
					break;
				}
				default:		
					break;
			}
		});
	return S_OK;
}

const _float4x4* CGigatzo::Get_FxParentMatrix(const _wstring& strFx) const
{
	// º°µµ ÀÌÆåÆ® ºÎÂø ½Ã ¿¬°á

	return m_pTransformCom->Get_WorldMatrixPtr();
}

void CGigatzo::Update(_float fTimeDelta)
{
	CAnimator* pAnim = Get_BodyAnimator();
	if (nullptr == pAnim)
		return;

	_float fAngleDeg = (m_ePitch == PITCH::HORIZONTAL) ? 90.f
		: (m_ePitch == PITCH::TILT) ? 45.f
		: 0.f;

	if (0.f != fAngleDeg)
		pAnim->SetBoneRotation("Rot2J", -fAngleDeg,
			XMVectorSet(1.f, 0.f, 0.f, 0.f));

	__super::Update(fTimeDelta);
}

void CGigatzo::Fire_Bullet()
{
	if (nullptr == m_pBody) return;
	const _float4x4* pMuzzle = m_pBody->Get_BoneMatrixPtr("HaedJ");
	if (nullptr == pMuzzle) return;

	_matrix matMuzzle =
		XMLoadFloat4x4(pMuzzle) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	_vector vMuzzle = matMuzzle.r[3];
	_vector vDir = XMVector3Normalize(matMuzzle.r[1]);

	_float3 vP, vD;
	XMStoreFloat3(&vP, vMuzzle);
	XMStoreFloat3(&vD, vDir);

	CProjectile* pProj = nullptr;
	CProjectile_Manager::GetInstance()->Spawn(
		Get_PrototypeLevelIndex(), Get_LevelIndex(),
		L"GigatzoBullet", CGigatzoBullet::PROTOTYPE_TAG, &pProj);
	if (nullptr == pProj) return;

	static_cast<CGigatzoBullet*>(pProj)->Configure(m_fBulletSpeed, m_fBulletDamage, m_fBulletLife);
	pProj->Launch(vP, vD);
}

CGigatzo* CGigatzo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGigatzo* pInstance = new CGigatzo(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGigatzo");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGigatzo::Clone(void* pArg)
{
	CGigatzo* pInstance = new CGigatzo(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGigatzo");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGigatzo::Free()
{
	__super::Free();
}
