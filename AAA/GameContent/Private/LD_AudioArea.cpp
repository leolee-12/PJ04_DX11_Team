#include "LD_AudioArea.h"
#include "LevelDesign_Registry.h"

#include "GameInstance.h"
#include "Parsing_Utils.h"
#include "Geometry_Utils.h"

namespace
{
	enum class AUDIO_AREA_PLAY_TYPE
	{
		BGM_FADE,
		SFX_LOOP,
		NONE
	};

	struct AUDIO_AREA_SOUND_BINDING
	{
		const _tchar* pObjectName = nullptr;
		const _tchar* pVariationType = nullptr;
		_uint iSoundId = 0xffffffffu;
		const _tchar* pSoundKey = L"";
		AUDIO_AREA_PLAY_TYPE ePlayType = AUDIO_AREA_PLAY_TYPE::NONE;
		_float fVolume = 1.f;
	};

	constexpr _float kAudioFrameToSecond = 1.f / 60.f;
	constexpr _uint kAnySoundId = 0xffffffffu;

	const AUDIO_AREA_SOUND_BINDING g_AudioAreaSoundBindings[] =
	{
		{ L"AreaBgmRequestor", nullptr, 1u, L"", AUDIO_AREA_PLAY_TYPE::BGM_FADE, 1.f },
		{ L"AreaSeJungle", L"Jungle", kAnySoundId, L"", AUDIO_AREA_PLAY_TYPE::SFX_LOOP, 1.f },
		{ L"AreaSeSeaWave", L"SeaWave", kAnySoundId, L"", AUDIO_AREA_PLAY_TYPE::SFX_LOOP, 1.f },
	};

	_bool Matches_OptionalText(const _wstring& strValue, const _tchar* pExpected)
	{
		return nullptr == pExpected || L'\0' == pExpected[0] || JsonUtils::Equals_NoCase(strValue.c_str(), pExpected);
	}

	_bool Matches_OptionalSoundId(_uint iValue, _uint iExpected)
	{
		return kAnySoundId == iExpected || iValue == iExpected;
	}

	const AUDIO_AREA_SOUND_BINDING* Find_AudioAreaSoundBinding(const _wstring& strObjectName, const LD_AUDIO_AREA_DESC& Desc)
	{
		for (const AUDIO_AREA_SOUND_BINDING& Binding : g_AudioAreaSoundBindings)
		{
			if (nullptr == Binding.pObjectName || !JsonUtils::Equals_NoCase(strObjectName.c_str(), Binding.pObjectName))
				continue;

			if (!Matches_OptionalText(Desc.strVariationType, Binding.pVariationType))
				continue;

			if (!Matches_OptionalSoundId(Desc.iSoundId, Binding.iSoundId))
				continue;

			return &Binding;
		}

		return nullptr;
	}

	_float Frames_ToSeconds(_uint iFrame)
	{
		return static_cast<_float>(iFrame) * kAudioFrameToSecond;
	}
}

NS_BEGIN(Client)

