#include "UI_SpriteAnim.h"
#include "GameInstance_Proxy.h"
#include "GameContent_const.h"
#include "Shader.h"
#include "Texture.h"
#include "VIBuffer_Rect.h"

CUI_SpriteAnim::CUI_SpriteAnim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIPartObject{ pDevice, pContext }
	, m_vColor{ 1.f, 1.f, 1.f, 1.f }, m_fAlpha{ 1.f }
	, m_iTextureLevel{ ETOUI(LEVEL::STATIC) }
	, m_fDuration{ 1.f }, m_fEndDelay{ 0.f }, m_bLoop{ true }, m_bAutoPlay{ true }
	, m_fSpinSpeedDeg{ 0.f }, m_bSpinEase{ false }, m_fSpinEaseCycle{ 1.f }, m_fSpinEaseMin{ 0.f }
{
}

CUI_SpriteAnim::CUI_SpriteAnim(const CUI_SpriteAnim& Prototype)
	: CUIPartObject(Prototype)
	, m_vColor{ Prototype.m_vColor }, m_fAlpha{ Prototype.m_fAlpha }
	, m_iTextureLevel{ Prototype.m_iTextureLevel }
	, m_strTextureProtoTag{ Prototype.m_strTextureProtoTag }
	, m_fDuration{ Prototype.m_fDuration }, m_fEndDelay{ Prototype.m_fEndDelay }
	, m_bLoop{ Prototype.m_bLoop }, m_bAutoPlay{ Prototype.m_bAutoPlay }
	, m_fSpinSpeedDeg{ Prototype.m_fSpinSpeedDeg }
	, m_bSpinEase{ Prototype.m_bSpinEase }
	, m_fSpinEaseCycle{ Prototype.m_fSpinEaseCycle }
	, m_fSpinEaseMin{ Prototype.m_fSpinEaseMin }
{
}

HRESULT CUI_SpriteAnim::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CUI_SpriteAnim::Initialize(void* pArg)
{
	UI_SPRITEANIM_DESC Default{};
	UI_SPRITEANIM_DESC* pDesc = pArg ? static_cast<UI_SPRITEANIM_DESC*>(pArg) : &Default;

	if (pDesc->szTextureProtoTag)
	{
		m_iTextureLevel = static_cast<int>(pDesc->iTextureLevel);
		m_strTextureProtoTag = pDesc->szTextureProtoTag;
	}
	m_fDuration = pDesc->fDuration;
	m_fEndDelay = pDesc->fEndDelay;
	m_bLoop		= pDesc->bLoop;
	m_bAutoPlay = pDesc->bAutoPlay;
	m_fSpinSpeedDeg = pDesc->fSpinSpeedDeg;
	m_bSpinEase = pDesc->bSpinEase;
	m_fSpinEaseCycle = pDesc->fSpinEaseCycle;
	m_fSpinEaseMin = pDesc->fSpinEaseMin;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	m_eRenderLayer = static_cast<RENDERUIID>(pDesc->iRenderLayer);
	m_pTransformCom->Set_Scale(pDesc->vSize.x, pDesc->vSize.y, 1.f);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, 1.f, 1.f));

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	Sync_FrameCount();
	m_fAccTime = 0.f;
	m_iTexIndex = 0;
	m_bFinished = false;
	m_bIsPlay = (m_bAutoPlay && m_iFrameCount > 1);

	return S_OK;
}

void CUI_SpriteAnim::Priority_Update(_float fTimeDelta)
{

}

void CUI_SpriteAnim::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	Update_Spin(fTimeDelta);

	if (!m_bIsPlay || m_iFrameCount <= 1 || m_fDuration <= 0.f)
		return;

	m_fAccTime += fTimeDelta;

	if (m_fAccTime < m_fDuration)
	{
		m_iTexIndex = static_cast<_int>((m_fAccTime / m_fDuration) * static_cast<_float>(m_iFrameCount));
		if (m_iTexIndex >= m_iFrameCount)
			m_iTexIndex = m_iFrameCount - 1;
		return;
	}

	m_iTexIndex = m_iFrameCount - 1;
	if (!m_bLoop)
	{
		m_bIsPlay = false;
		m_bFinished = true;
		return;
	}

	// 재생이 끝나고 Delay 시간이 아직 안지난 경우
	if (m_fAccTime < m_fDuration + m_fEndDelay)
		return;

	m_fAccTime = 0.f;
	m_iTexIndex = 0;
}

