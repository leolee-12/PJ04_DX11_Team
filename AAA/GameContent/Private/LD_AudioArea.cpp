#include "LD_AudioArea.h"
#include "LevelDesign_Registry.h"
#include "GameContrnt_Events.h"

#include "GameInstance.h"
#include "Parsing_Utils.h"
#include "Geometry_Utils.h"

namespace
{
	enum class AUDIO_AREA_PLAY_TYPE
	{
		BGM_FADE,
		SFX_LOOP,
		AMBIENT,
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
		_float fFalloffRange = 10.f;
	};

	constexpr _float kAudioFrameToSecond = 1.f / 60.f;
	constexpr _uint kAnySoundId = 0xffffffffu;

	const AUDIO_AREA_SOUND_BINDING g_AudioAreaSoundBindings[] =
	{
		{ L"AreaBgmRequestor",		nullptr,				1u,					L"K15_Grassland1.marker.wav",			AUDIO_AREA_PLAY_TYPE::BGM_FADE,		0.25f, 0.f },
		{ L"AreaSeJungle",			L"Jungle",				kAnySoundId,		L"EnvJungle_Jungle1.wav",				AUDIO_AREA_PLAY_TYPE::AMBIENT,		0.12f, 50.f },
		{ L"AreaSeSeaWave",			L"SeaWave",				kAnySoundId,		L"EnvWaterWave_SeaWave1.wav",			AUDIO_AREA_PLAY_TYPE::AMBIENT,		0.18f, 40.f },
		{ L"AreaSeWaterPipe",		L"WaterPipe",			kAnySoundId,		L"GimmickWaterPipe_FallingWater.wav",	AUDIO_AREA_PLAY_TYPE::AMBIENT,		0.35f, 8.f },
		{ L"AreaSeLavaWaterFall",	L"LavaWaterFall",		kAnySoundId,		L"EnvLavaWaterFall_Lava1.wav",			AUDIO_AREA_PLAY_TYPE::AMBIENT,		0.35f, 20.f },
		{ L"AreaSeSandWaterFall",	L"SandWaterFall",		kAnySoundId,		L"EnvSandWaterFall_SandFall1.wav",		AUDIO_AREA_PLAY_TYPE::AMBIENT,		0.35f, 10.f },
		{ L"AreaSeWorldMap",		L"WorldMap",			kAnySoundId,		L"EnvWorldMap_World1Wind.wav",			AUDIO_AREA_PLAY_TYPE::AMBIENT,		0.35f, 10.f },
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

	void Build_DefaultAudioAreaDesc(LD_PARSED_OBJECT* pOutDesc)
	{
		if (nullptr == pOutDesc)
			return;

		*pOutDesc = {};

		pOutDesc->wstrSourcePath = L"Palette";
		pOutDesc->strSourceFile = L"AudioArea";
		pOutDesc->strSection = L"Palette";
		pOutDesc->strEntryKey = L"AudioArea_Default";
		pOutDesc->strObjectName = L"AreaBgmRequestor";
		pOutDesc->strKind = L"Palette";

		pOutDesc->eCategory = LD_CATEGORY::AUDIO_AREA;

		pOutDesc->fScale = 1.f;
		pOutDesc->vRight = { 1.f, 0.f, 0.f, 0.f };
		pOutDesc->vUp = { 0.f, 1.f, 0.f, 0.f };
		pOutDesc->vLook = { 0.f, 0.f, 1.f, 0.f };
		pOutDesc->vPosition = { 0.f, 0.f, 0.f, 1.f };

		pOutDesc->vParsedPosition = { 0.f, 0.f, 0.f };
		pOutDesc->qParsedRotation = { 0.f, 0.f, 0.f, 1.f };
		pOutDesc->vParsedScale = { 1.f, 1.f, 1.f };

		pOutDesc->AudioArea.iSoundId = 1u;
		pOutDesc->AudioArea.strShapeType = L"Rectangular";
		pOutDesc->AudioArea.vAreaSize = { 1.f, 1.f, 1.f };
		pOutDesc->AudioArea.iFadeInFrame = 60u;
		pOutDesc->AudioArea.iInactivateFrame = 60u;
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
	LD_PARSED_OBJECT DefaultDesc{};
	if (nullptr == pArg)
	{
		Build_DefaultAudioAreaDesc(&DefaultDesc);
		pArg = &DefaultDesc;
	}

	const LD_PARSED_OBJECT* pParsedDesc = static_cast<const LD_PARSED_OBJECT*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_tAudioAreaDesc = pParsedDesc->AudioArea;
	m_eMode = Resolve_Mode();

	if (FAILED(Ready_Trigger()))
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

	const _wstring& strObjectName =	m_tLevelDesignDesc.strObjectName;

	const _bool bAudioAreaName = JsonUtils::Equals_NoCase(strObjectName.c_str(), L"AreaBgmRequestor")
															|| 0 == _wcsnicmp(strObjectName.c_str(), L"AreaSe", 6);

	if (!bAudioAreaName)
		return E_FAIL;

	return S_OK;
}

void CLD_AudioArea::Late_Update(_float fTimeDelta)
{
	Update_Trigger();

	if (m_eMode == MODE::BGM_REQUEST)
	{
		if (m_bBgmStopping)
			Update_BgmFadeOut(fTimeDelta);
		return;
	}

	if (m_eMode != MODE::AMBIENT_LOOP)
		return;

	Update_Proximity();
}

void CLD_AudioArea::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

HRESULT CLD_AudioArea::Ready_Trigger()
{
	m_pTrigger = Add_Component<CCollider>(
		L"Com_Trigger",
		CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));

	if (nullptr == m_pTrigger)
		return E_FAIL;

	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.pOwner = this;
	ColliderDesc.vCenter = { 0.f, 0.f, 0.f };
	ColliderDesc.vSize = { 1.f, 1.f, 1.f };

	if (FAILED(m_pTrigger->Initialize(&ColliderDesc)))
		return E_FAIL;

	m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

	return S_OK;
}

void CLD_AudioArea::Update_Trigger()
{
	const _float3 vAreaSize = GeometryUtils::Make_AbsSize(m_tAudioAreaDesc.vAreaSize);
	const _bool bValidArea = GeometryUtils::Has_UsableSize(vAreaSize);

	m_pTrigger->Set_Enabled(bValidArea);

	if (!bValidArea)
		return;

	const _matrix ScaleMatrix =	XMMatrixScaling(vAreaSize.x, vAreaSize.y, vAreaSize.z);

	_matrix TriggerWorldMatrix;
	if (m_tAudioAreaDesc.bHasAreaCenter)
	{
		const _float3& vCenter = m_tAudioAreaDesc.vAreaCenter;
		TriggerWorldMatrix = ScaleMatrix * XMMatrixTranslation(vCenter.x, vCenter.y, vCenter.z);
	}
	else
	{
		TriggerWorldMatrix = ScaleMatrix * XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	}

	m_pTrigger->Update(TriggerWorldMatrix);

#ifdef _DEBUG
	m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
}

void CLD_AudioArea::SetUp_Collider_Callback()
{
	if (nullptr == m_pTrigger)
		return;

	m_pTrigger->Set_OnEnter([this](CCollider* pOther) { Handle_TriggerEnter(pOther); });
	m_pTrigger->Set_OnStay([this](CCollider* pOther) { Handle_TriggerStay(pOther); });
	m_pTrigger->Set_OnExit([this](CCollider* pOther) { Handle_TriggerExit(pOther); });
}

void CLD_AudioArea::Handle_TriggerEnter(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;

	if (m_eMode == MODE::BGM_REQUEST)
		Request_BGM();

#ifdef _DEBUG
	const _wstring strMessage =	L"[CLD_AudioArea] Enter: " + Make_LevelDesignObjectKey() + L"\n";
	OutputDebugStringW(strMessage.c_str());
#endif
}

void CLD_AudioArea::Handle_TriggerStay(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;
}

void CLD_AudioArea::Handle_TriggerExit(CCollider* pOther)
{
	if (!Is_TriggerActivator(pOther))
		return;

	if (m_eMode == MODE::BGM_REQUEST)
		Release_BGM();

#ifdef _DEBUG
	const _wstring strMessage =	L"[CLD_AudioArea] Exit: " + Make_LevelDesignObjectKey() + L"\n";
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
	const AUDIO_AREA_SOUND_BINDING* pBinding =	Find_AudioAreaSoundBinding(
			m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);
	if (nullptr == pBinding	|| pBinding->ePlayType != AUDIO_AREA_PLAY_TYPE::BGM_FADE)
		return;

	m_bBgmStopping = false;

	if (m_BgmHandle.Is_Valid())
	{
		m_pGameInstance_Proxy->Resume_BGM_Fade(m_BgmHandle, Get_FadeInSeconds());

		if (m_BgmHandle.Is_Valid())
		{
			m_BgmHandle.Set_Volume(pBinding->fVolume);
			m_bBgmPaused = false;
			return;
		}
	}

	const _wstring strSoundKey = Resolve_SoundKey();
	if (strSoundKey.empty())
		return;

	m_pGameInstance_Proxy->Play_BGM_Fade(strSoundKey.c_str(), Get_FadeInSeconds(), pBinding->fVolume, &m_BgmHandle);
	m_bBgmPaused = false;
}

void CLD_AudioArea::Release_BGM()
{
	if (!m_BgmHandle.Is_Valid())
		return;

	const AUDIO_AREA_SOUND_BINDING* pBinding =	Find_AudioAreaSoundBinding(m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);

	m_fCurrentVolume =
		nullptr != pBinding ? pBinding->fVolume : 1.f;
	m_bBgmStopping = true;
}

void CLD_AudioArea::Update_Proximity()
{
	const AUDIO_AREA_SOUND_BINDING* pBinding =
		Find_AudioAreaSoundBinding(
			m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);
	if (nullptr == pBinding
		|| pBinding->ePlayType != AUDIO_AREA_PLAY_TYPE::AMBIENT)
		return;

	if (!m_AmbientHandle.Is_Valid())
	{
		const _wstring strSoundKey = Resolve_SoundKey();
		if (strSoundKey.empty())
			return;

		m_AmbientHandle = m_pGameInstance_Proxy->Play_SFX_Loop(
			strSoundKey.c_str(), 0.f, ESoundBus::AMBIENT);
		if (!m_AmbientHandle.Is_Valid())
			return;
	}

	if (nullptr == m_pPlayer)
	{
		PLAYER_QUERY PlayerQuery{};
		m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &PlayerQuery);
		m_pPlayer = PlayerQuery.pPlayer;
		if (nullptr == m_pPlayer)
			return;
	}

