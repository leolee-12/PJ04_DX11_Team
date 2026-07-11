#include "LD_GarageRadio.h"
#include "LevelDesign_Registry.h"
#include "GameContent_const.h"

#include "Parsing_Utils.h"

namespace
{
	// AnimEvent 추가 후
	// 1. Validated_Initailzed에서 방어 코드 제거
	// 2. Clone에서 AnimEvent json 경로 추가

	inline constexpr const _tchar* TEMP_EVENT_TAG = L"Bridge.CutSceneStart";

	inline constexpr const _char* GARAGE_RADIO_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/GarageRadio/GarageRadio.ysh";
	inline constexpr const _tchar* GARAGE_RADIO_ANIM_EVENT_FILE = L"../../Resources/Map/Gimmick/Anim/GarageRadio/GarageRadio_Animevents.json";

	inline constexpr const _char* ANIM_CUT1 = "Cut1";
	inline constexpr const _char* ANIM_WAIT = "Wait";
	inline constexpr const _char* GARAGE_RADIO_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { ANIM_CUT1, ANIM_WAIT };
	inline constexpr _float GARAGE_RADIO_ANIM_SPEED = 1.5f;
}

NS_BEGIN(Client)

CLD_GarageRadio::CLD_GarageRadio(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_GarageRadio::CLD_GarageRadio(const CLD_GarageRadio& Prototype)
	: CLD_EventObject(Prototype)
	, m_eState(Prototype.m_eState)
{
}

HRESULT CLD_GarageRadio::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	const LD_ANIM_PLAY_DESC AnimDescs[] =
	{
		{ ANIM_CUT1, false, GARAGE_RADIO_ANIM_SPEED },
		{ ANIM_WAIT, false, GARAGE_RADIO_ANIM_SPEED },
	};

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, static_cast<_uint>(_countof(AnimDescs)))))
		return E_FAIL;

	Set_AnimPose(ANIM_WAIT, 0.f);

	return S_OK;
}

HRESULT CLD_GarageRadio::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tEventObjectDesc.strObjectName.c_str()))
		return E_FAIL;

	if (m_tEventObjectDesc.eModelType != MODEL::ANIM || m_tEventObjectDesc.wstrModelProtoTag != MODEL_PROTO_TAG)
		return E_FAIL;

	if (m_tEventObjectDesc.bUseCollMesh || nullptr != m_pRigidStatic)
		return E_FAIL;

	if (m_tEventObjectDesc.strAnimEventFile.empty())
		return E_FAIL;

	if (m_tEventObjectDesc.strAnimNames[0] != GARAGE_RADIO_ANIM_NAMES[0])
		return E_FAIL;

	if (m_tEventObjectDesc.strAnimNames[1] != GARAGE_RADIO_ANIM_NAMES[1])
		return E_FAIL;

	return S_OK;
}

void CLD_GarageRadio::Update(_float fTimeDelta)
{
	const _bool bAnimationWasActive = m_bAnimationActive;

	__super::Update(fTimeDelta);

	if (bAnimationWasActive && !m_bAnimationActive && STATE::PLAYING == m_eState)
		m_eState = STATE::IDLE;
}

void CLD_GarageRadio::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_GarageRadio::Register_LevelDesignSpecs()
{
	LD_SPAWN_SPEC Spec{};
	Spec.strObjectName = OBJECT_NAME;
	Spec.strPrototypeTag = PROTOTYPE_TAG;
	Spec.strLayerTag = LAYER_TAG;
	Spec.eCategory = LD_CATEGORY::GIMMICK;
	Spec.wstrModelProtoTag = MODEL_PROTO_TAG;
	Spec.eModelType = MODEL::ANIM;
	Spec.pPrototypeFactory = &Create_Prototype;
	Spec.pBuildDesc = &Build_Desc;
	Spec.ModelRequirements =
	{
		{ MODEL_PROTO_TAG, GARAGE_RADIO_MODEL_PATH, MODEL::ANIM, false },
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_GarageRadio::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec,
	LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.eModelType != MODEL::ANIM || Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = false;
	Desc.strAnimEventFile = GARAGE_RADIO_ANIM_EVENT_FILE;

	Desc.strAnimNames[0] = GARAGE_RADIO_ANIM_NAMES[0];
	Desc.strAnimNames[1] = GARAGE_RADIO_ANIM_NAMES[1];

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_GarageRadio::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_GarageRadio::Create(pDevice, pContext);
}

HRESULT CLD_GarageRadio::Ready_Events()
{
	Subscribe_Event(TEMP_EVENT_TAG, [this](void* pData)
		{
			_float4x4* pMat = { nullptr };
			if (pMat = static_cast<_float4x4*>(pData))
			{
				_matrix fixMat = XMMatrixRotationY(XMConvertToRadians(180)) * XMLoadFloat4x4(pMat);
				m_pTransformCom->Set_WorldMatrix(fixMat);
				On_Event();
			}
		});

	return S_OK;
}

void CLD_GarageRadio::On_AnimEvent(const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase)
{
	if (ANIM_EVENT_PHASE::POINT != ePhase)
		return;

	switch (static_cast<EANIM_EVENT>(AnimEvent.iEventType))
	{
	case EANIM_EVENT::Sound:
	{
		if (AnimEvent.strParam.empty())
		{
#ifdef _DEBUG
			MSG_BOX("Sound StrParam is Empty");
#endif
			return;
		}

		if (ePhase == ANIM_EVENT_PHASE::POINT)
		{
			if (AnimEvent.iIntParam == 1)
				m_pGameInstance_Proxy->Play_BGM_Fade(StrToWstr(AnimEvent.strParam).c_str(), AnimEvent.vOffset.x);
			else 
				m_pGameInstance_Proxy->Play_SFX(StrToWstr(AnimEvent.strParam).c_str(), AnimEvent.vOffset.x);
		}
		break;
	}
	default:
		break;

	}
}

void CLD_GarageRadio::On_Event()
{
	if (STATE::PLAYING == m_eState)
		return;

	Play_Anim(ANIM_CUT1);
	m_eState = STATE::PLAYING;
}

CLD_GarageRadio* CLD_GarageRadio::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_GarageRadio* pInstance = new CLD_GarageRadio(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_GarageRadio");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_GarageRadio::Clone(void* pArg)
{
	CLD_GarageRadio* pInstance = new CLD_GarageRadio(*this);
	
	LD_EVENTOBJECT_DESC TempDesc{};
	if (nullptr == pArg)
	{
		TempDesc.strObjectName = OBJECT_NAME;
		TempDesc.strKind = OBJECT_NAME;
		TempDesc.eCategory = LD_CATEGORY::GIMMICK;
		TempDesc.iModelProtoLevel = m_iPrototypeLevel;
		TempDesc.eModelType = MODEL::ANIM;
		TempDesc.wstrModelProtoTag = MODEL_PROTO_TAG;
		TempDesc.bUseCollMesh = false;
		TempDesc.strAnimEventFile.clear();

		TempDesc.strAnimNames[0] = GARAGE_RADIO_ANIM_NAMES[0];
		TempDesc.strAnimNames[1] = GARAGE_RADIO_ANIM_NAMES[1];

		pArg = &TempDesc;
	}

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_GarageRadio");
		Safe_Release(pInstance);
	}

	return pInstance;
}

NS_END