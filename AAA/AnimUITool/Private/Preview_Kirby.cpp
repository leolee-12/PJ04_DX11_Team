#include "Preview_Kirby.h"
#include "GameInstance_proxy.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"

CPreview_Kirby::CPreview_Kirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
	, m_eBody{ KIRBY_BODY_STATE::NORMAL }
	, m_eMouth{ KIRBY_MOUTH_STATE::IDLE }
	, m_eEye{ KIRBY_EYE_STATE::IDLE }
{
}

CPreview_Kirby::CPreview_Kirby(const CPreview_Kirby& Prototype)
	: CGameObject( Prototype )
	, m_eBody{ Prototype.m_eBody }
	, m_eMouth{ Prototype.m_eMouth }
	, m_eEye{ Prototype.m_eEye }
{
}

HRESULT CPreview_Kirby::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;

	return S_OK;
}

HRESULT CPreview_Kirby::Initialize(void* pArg)
{
	if (pArg)
		m_Desc = *static_cast<PREVIEW_KIRBY_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pAnimatorCom->Play("Wait", true, true, 0.2f, 1.0f);
	
	return S_OK;
}

void CPreview_Kirby::Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	m_pAnimatorCom->Update(fTimeDelta);

}

void CPreview_Kirby::Late_Update(_float fTimeDelta)
{
	if (!m_bActive)
		return;

	m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CPreview_Kirby::Render()
{
	Resolve_VisibleMeshes();    // 현재 상태 → m_MeshVisible

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const _uint iNum = (_uint)m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNum; ++i)
	{
		if (i >= m_MeshVisible.size() || !m_MeshVisible[i])
			continue;

		// Eye 텍스처 바인딩을 Material 대신에 배열에서 바인딩
		if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_EyeTexture", ETOUI(m_eEye))))
			return E_FAIL;
		if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_EyeMaskTexture", ETOUI(m_eEye))))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_SkinTexture", i, MTEX_TYPE::UNKNOWN, 1)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MouthTexture", i, MTEX_TYPE::UNKNOWN, 2)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vBodyColor", &m_vBodyColor, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vFootColor", &m_vFootColor, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vBlushColor", &m_vBlushColor, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(0)))     // pass 0 = Body
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

void CPreview_Kirby::Resolve_VisibleMeshes()
{
	const _uint n = (_uint)m_pModelCom->Get_NumMeshes();
	m_MeshVisible.assign(n, false);
	auto On = [&](_int i)
		{
			if (i >= 0 && (_uint)i < n)
				m_MeshVisible[i] = true;
		};

	// 바디 메쉬 = BODY_STATE
	switch (m_eBody)
	{
	case Client::KIRBY_BODY_STATE::NORMAL:
		On(KMESH_BODY);
		break;
	case Client::KIRBY_BODY_STATE::STUFFED:
		On(KMESH_BODY_BIG);
		break;
	case Client::KIRBY_BODY_STATE::INHALE:
		On(KMESH_BODY_VACUUM);
		break;
	default:
		break;
	}

	// 입 메쉬 = 바디가 NORMAL일 때만 MOUTH_STATE
	if (m_eBody == Client::KIRBY_BODY_STATE::NORMAL)
	{
		switch (m_eMouth)
		{
		case Client::KIRBY_MOUTH_STATE::IDLE:
			On(KMESH_MOUTH_NORMAL);
			break;
		case Client::KIRBY_MOUTH_STATE::OPEN:
			On(KMESH_MOUTH_OPEN);
			break;
		case Client::KIRBY_MOUTH_STATE::ANGRY:
			On(KMESH_MOUTH_ANGRY);
			break;
		case Client::KIRBY_MOUTH_STATE::SMILE_OPEN:
			On(KMESH_MOUTH_SMILE_OP);
			break;
		case Client::KIRBY_MOUTH_STATE::SMILE_CLOSE:
			On(KMESH_MOUTH_SMILE_CL);
			break;
		default:
			break;
		}
	}

	//  limbs 항상 ON
	On(KMESH_LIMBS);

	// 눈 = 텍스처 (메쉬 무관)
}

HRESULT CPreview_Kirby::Ready_Components()
{
	m_pShaderCom = Add_Component<CShader>(m_Desc.iProtoLevel, m_Desc.szShaderTag, TEXT("Com_Shader"));
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom = Add_Component<CModel>(m_Desc.iProtoLevel, m_Desc.szModelTag, TEXT("Com_Model"));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	if (FAILED(Ready_EyeTextures()))
		return E_FAIL;

	m_MeshVisible.assign(m_pModelCom->Get_NumMeshes(), true);


	CAnimator::ANIMATOR_DESC AnimDesc{};
	AnimDesc.pModel = m_pModelCom;
	AnimDesc.strDataFile = m_Desc.strAnimEvents;

	m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));
	if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
		return E_FAIL;

	// 이벤트 콜백 배선 구현 예정
	m_pAnimatorCom->Set_EventCallback(
		[this](const ANIM_EVENT& e, ANIM_EVENT_PHASE phase)
		{
			if (phase != ANIM_EVENT_PHASE::POINT)
				return;

			switch (static_cast<EANIM_EVENT>(e.iEventType))
			{
			case EANIM_EVENT::SetBody:
				Set_Body((KIRBY_BODY_STATE)e.iIntParam);
				break;
			case EANIM_EVENT::SetMouth:
				Set_Mouth((KIRBY_MOUTH_STATE)e.iIntParam);
				break;
			case EANIM_EVENT::SetEye:
				Set_Eye((KIRBY_EYE_STATE)e.iIntParam);
				break;
			default:
				break;
			}

		});


	return S_OK;
}

HRESULT CPreview_Kirby::Bind_ShaderResources()
{
	// 단독 객체 → 자기 Transform 월드행렬 (Kirby_Body는 CombinedWorldMatrix를 쓰는 부분만 다름)
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPreview_Kirby::Ready_EyeTextures()
{
	m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
		CTexture::Create(m_pDevice, m_pContext,
			L"../../Resources/CHJ/AnimModel/Kirby/KirbyEye.%02d.png", ETOUI(KIRBY_EYE_STATE::END)));
	if (nullptr == m_pEyeTextureCom)
		return E_FAIL;

	m_pEyeMaskTextureCom = Add_Component<CTexture>(TEXT("Com_EyeMaskTexture"),
		CTexture::Create(m_pDevice, m_pContext,
			L"../../Resources/CHJ/AnimModel/Kirby/KirbyEyeMask.%02d.png", ETOUI(KIRBY_EYE_STATE::END)));
	if (nullptr == m_pEyeMaskTextureCom)
		return E_FAIL;

	return S_OK;
}

CPreview_Kirby* CPreview_Kirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPreview_Kirby* pInstance = new CPreview_Kirby(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPreview_Kirby");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CPreview_Kirby::Clone(void* pArg)
{
	CPreview_Kirby* pInstance = new CPreview_Kirby(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPreview_Kirby");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPreview_Kirby::Free()
{
	__super::Free();
}