	_float4 vPlayerPos{};
	XMStoreFloat4(&vPlayerPos, m_pPlayer->Get_Transform()->Get_State(STATE::POSITION));

	_float3 vAreaCenter{};
	if (m_tAudioAreaDesc.bHasAreaCenter)
		vAreaCenter = m_tAudioAreaDesc.vAreaCenter;
	else
	{
		_float4 vAnchorPos{};
		XMStoreFloat4(&vAnchorPos, m_pTransformCom->Get_State(STATE::POSITION));
		vAreaCenter = { vAnchorPos.x, vAnchorPos.y, vAnchorPos.z };
	}

	const _float3 vAreaSize = GeometryUtils::Make_AbsSize(m_tAudioAreaDesc.vAreaSize);
	const _float3 vHalfSize = {vAreaSize.x * 0.5f, vAreaSize.y * 0.5f, vAreaSize.z * 0.5f };

	// per-axis distance from player to box surface (inside = 0)
	_float3 vSurfaceDist{};
	vSurfaceDist.x = max(fabsf(vPlayerPos.x - vAreaCenter.x) - vHalfSize.x, 0.f);
	vSurfaceDist.y = max(fabsf(vPlayerPos.y - vAreaCenter.y) - vHalfSize.y, 0.f);
	vSurfaceDist.z = max(fabsf(vPlayerPos.z - vAreaCenter.z) - vHalfSize.z, 0.f);

