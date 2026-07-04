#include "LD_SlopeBoardC.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"

namespace
{
	inline constexpr const _char* SLOPEBOARD_C_MODEL_PATH = "../../Resources/Map/Gimmick/Anim/SlopeBoard/SlopeBoardC.ysh";
	inline constexpr const _char * SLOPEBOARD_C_ANIM_NAMES[LD_ANIM_SLOT_COUNT] = { "FallenWait", "Wait", "", "" };
}

NS_BEGIN(Client)

CLD_SlopeBoardC::CLD_SlopeBoardC(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLD_EventObject(pDevice, pContext)
{
}

CLD_SlopeBoardC::CLD_SlopeBoardC(const CLD_SlopeBoardC& Prototype)
	: CLD_EventObject(Prototype)
{
}

HRESULT CLD_SlopeBoardC::Validate_Initialized()
{
	if (FAILED(__super::Validate_Initialized()))
		return E_FAIL;

	if (!JsonUtils::Equals_NoCase(OBJECT_NAME, m_tEventObjectDesc.strObjectName.c_str()))
		return E_FAIL;

	if (m_tEventObjectDesc.eModelType != MODEL::ANIM || m_tEventObjectDesc.wstrModelProtoTag !=
		MODEL_PROTO_TAG)
		return E_FAIL;

	if (!m_tEventObjectDesc.bUseCollMesh || !m_tEventObjectDesc.strAnimEventFile.empty())
		return E_FAIL;

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
	{
		if (m_tEventObjectDesc.strAnimNames[i] != SLOPEBOARD_C_ANIM_NAMES[i])
			return E_FAIL;
	}

	return S_OK;
}

void CLD_SlopeBoardC::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

void CLD_SlopeBoardC::Register_LevelDesignSpecs()
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
	Spec.ModelRequirements = { { MODEL_PROTO_TAG, SLOPEBOARD_C_MODEL_PATH, MODEL::ANIM, true }, };

	CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
}

_bool CLD_SlopeBoardC::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const
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
	Desc.strAnimEventFile.clear();

	for (_uint i = 0; i < LD_ANIM_SLOT_COUNT; ++i)
		Desc.strAnimNames[i] = SLOPEBOARD_C_ANIM_NAMES[i];

	*pOutEntry = Desc;
	return true;
}

CGameObject* CLD_SlopeBoardC::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return CLD_SlopeBoardC::Create(pDevice, pContext);
}

CLD_SlopeBoardC* CLD_SlopeBoardC::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLD_SlopeBoardC* pInstance = new CLD_SlopeBoardC(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLD_SlopeBoardC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLD_SlopeBoardC::Clone(void* pArg)
{
	CLD_SlopeBoardC* pInstance = new CLD_SlopeBoardC(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLD_SlopeBoardC");
		Safe_Release(pInstance);
	}

	return pInstance;
}

NS_END