#include "LD_Stage1BossDemo.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameContent_Events.h"
#include "GameContent_AnimEvents.h"

#include "Model.h"
#include "GameInstance.h"

namespace
{
	inline constexpr const _char* STAGE1_BOSS_DEMO_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/Level1BossDemoBg/Level1BossDemoBg.ysh";
	inline constexpr const _tchar* STAGE1_BOSS_DEMO_ANIM_EVENT_FILE = L"../../Resources/Map/Gimmick/Anim/Level1BossDemoBg/Level1BossDemoBg_AnimEvents.json";
	
	inline constexpr const _char* ANIM_APPEAR = "DemoAppear2";
	inline constexpr const _char* ANIM_AFTERWAIT = "DemoAppear2AfterWait";
	inline constexpr const _char* ANIM_BEFOREWAIT = "DemoAppear2BeforWait";
	
	inline constexpr const _char* STAGE1_BOSS_DEMO_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { ANIM_APPEAR, ANIM_AFTERWAIT, ANIM_BEFOREWAIT, "" };

	inline constexpr _uint EVENTOBJECT_ANIM_DEFAULT_PASS = 1u;
	inline constexpr _uint STAGE1_BOSS_DEMO_GLASS_PASS = 0u;
	inline constexpr _bool ENABLE_STAGE1_BOSS_DEMO_GLASS_PASS = false;
	inline constexpr _uint STAGE1_BOSS_DEMO_REQUIRED_MESH_COUNT = 49u;

	_bool Is_Stage1BossDemoGlassMesh(const _string& strMeshName)
	{
		return "GlassSideM__MlBossGlassC" == strMeshName || "GlassM__MlBossGlassC" == strMeshName;
	}
}

NS_BEGIN(Client)

CLD_Stage1BossDemo::CLD_Stage1BossDemo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_Stage1BossDemo::CLD_Stage1BossDemo(const CLD_Stage1BossDemo& Prototype)
	: CLD_EventObject(Prototype)
	, m_eDemoState(Prototype.m_eDemoState)
{
}

HRESULT CLD_Stage1BossDemo::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tEventObjectDesc.strObjectName.c_str()))
		return E_FAIL;

	if (m_tEventObjectDesc.eModelType != MODEL::ANIM || m_tEventObjectDesc.wstrModelProtoTag !=
		MODEL_PROTO_TAG)
		return E_FAIL;

	if (!m_tEventObjectDesc.bUseCollMesh || m_tEventObjectDesc.strAnimEventFile !=
		STAGE1_BOSS_DEMO_ANIM_EVENT_FILE)
		return E_FAIL;

	if (m_pModelCom->Get_NumMeshes() < STAGE1_BOSS_DEMO_REQUIRED_MESH_COUNT)
		return E_FAIL;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
	{
		if (m_tEventObjectDesc.strAnimNames[i] != STAGE1_BOSS_DEMO_ANIM_NAMES[i])
			return E_FAIL;
	}

	return S_OK;
}

void CLD_Stage1BossDemo::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_Stage1BossDemo::Register_LevelDesignSpecs()
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
			{ MODEL_PROTO_TAG, STAGE1_BOSS_DEMO_MODEL_PATH, MODEL::ANIM, true },
	};

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_Stage1BossDemo::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const
	LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	if (nullptr == pOutEntry)
		return false;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, CommonDesc.strObjectName.c_str()))
		return false;

	if (Spec.strPrototypeTag != PROTOTYPE_TAG || Spec.strLayerTag != LAYER_TAG)
		return false;

	if (Spec.eCategory != LD_CATEGORY::GIMMICK || Spec.eModelType != MODEL::ANIM ||
		Spec.wstrModelProtoTag != MODEL_PROTO_TAG)
		return false;

	LD_EVENTOBJECT_DESC Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;
	Desc.eModelType = Spec.eModelType;
	Desc.wstrModelProtoTag = Spec.wstrModelProtoTag;
	Desc.bUseCollMesh = true;
	Desc.strAnimEventFile = STAGE1_BOSS_DEMO_ANIM_EVENT_FILE;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = STAGE1_BOSS_DEMO_ANIM_NAMES[i];

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_Stage1BossDemo::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext*
	pContext)
{
	return CLD_Stage1BossDemo::Create(pDevice, pContext);
}

