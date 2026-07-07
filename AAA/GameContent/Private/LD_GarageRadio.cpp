#include "LD_GarageRadio.h"
#include "LevelDesign_Registry.h"
#include "GameContrnt_Events.h"

#include "Parsing_Utils.h"

namespace
{
	inline constexpr const _tchar* TEMP_EVENT_TAG = L"Temp"; // TODO: 컷신 트리거 이벤트 태그 확정 후 정리

	inline constexpr const _char* GARAGE_RADIO_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/GarageRadio/GarageRadio.ysh";
	inline constexpr const _char* ANIM_PERFORMANCE = "PerformanceAnim";
	inline constexpr const _char* ANIM_SPEAKERBIG = "SpeakerBig";
	inline constexpr const _char* ANIM_WAIT = "Wait";
	inline constexpr const _char* ANIM_CUT1 = "Cut1";
	inline constexpr const _char* GARAGE_RADIO_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { ANIM_PERFORMANCE, ANIM_SPEAKERBIG, ANIM_WAIT, ANIM_CUT1 };
	inline constexpr _float GARAGE_RADIO_ANIM_SPEED = 1.f;
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
		{ ANIM_PERFORMANCE, false, GARAGE_RADIO_ANIM_SPEED },
		{ ANIM_SPEAKERBIG, false, GARAGE_RADIO_ANIM_SPEED },
		{ ANIM_WAIT, true, GARAGE_RADIO_ANIM_SPEED },
		{ ANIM_CUT1, false, GARAGE_RADIO_ANIM_SPEED },
	};

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, static_cast<_uint>(_countof(AnimDescs)))))
		return E_FAIL;

	return Set_AnimPose(ANIM_WAIT, 0.f);
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

	if (!m_tEventObjectDesc.strAnimEventFile.empty())
		return E_FAIL;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
	{
		if (m_tEventObjectDesc.strAnimNames[i] != GARAGE_RADIO_ANIM_NAMES[i])
			return E_FAIL;
	}

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
	Desc.strAnimEventFile.clear();

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = GARAGE_RADIO_ANIM_NAMES[i];

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_GarageRadio::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_GarageRadio::Create(pDevice, pContext);
}

HRESULT CLD_GarageRadio::Ready_Events()
{
	Subscribe_Event(TEMP_EVENT_TAG, [this](void*) { On_Event(); });

	return S_OK;
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

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_GarageRadio");
		Safe_Release(pInstance);
	}

	return pInstance;
}

NS_END