CLD_AudioArea::CLD_AudioArea(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLD_AudioArea::CLD_AudioArea(const CLD_AudioArea& Prototype)
	: CLevelDesignObject(Prototype)
	, m_tAudioAreaDesc(Prototype.m_tAudioAreaDesc)
	, m_eMode(Prototype.m_eMode)
{
}

HRESULT CLD_AudioArea::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLD_AudioArea::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const LD_PARSED_OBJECT* pParsedDesc = static_cast<const LD_PARSED_OBJECT*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tAudioAreaDesc = pParsedDesc->AudioArea;
	m_eMode = Resolve_Mode();

	if (FAILED(Ready_TriggerCollider()))
		return E_FAIL;

	SetUp_Collider_Callback();

	if (FAILED(Validate_Initialized()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLD_AudioArea::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (m_tLevelDesignDesc.eCategory != LD_CATEGORY::AUDIO_AREA)
		return E_FAIL;

	const _wstring& strObjectName = m_tLevelDesignDesc.strObjectName;
	if (!JsonUtils::Equals_NoCase(strObjectName.c_str(), L"AreaBgmRequestor")
		&& !JsonUtils::Equals_NoCase(strObjectName.c_str(), L"AreaSeJungle")
		&& !JsonUtils::Equals_NoCase(strObjectName.c_str(), L"AreaSeSeaWave"))
	{
		return E_FAIL;
	}

	if (m_eMode == MODE::UNKNOWN)
		return E_FAIL;

	if (nullptr == m_pTriggerCollider)
		return E_FAIL;

	return S_OK;
}

void CLD_AudioArea::Late_Update(_float fTimeDelta)
{
	Update_TriggerCollider();

	if (m_eMode != MODE::AMBIENT_LOOP)
		return;

	if (m_bAmbientStopping)
	{
		Update_AmbientFadeOut(fTimeDelta);
		return;
	}

	if (m_bInside)
		Update_Ambient(fTimeDelta);
}

void CLD_AudioArea::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

HRESULT CLD_AudioArea::Ready_TriggerCollider()
{
	m_pTriggerCollider = Add_Component<CCollider>(
		L"Com_AudioAreaTrigger",
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));

	if (nullptr == m_pTriggerCollider)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };
	ColliderDesc.vSize = { 1.f, 1.f, 1.f };

	if (FAILED(m_pTriggerCollider->Initialize(&ColliderDesc)))
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pTriggerCollider, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLD_AudioArea::Update_TriggerCollider()
{
	const _float3 vAreaSize = GeometryUtils::Make_AbsSize(m_tAudioAreaDesc.vAreaSize);
	const _bool bValidArea = GeometryUtils::Has_UsableSize(vAreaSize);

	m_pTriggerCollider->Set_Enabled(bValidArea);

	if (!bValidArea)
		return;

	_matrix TriggerMatrix =
		XMMatrixScaling(vAreaSize.x, vAreaSize.y, vAreaSize.z) *
		XMMatrixTranslation(
			m_tAudioAreaDesc.vAreaCenter.x,
			m_tAudioAreaDesc.vAreaCenter.y,
			m_tAudioAreaDesc.vAreaCenter.z);

	if (m_tAudioAreaDesc.eCenterSpace == LD_AUDIO_AREA_CENTER_SPACE::LOCAL)
		TriggerMatrix = TriggerMatrix * XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());

	m_pTriggerCollider->Update(TriggerMatrix);

#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pTriggerCollider);
#endif
}

void CLD_AudioArea::SetUp_Collider_Callback()
{
	if (nullptr == m_pTriggerCollider)
		return;

	m_pTriggerCollider->Set_OnEnter([this](CCollider* pOther) { OnTriggerEnter(pOther); });
	m_pTriggerCollider->Set_OnStay([this](CCollider* pOther) { OnTriggerStay(pOther); });
	m_pTriggerCollider->Set_OnExit([this](CCollider* pOther) { OnTriggerExit(pOther); });
}

void CLD_AudioArea::OnTriggerEnter(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;

	m_bInside = true;
	m_bAmbientStopping = false;

	switch (m_eMode)
	{
	case MODE::BGM_REQUEST:
		Request_BGM();
		break;

	case MODE::AMBIENT_LOOP:
		Start_Ambient();
		break;

	default:
		break;
	}

#ifdef _DEBUG
	const _wstring strMessage = L"[CLD_AudioArea] Enter: " + Make_LevelDesignObjectKey() + L"\n";
	OutputDebugStringW(strMessage.c_str());
#endif
}

void CLD_AudioArea::OnTriggerStay(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;
}

void CLD_AudioArea::OnTriggerExit(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;

	m_bInside = false;

	switch (m_eMode)
	{
	case MODE::BGM_REQUEST:
		Release_BGM();
		break;

	case MODE::AMBIENT_LOOP:
		Stop_Ambient();
		break;

	default:
		break;
	}

#ifdef _DEBUG
	const _wstring strMessage = L"[CLD_AudioArea] Exit: " + Make_LevelDesignObjectKey() + L"\n";
	OutputDebugStringW(strMessage.c_str());
#endif
}

_bool CLD_AudioArea::Is_TriggerActivator(CCollider* pOther) const
{
	return nullptr != pOther
		&& ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup();
}

void CLD_AudioArea::Request_BGM()
{
	const AUDIO_AREA_SOUND_BINDING* pBinding = Find_AudioAreaSoundBinding(m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);
	if (nullptr == pBinding || pBinding->ePlayType != AUDIO_AREA_PLAY_TYPE::BGM_FADE)
		return;

	const _wstring strSoundKey = Resolve_SoundKey();
	if (strSoundKey.empty())
		return;

	m_pGameInstance_Proxy->Play_BGM_Fade(strSoundKey.c_str(), Get_FadeInSeconds(), pBinding->fVolume);
}

void CLD_AudioArea::Release_BGM()
{
	// 현재 SoundManager에는 BGM request stack/restore가 없으므로 Exit에서는 끊지 않는다.
}

void CLD_AudioArea::Start_Ambient()
{
	if (m_AmbientHandle.Is_Valid())
		return;

	const AUDIO_AREA_SOUND_BINDING* pBinding = Find_AudioAreaSoundBinding(m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);
	if (nullptr == pBinding || pBinding->ePlayType != AUDIO_AREA_PLAY_TYPE::SFX_LOOP)
		return;

	const _wstring strSoundKey = Resolve_SoundKey();
	if (strSoundKey.empty())
		return;

	m_AmbientHandle = m_pGameInstance_Proxy->Play_SFX_Loop(strSoundKey.c_str(), 0.f, ESoundBus::SFX);
	m_fCurrentVolume = 0.f;
	m_bAmbientStopping = false;
}