HRESULT CLD_Stage1BossDemo::Ready_Events()
{
	Subscribe_Event(EventTag::Cutscene_GorillaBreak,
		[this](void*) { On_Event(EventTag::Cutscene_GorillaBreak); });

	return S_OK;
}

HRESULT CLD_Stage1BossDemo::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Stage1BossDemo()))
		return E_FAIL;

	m_bUseShadow = true;

	return S_OK;
}

HRESULT CLD_Stage1BossDemo::Ready_Stage1BossDemo()
{
	m_eDemoState = DEMO_STATE::IDLE;

	const LD_ANIM_PLAY_DESC AnimDescs[] =
	{
			{ ANIM_APPEAR, false, 1.5f },
			{ ANIM_AFTERWAIT, false, 1.5f },
			{ ANIM_BEFOREWAIT, false, 1.5f },
	};

	if (FAILED(Ready_AnimPlayDescs(AnimDescs, static_cast<_uint>(_countof(AnimDescs)))))
		return E_FAIL;

	static constexpr _uint PieceIndices[] =
	{
			28u, 29u, 30u, 31u, 32u, 33u, 34u, 35u, 36u,
			37u, 38u, 39u, 40u, 41u, 42u, 43u, 44u, 45u, 46u,
	};

	for (_uint iMeshIndex : PieceIndices)
		Set_MeshVisible(iMeshIndex, false);

	Play_Anim(ANIM_BEFOREWAIT);

	return S_OK;
}

void CLD_Stage1BossDemo::On_Event(const _wstring& strEventTag)
{
	UNREFERENCED_PARAMETER(strEventTag);

	m_eDemoState = DEMO_STATE::PLAYING;
	Play_Anim(ANIM_APPEAR);
}

void CLD_Stage1BossDemo::On_AnimEvent(const ANIM_EVENT& AnimEvent, ANIM_EVENT_PHASE ePhase)
{
	if (ANIM_EVENT_PHASE::POINT != ePhase)
		return;

	switch (static_cast<EANIM_EVENT>(AnimEvent.iEventType))
	{
	case EANIM_EVENT::PubEvent:
	{
		if (AnimEvent.strParam.empty())
			break;

		const _wstring strEventTag(AnimEvent.strParam.begin(), AnimEvent.strParam.end());
		m_pGameInstance_Proxy->Publish(strEventTag, nullptr);
		break;
	}

	case EANIM_EVENT::OnOffMesh:
	{
		Set_AllMeshesVisible(true);

		static constexpr _uint AfterMeshIndices[] = { 47u, 48u };
		for (_uint iMeshIndex : AfterMeshIndices)
			Set_MeshVisible(iMeshIndex, false);

		break;
	}

	default:
		break;
	}
}

_bool CLD_Stage1BossDemo::Should_RenderMesh(_uint iMeshIndex) const
{
	if (!__super::Should_RenderMesh(iMeshIndex))
		return false;

	if (Is_Stage1BossDemoGlassMesh(m_pModelCom->Get_MeshName(iMeshIndex))
		&& !ENABLE_STAGE1_BOSS_DEMO_GLASS_PASS)
	{
		return false;
	}

	return true;
}

_uint CLD_Stage1BossDemo::Resolve_RenderPass(_uint iMeshIndex) const
{
	if (Is_Stage1BossDemoGlassMesh(m_pModelCom->Get_MeshName(iMeshIndex)))
		return STAGE1_BOSS_DEMO_GLASS_PASS;

	return EVENTOBJECT_ANIM_DEFAULT_PASS;
}

CLD_Stage1BossDemo* CLD_Stage1BossDemo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_Stage1BossDemo* pInstance = new CLD_Stage1BossDemo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_Stage1BossDemo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_Stage1BossDemo::Clone(void* pArg)
{
	CLD_Stage1BossDemo* pInstance = new CLD_Stage1BossDemo(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_Stage1BossDemo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

NS_END