void CUI_SpriteAnim::Late_Update(_float fTimeDelta)
{
	m_pGameInstance_Proxy->Add_RenderGroup_UI(m_eRenderLayer, this);
}

HRESULT CUI_SpriteAnim::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(Render_Pass())))		// pass 1 : SpriteAnim
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CUI_SpriteAnim::Deserialize_Internal(const json& j)
{
	__super::Deserialize_Internal(j);
	if (!m_strTextureProtoTag.empty() && !m_pTextureCom)
		m_pTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, TEXT("Com_Texture"));

	Sync_FrameCount();
	m_fAccTime = 0.f;
	m_iTexIndex = 0;
	m_bFinished = false;
	m_bIsPlay = (m_bAutoPlay && m_iFrameCount > 1);
}

void CUI_SpriteAnim::Play(_bool bLoop)
{
	m_bLoop = bLoop;
	m_fAccTime = 0.f;
	m_iTexIndex = 0;
	m_bFinished = false;
	m_bIsPlay = (m_iFrameCount > 1);
}

void CUI_SpriteAnim::Stop()
{
	m_bIsPlay = false;
	m_fAccTime = 0.f;
	m_iTexIndex = 0;
	m_bFinished = false;
	m_fSpinAngle = 0.f;
	m_fSpinPhase = 0.f;
	m_bPrevPlay = false;
	if (m_fSpinSpeedDeg != 0.f)
		Apply_Spin();
}

void CUI_SpriteAnim::Set_Frame(_int iFrame)
{
	if (iFrame < 0)
		iFrame = 0;
	if (m_iFrameCount > 0 && iFrame >= m_iFrameCount)
		iFrame = m_iFrameCount - 1;
	m_iTexIndex = iFrame;
	if (m_iFrameCount > 0)
		m_fAccTime = (static_cast<_float>(iFrame) / static_cast<_float>(m_iFrameCount) * m_fDuration);
	m_bFinished = false;
}

HRESULT CUI_SpriteAnim::Set_Texture(_int iLevel, const _wstring& strProtoTag)
{
	if (strProtoTag.empty())
		return E_FAIL;

	// try_emplace 덮어쓰기 안 되므로 기존 Com_Texture 먼저 제거
	auto iter = m_Components.find(TEXT("Com_Texture"));
	if (iter != m_Components.end())
	{
		Safe_Release(iter->second);
		m_Components.erase(iter);
		m_pTextureCom = nullptr;
	}

	m_iTextureLevel = iLevel;
	m_strTextureProtoTag = strProtoTag;

	m_pTextureCom = Add_Component<CTexture>(
		static_cast<_uint>(m_iTextureLevel),
		m_strTextureProtoTag,
		TEXT("Com_Texture"));

	if (!m_pTextureCom)
		return E_FAIL;

	// 새 배열의 슬라이스 수로 프레임 재동기 + 재생 리셋
	Sync_FrameCount();
	m_fAccTime = 0.f;
	m_iTexIndex = 0;
	m_bFinished = false;
	m_bIsPlay = (m_bAutoPlay && m_iFrameCount > 1);

	return S_OK;
}

void CUI_SpriteAnim::Seek(_float fRatio)
{
	if (fRatio < 0.f) fRatio = 0.f;
	if (fRatio > 1.f) fRatio = 1.f;

	m_fAccTime = fRatio * m_fDuration;

	_int iFrame = static_cast<_int>(
		fRatio * static_cast<_float>(m_iFrameCount));
	if (iFrame >= m_iFrameCount) iFrame = m_iFrameCount - 1;
	if (iFrame < 0) iFrame = 0;
	m_iTexIndex = iFrame;

	m_bFinished = false;
}

_float CUI_SpriteAnim::Get_Progress() const
{
	if (m_fDuration <= 0.f) return 0.f;
	_float fR = m_fAccTime / m_fDuration;
	if (fR < 0.f) fR = 0.f;
	if (fR > 1.f) fR = 1.f;
	return fR;
}