void CLD_AudioArea::Update_Ambient(_float fTimeDelta)
{
	if (!m_AmbientHandle.Is_Valid())
		return;

	const AUDIO_AREA_SOUND_BINDING* pBinding = Find_AudioAreaSoundBinding(m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);
	const _float fTargetVolume = nullptr != pBinding ? pBinding->fVolume : 1.f;
	const _float fFadeSec = max(Get_FadeInSeconds(), 0.001f);

	m_fCurrentVolume += fTimeDelta / fFadeSec * fTargetVolume;
	if (m_fCurrentVolume > fTargetVolume)
		m_fCurrentVolume = fTargetVolume;

	m_AmbientHandle.Set_Volume(m_fCurrentVolume);
}

void CLD_AudioArea::Stop_Ambient()
{
	if (!m_AmbientHandle.Is_Valid())
		return;

	m_bAmbientStopping = true;
}

void CLD_AudioArea::Update_AmbientFadeOut(_float fTimeDelta)
{
	if (!m_AmbientHandle.Is_Valid())
	{
		m_bAmbientStopping = false;
		return;
	}

	const _float fFadeSec = max(Get_InactivateSeconds(), 0.001f);

	m_fCurrentVolume -= fTimeDelta / fFadeSec;
	if (m_fCurrentVolume > 0.f)
	{
		m_AmbientHandle.Set_Volume(m_fCurrentVolume);
		return;
	}

	m_fCurrentVolume = 0.f;
	m_AmbientHandle.Stop();
	m_AmbientHandle = {};
	m_bAmbientStopping = false;
}

_wstring CLD_AudioArea::Resolve_SoundKey() const
{
	const AUDIO_AREA_SOUND_BINDING* pBinding = Find_AudioAreaSoundBinding(m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);
	if (nullptr == pBinding || nullptr == pBinding->pSoundKey || L'\0' == pBinding->pSoundKey[0])
	{
#ifdef _DEBUG
		const _wstring strMessage = L"[CLD_AudioArea] SoundKey unresolved: " + Make_LevelDesignObjectKey() + L"\n";
		OutputDebugStringW(strMessage.c_str());
#endif
		return {};
	}

	return pBinding->pSoundKey;
}

_float CLD_AudioArea::Get_FadeInSeconds() const
{
	return Frames_ToSeconds(m_tAudioAreaDesc.iFadeInFrame);
}

_float CLD_AudioArea::Get_InactivateSeconds() const
{
	return Frames_ToSeconds(m_tAudioAreaDesc.iInactivateFrame);
}

CLD_AudioArea::MODE CLD_AudioArea::Resolve_Mode() const
{
	const _wstring& strObjectName = m_tLevelDesignDesc.strObjectName;

	if (JsonUtils::Equals_NoCase(strObjectName.c_str(), L"AreaBgmRequestor"))
		return MODE::BGM_REQUEST;

	if (JsonUtils::Equals_NoCase(strObjectName.c_str(), L"AreaSeJungle")
		|| JsonUtils::Equals_NoCase(strObjectName.c_str(), L"AreaSeSeaWave"))
	{
		return MODE::AMBIENT_LOOP;
	}

	return MODE::UNKNOWN;
}

void CLD_AudioArea::Register_LevelDesignSpecs()
{
	const _wstring ObjectNames[] =
	{
			L"AreaBgmRequestor",
			L"AreaSeJungle",
			L"AreaSeSeaWave",
	};

	for (const _wstring& strObjectName : ObjectNames)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = strObjectName;
		Spec.strPrototypeTag = PROTOTYPE_TAG;
		Spec.strLayerTag = LAYER_TAG;
		Spec.eCategory = LD_CATEGORY::AUDIO_AREA;
		Spec.pPrototypeFactory = &Create_Prototype;
		Spec.pBuildDesc = nullptr;

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

CGameObject* CLD_AudioArea::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_AudioArea::Create(pDevice, pContext);
}

CLD_AudioArea* CLD_AudioArea::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_AudioArea* pInstance = new CLD_AudioArea(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_AudioArea");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_AudioArea::Clone(void* pArg)
{
	CLD_AudioArea* pInstance = new CLD_AudioArea(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_AudioArea");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLD_AudioArea::Free()
{
	if (m_AmbientHandle.Is_Valid())
		m_AmbientHandle.Stop();

	__super::Free();
}

NS_END