	const _float fDistToArea = sqrtf(vSurfaceDist.x * vSurfaceDist.x
									+ vSurfaceDist.y * vSurfaceDist.y
									+ vSurfaceDist.z * vSurfaceDist.z);

	const _float fFalloffRange = max(pBinding->fFalloffRange, 0.001f);
	const _float fAttenRatio = max(0.f, 1.f - fDistToArea / fFalloffRange);

	m_AmbientHandle.Set_Volume(pBinding->fVolume * fAttenRatio);
}

void CLD_AudioArea::Update_BgmFadeOut(_float fTimeDelta)
{
	if (!m_BgmHandle.Is_Valid())
	{
		m_bBgmStopping = false;
		return;
	}

	const AUDIO_AREA_SOUND_BINDING* pBinding =	Find_AudioAreaSoundBinding(m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);
	const _float fStartVolume =	nullptr != pBinding ? pBinding->fVolume : 1.f;

	const _float fFadeSec =	max(Get_InactivateSeconds(), 0.001f);

	m_fCurrentVolume -=	fStartVolume * fTimeDelta / fFadeSec;
	if (m_fCurrentVolume > 0.f)
	{
		m_BgmHandle.Set_Volume(m_fCurrentVolume);
		return;
	}

	m_fCurrentVolume = 0.f;
	m_BgmHandle.Set_Volume(0.f);
	m_BgmHandle.Set_Paused(true);
	m_bBgmPaused = true;
	m_bBgmStopping = false;
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
	const AUDIO_AREA_SOUND_BINDING* pBinding =	Find_AudioAreaSoundBinding(m_tLevelDesignDesc.strObjectName, m_tAudioAreaDesc);

	if (nullptr != pBinding)
	{
		switch (pBinding->ePlayType)
		{
		case AUDIO_AREA_PLAY_TYPE::BGM_FADE:
			return MODE::BGM_REQUEST;
		case AUDIO_AREA_PLAY_TYPE::SFX_LOOP:
			return MODE::AMBIENT_LOOP;
		default:
			break;
		}
	}

	const _wstring& strObjectName =	m_tLevelDesignDesc.strObjectName;

	if (JsonUtils::Equals_NoCase(strObjectName.c_str(),	L"AreaBgmRequestor"))
		return MODE::BGM_REQUEST;

	if (0 == _wcsnicmp(strObjectName.c_str(), L"AreaSe", 6))
		return MODE::AMBIENT_LOOP;

	return MODE::UNKNOWN;
}

void CLD_AudioArea::Register_LevelDesignSpecs()
{
	const _wstring ObjectNames[] =
	{
		L"AreaBgmRequestor",
		L"AreaSeJungle",
		L"AreaSeSeaWave",
		L"AreaSeWaterPipe",
		L"AreaSeLavaWaterFall",
		L"AreaSeSandWaterFall",
		L"AreaSeWorldMap",
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

	if (m_bBgmPaused && m_BgmHandle.Is_Valid())
		m_BgmHandle.Stop();

	__super::Free();
}

NS_END