HRESULT CUI_SpriteAnim::Ready_Components(UI_SPRITEANIM_DESC* pDesc)
{
	m_pShaderCom = Add_Component<CShader>(Shader_UI.iLevelID, Shader_UI.szProtoTag, TEXT("Com_Shader"));
	if (!m_pShaderCom)
		return E_FAIL;

	m_pVIBufferCom = Add_Component<CVIBuffer_Rect>(VI_Rect.iLevelID, VI_Rect.szProtoTag, TEXT("Com_Model"));
	if (!m_pVIBufferCom)
		return E_FAIL;

	if (!m_strTextureProtoTag.empty())
	{
		m_pTextureCom = Add_Component<CTexture>(static_cast<_uint>(m_iTextureLevel), m_strTextureProtoTag, TEXT("Com_Texture"));
		if (!m_pTextureCom)
			return E_FAIL;
	}

	return S_OK;
}

HRESULT	CUI_SpriteAnim::Bind_ShaderResources()
{
	if (m_pParentMatrix)
	{
		Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
			return E_FAIL;
	}
	else if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrixPtr())))
		return E_FAIL;

	if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW, PROJ_TYPE::ORTHO)))
		return E_FAIL;
	if (FAILED(Bind_ShaderResource(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ, PROJ_TYPE::ORTHO)))
		return E_FAIL;

	if (m_pTextureCom)
	{
		_int iFrame = m_iTexIndex;
		if (iFrame < 0)
			iFrame = 0;
		if (m_iFrameCount > 0 && iFrame >= m_iFrameCount)
			iFrame = m_iFrameCount - 1;

		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_TextureArray", 0)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_iFrame", &iFrame, sizeof(iFrame))))
			return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(m_vColor))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
		return E_FAIL;

	return S_OK;
}


void CUI_SpriteAnim::Sync_FrameCount()
{
	m_iFrameCount = m_pTextureCom
		? (_int)m_pTextureCom->Get_ArraySize() : 1;
	if (m_iFrameCount < 1)
		m_iFrameCount = 1;
}

void CUI_SpriteAnim::Update_Spin(_float fTimeDelta)
{
	if (m_fSpinSpeedDeg == 0.f)        // 회전 옵션 꺼짐 → 트랜스폼 건드리지 않음
		return;

	if (m_bIsPlay && !m_bPrevPlay)     // 재생이 막 시작되면 각/위상 0으로
	{
		m_fSpinAngle = 0.f;
		m_fSpinPhase = 0.f;
	}
	m_bPrevPlay = m_bIsPlay;

	if (!m_bIsPlay)
		return;

	_float fSpeedScale = 1.f;
	if (m_bSpinEase && m_fSpinEaseCycle > 0.f)
	{
		m_fSpinPhase += fTimeDelta;
		if (m_fSpinPhase >= m_fSpinEaseCycle)
			m_fSpinPhase = fmodf(m_fSpinPhase, m_fSpinEaseCycle);

		// (1 - cos)/2 : 0 → 1 → 0 으로 부드럽게(느림→빠름→느림)
		const _float fWave = 0.5f * (1.f - cosf(XM_2PI * (m_fSpinPhase / m_fSpinEaseCycle)));

		// 최저 배율 ~ 1.0 사이로 매핑 (0이면 정점에서 잠깐 멈춤 느낌)
		const _float fMin = max(0.f, min(1.f, m_fSpinEaseMin));
		fSpeedScale = fMin + (1.f - fMin) * fWave;
	}

	m_fSpinAngle += XMConvertToRadians(m_fSpinSpeedDeg) * fSpeedScale * fTimeDelta;
	Apply_Spin();
}

void CUI_SpriteAnim::Apply_Spin()
{
	// 회전해도 basis 벡터 길이는 보존되므로 매 프레임 현재 스케일을 그대로 읽어 재구성
	_float3 vScale = m_pTransformCom->Get_Scaled();
	const _float c = cosf(m_fSpinAngle);
	const _float s = sinf(m_fSpinAngle);
	m_pTransformCom->Set_State(STATE::RIGHT, XMVectorSet(c * vScale.x, s * vScale.x, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::UP, XMVectorSet(-s * vScale.y, c * vScale.y, 0.f, 0.f));
	m_pTransformCom->Set_State(STATE::LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));
}

CUI_SpriteAnim* CUI_SpriteAnim::Create(
	ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_SpriteAnim* pInstance =
		new CUI_SpriteAnim(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_SpriteAnim");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_SpriteAnim::Clone(void* pArg)
{
	CUI_SpriteAnim* pInstance = new CUI_SpriteAnim(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_SpriteAnim");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_SpriteAnim::Free()
{
	__super::Free();
}
