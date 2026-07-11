#include "Panel_Inspector.h"
#include "EditInstance.h"
#include "Level_Edit.h"
#include "Shader_PassMeta.h"
#include "MapStage.h"
#include "MapSection.h"
#include "MapEvent_BreakWall.h"
#include "Map_EditFile.h"
#include "Map_EditSession.h"
#include "EnvObject.h"
#include "EnvTrigger_RenderGlobals.h"
#include "LevelDesignObject.h"
#include "LevelDesign_Bush.h"
#include "Editable.h"
#include "EffectPart_Enum.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "ContainerObject.h"
#include "PartObject.h"
#include "UIContainerObject.h"
#include "UIPartObject.h"

#include "imgui.h"
#include <cmath>

namespace
{
	const _char* TexTypeName(_uint t)
	{
		static const char* names[MTEX_TYPE_MAX] = {
				"None","Diffuse","Specular","Ambient","Emissive","Height","Normals","Shininess",
				"Opacity","Displacement","Lightmap","Reflection","BaseColor","NormalCamera",
				"EmissionColor","Metalness(MRA)","Roughness","AO","Unknown(Mask)","Sheen","Clearcoat",
				"Transmission","MayaBase","MayaSpecular","MayaSpecColor","MayaSpecRough","Anisotropy"
		};

		return (t < MTEX_TYPE_MAX) ? names[t] : "?";
	}

	const _char* GetMapShaderPassComboItem(void*, _int idx)
	{
		if (idx < 0 || idx >= static_cast<_int>(_countof(g_MapShaderPassMetas)))
			return nullptr;

		return g_MapShaderPassMetas[idx].szName;
	}

	_bool Is_EnvImportantTexType(MTEX_TYPE eType)
	{
		switch (eType)
		{
		case MTEX_TYPE::DIFFUSE:
		case MTEX_TYPE::NORMALS:
		case MTEX_TYPE::METALNESS:
		case MTEX_TYPE::UNKNOWN:
			return true;
		default:
			return false;
		}
	}

	int ToMapRenderGroupIndex(RENDERID eRenderID)
	{
		return (eRenderID == RENDERID::BLEND) ? 1 : 0;
	}

	RENDERID FromMapRenderGroupIndex(int iIndex)
	{
		return (iIndex == 1) ? RENDERID::BLEND : RENDERID::NONBLEND;
	}

	_bool* FindBoolProperty(IReflectable* pHolder, const _wstring& strName, const _wstring& strCategory)
	{
		if (nullptr == pHolder)
			return nullptr;

		for (const Engine::FPROPERTY& Property : pHolder->Get_Properties())
		{
			if (Engine::PROP_TYPE::BOOL != Property.eType)
				continue;
			if (Property.strName != strName || Property.strCategory != strCategory)
				continue;

			return static_cast<_bool*>(pHolder->Get_PropertyPtr(Property.uOffset));
		}

		return nullptr;
	}

	_bool ReadBoolProperty(const IReflectable* pHolder, const _wstring& strName, const _wstring& strCategory, _bool bDefault = false)
	{
		if (nullptr == pHolder)
			return bDefault;

		for (const Engine::FPROPERTY& Property : pHolder->Get_Properties())
		{
			if (Engine::PROP_TYPE::BOOL != Property.eType)
				continue;
			if (Property.strName != strName || Property.strCategory != strCategory)
				continue;

			const _bool* pValue = static_cast<const
				_bool*>(pHolder->Get_PropertyPtr(Property.uOffset));
			return nullptr != pValue ? *pValue : bDefault;
		}

		return bDefault;
	}

	const _char* GetWorldShaderPassComboItem(void*, _int idx)
	{
		if (idx < 0 || idx >= static_cast<int>(_countof(g_WorldShaderPassMetas)))
			return nullptr;

		return g_WorldShaderPassMetas[idx].szName;
	}

	struct MESH_LAYER_UI_CONTEXT
	{
		CModel* pModel = { nullptr };
		const _tchar* pModelComponentTag = { L"Com_Model" };

		_bool bEnvObjectMeshUi = { false };
		_bool bMapObjectMeshUi = { false };
		_bool bWorldPassMeshUi = { false };
		_bool bBushMeshUi = { false };
		_bool bBushBasicMeshUi = { false };

		IEditable* pEditable = { nullptr };
		EDITABLE_DESC EditDesc = {};
		_uint iModelSlot = { 0u };
	};

	MESH_LAYER_UI_CONTEXT Resolve_MeshLayerUIContext(CGameObject* pObject, int iModelSlot)
	{
		MESH_LAYER_UI_CONTEXT Ctx{};

		if (nullptr == pObject)
			return Ctx;

		Ctx.pEditable = dynamic_cast<IEditable*>(pObject);
		if (nullptr != Ctx.pEditable)
		{
			if (!Ctx.pEditable->Get_EditDesc(&Ctx.EditDesc))
				return Ctx;

			if (0u == (Ctx.EditDesc.iCapabilities & EDIT_CAP_MESH_LAYER))
				return Ctx;

			if (Ctx.EditDesc.ModelSlots.empty())
				return Ctx;

			if (iModelSlot < 0 || static_cast<size_t>(iModelSlot) >= Ctx.EditDesc.ModelSlots.size())
				iModelSlot = 0;

			Ctx.iModelSlot = static_cast<_uint>(iModelSlot);
			const EDITABLE_MODEL_SLOT& Slot = Ctx.EditDesc.ModelSlots[Ctx.iModelSlot];

			Ctx.pModel = Slot.pModel;
			Ctx.bBushMeshUi = nullptr != dynamic_cast<CLevelDesign_Bush*>(pObject);
			Ctx.bBushBasicMeshUi = Ctx.bBushMeshUi && Slot.strLabel == L"Basic";
			Ctx.bEnvObjectMeshUi = EDITABLE_OBJECT_KIND::ENV_OBJECT == Ctx.EditDesc.eKind;
			Ctx.bMapObjectMeshUi = nullptr != dynamic_cast<Client::CMapObject*>(pObject);
			Ctx.bWorldPassMeshUi =
				EDITABLE_OBJECT_KIND::ENV_OBJECT == Ctx.EditDesc.eKind ||
				EDITABLE_OBJECT_KIND::LEVEL_DESIGN_OBJECT == Ctx.EditDesc.eKind;

			return Ctx;
		}

		Ctx.bBushMeshUi = nullptr != dynamic_cast<CLevelDesign_Bush*>(pObject);
		Ctx.bBushBasicMeshUi = Ctx.bBushMeshUi && 0 == iModelSlot;

		Ctx.pModelComponentTag = Ctx.bBushMeshUi
			? (Ctx.bBushBasicMeshUi ? L"Com_Model_Basic" : L"Com_Model_Cut")
			: L"Com_Model";

		Ctx.pModel = pObject->Get_Component<CModel>(Ctx.pModelComponentTag);

		Ctx.bEnvObjectMeshUi = nullptr != dynamic_cast<CEnvObject*>(pObject);
		Ctx.bMapObjectMeshUi = nullptr != dynamic_cast<CMapObject*>(pObject);
		Ctx.bWorldPassMeshUi =
			Ctx.bEnvObjectMeshUi ||
			nullptr != dynamic_cast<CLevelDesignObject*>(pObject);

		return Ctx;
	}

#pragma region MAP_LAYER_EX
	static const _char* kLayerExGroupName[MAP_LAYER_EX_GROUP::GROUP_COUNT] =
	{
		"Main", "ExtR", "ExtG", "ExtB", "ExtA",
	};

	static const _char* kLayerExEntryName[MESH_LAYER_EX_ENTRY_COUNT] =
	{
		  "DIFF", "MRA", "NORM", "UKWN",
	};

	static const MTEX_TYPE kLayerExTexType[MESH_LAYER_EX_ENTRY_COUNT] =
	{
		  MTEX_TYPE::DIFFUSE,
		  MTEX_TYPE::METALNESS,
		  MTEX_TYPE::NORMALS,
		  MTEX_TYPE::UNKNOWN
	};

	void Fill_LayerExBind(MESH_LAYER_TEX_BIND_EX& Out, MTEX_TYPE eType, _int iSlot,
		_uint iUVIndex, const _float2 vScale, const _float2 vOffset, _float fRotate)
	{
		Out.bEnable = (iSlot >= 0);
		Out.iTexType = ETOUI(eType);
		Out.iSlot = iSlot;
		Out.iUVIndex = (iUVIndex <= 3u) ? iUVIndex : 0u;
		Out.vUVScale = vScale;
		Out.vUVOffset = vOffset;
		Out.fUVRotate = fRotate;
	}

	void InitializeLayerExFromLegacy(MESH_LAYER_IDX& Layer)
	{
		// Init
		for (_uint g = 0; g < MESH_LAYER_EX_GROUP_COUNT; ++g)
		{
			for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
				Layer.LayerEx[g][e] = {};
		}

		const _float2 vOffset = Layer.vUVOffset;
		const _float fRotate = Layer.fUVRotate;

		// Main.Diffuse
		Fill_LayerExBind(
			Layer.LayerEx[MAIN][LAYER_EX_DIFF],
			MTEX_TYPE::DIFFUSE,
			static_cast<_int>(Layer.idx[ETOUI(MTEX_TYPE::DIFFUSE)]),
			Layer.iUVIndex,
			Layer.bUseUVTransform ? Layer.vUVScale : XMFLOAT2{ 1.f, 1.f },
			Layer.bUseUVTransform ? vOffset : XMFLOAT2{ 0.f, 0.f },
			Layer.bUseUVTransform ? fRotate : 0.f);

		// Main.MRA
		Fill_LayerExBind(
			Layer.LayerEx[MAIN][LAYER_EX_MRA],
			MTEX_TYPE::METALNESS,
			static_cast<_int>(Layer.idx[ETOUI(MTEX_TYPE::METALNESS)]),
			Layer.iUVIndex,
			Layer.bUseUVTransform ? Layer.vUVScaleMaterial : XMFLOAT2{ 1.f, 1.f },
			Layer.bUseUVTransform ? vOffset : XMFLOAT2{ 0.f, 0.f },
			Layer.bUseUVTransform ? fRotate : 0.f);

		// Main.Normal
		Fill_LayerExBind(
			Layer.LayerEx[MAIN][LAYER_EX_NORM],
			MTEX_TYPE::NORMALS,
			static_cast<_int>(Layer.idx[ETOUI(MTEX_TYPE::NORMALS)]),
			Layer.iUVIndex,
			Layer.bUseUVTransform ? Layer.vUVScaleNormal : XMFLOAT2{ 1.f, 1.f },
			Layer.bUseUVTransform ? vOffset : XMFLOAT2{ 0.f, 0.f },
			Layer.bUseUVTransform ? fRotate : 0.f);

		// Main.Unknown
		Fill_LayerExBind(
			Layer.LayerEx[MAIN][LAYER_EX_UKWN],
			MTEX_TYPE::UNKNOWN,
			static_cast<_int>(Layer.idx[ETOUI(MTEX_TYPE::UNKNOWN)]),
			Layer.iUnknownUVIndex,
			Layer.bUseUVTransform ? Layer.vUVScale : XMFLOAT2{ 1.f, 1.f },
			Layer.bUseUVTransform ? vOffset : XMFLOAT2{ 0.f, 0.f },
			Layer.bUseUVTransform ? fRotate : 0.f);

		// Extra R/G/B/A → Group 1~4
		for (_uint c = 0; c < 4; ++c)
		{
			const _int iSlot = Layer.iExtraBind[c];
			if (iSlot < 0)
				continue;

			const MTEX_TYPE eTexType = static_cast<MTEX_TYPE>(Layer.iExtraTexType[c]);

			_uint iEntry = LAYER_EX_UKWN;
			XMFLOAT2 vScale = Layer.bUseUVTransform ? Layer.vUVScale : XMFLOAT2{ 1.f, 1.f };

			if (eTexType == MTEX_TYPE::DIFFUSE)
			{
				iEntry = LAYER_EX_DIFF;
				vScale = Layer.bUseUVTransform ? Layer.vUVScale : XMFLOAT2{ 1.f, 1.f };
			}
			else if (eTexType == MTEX_TYPE::METALNESS)
			{
				iEntry = LAYER_EX_MRA;
				vScale = Layer.bUseUVTransform ? Layer.vUVScaleMaterial : XMFLOAT2{ 1.f, 1.f };
			}
			else if (eTexType == MTEX_TYPE::NORMALS)
			{
				iEntry = LAYER_EX_NORM;
				vScale = Layer.bUseUVTransform ? Layer.vUVScaleNormal : XMFLOAT2{ 1.f, 1.f };
			}

			Fill_LayerExBind(
				Layer.LayerEx[c + 1][iEntry],
				eTexType,
				iSlot,
				Layer.iExtraUVIndex[c],
				vScale,
				Layer.bUseUVTransform ? vOffset : XMFLOAT2{ 0.f, 0.f },
				Layer.bUseUVTransform ? fRotate : 0.f);
		}

		Layer.bUseLayerEx = true;
	}
#pragma endregion
}

CPanel_Inspector::CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPanel(pDevice, pContext)
{
	strcpy_s(m_szName, "Inspector");
}

void CPanel_Inspector::Render()
{
	if (!Begin_Panel())
	{
		End_Panel();
		return;
	}

	CLevel_Edit* pLevel = CEditInstance::GetInstance()->Get_Level();
	if (nullptr == pLevel)
	{
		End_Panel();
		return;
	}

	CGameObject* pSelected = pLevel->Get_Selected();
	if (nullptr == pSelected)
	{
		End_Panel();
		return;
	}

	_bool bRenderGlobalsDirty = false;

	const _bool bTransformChanged = Draw_Transform(pSelected);
	if (bTransformChanged)
	{
		if (IEditable* pEditable = dynamic_cast<IEditable*>(pSelected))
		{
			const HRESULT hr = pEditable->On_EditTransformChanged();
#ifdef _DEBUG
			if (FAILED(hr))
				OutputDebugStringA("[MapTool] IEditable::On_EditTransformChanged failed in Inspector.\n");
#else
			UNREFERENCED_PARAMETER(hr);
#endif
		}
	}

	ImGui::Separator();
	Draw_EditableObjectPolicyPanel(pSelected);

	if (dynamic_cast<CEnvObject*>(pSelected))
	{
		ImGui::Separator();
		Draw_EnvObjectEditPanel(pLevel, pSelected);
	}

	if (dynamic_cast<CMapEvent_BreakWall*>(pSelected))
	{
		ImGui::Separator();
		Draw_MapSectionEditPanel(pLevel, nullptr, pSelected);
	}

	ImGui::Separator();
	Draw_MeshLayerPanel(pSelected);

	ImGui::Separator();
	bRenderGlobalsDirty |= Draw_Properties(pSelected);

	if (bRenderGlobalsDirty)
	{
		if (auto* pRenderGlobals = dynamic_cast<Client::CEnvTrigger_RenderGlobals*>(pSelected))
			pRenderGlobals->Apply_RenderGlobals();
	}

	for (auto& [tag, pComponent] : pSelected->Get_Components())
	{
		if (!pComponent) continue;
		if (pComponent->Get_Properties().empty()) continue;

		string strTag = WstrToStr(tag);
		if (ImGui::CollapsingHeader(strTag.c_str()))
		{
			ImGui::PushID(pComponent);
			Draw_Properties(pComponent);
			ImGui::PopID();
		}
	}

	ImGui::Separator();

	if (auto pContainer = dynamic_cast<CContainerObject*>(pSelected))
	{
		auto& PartObjects = pContainer->Get_PartObjects();
		vector<pair<wstring, CPartObject*>> vecSorted(PartObjects.begin(), PartObjects.end());
		sort(vecSorted.begin(), vecSorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

		for (auto& [tag, pPart] : vecSorted)
		{
			string strTag = WstrToStr(tag);
			if (ImGui::CollapsingHeader(strTag.c_str()))
			{
				ImGui::PushID(pPart);
				Draw_Transform(pPart, strTag);
				ImGui::Separator();
				Draw_Properties(pPart);
				ImGui::PopID();
			}
		}
	}

	if (auto pUIContainer = dynamic_cast<CUIContainerObject*>(pSelected))
	{
		auto& UIPartObjects = pUIContainer->Get_UIPartObjects();
		vector<pair<wstring, CUIPartObject*>> vecSorted(UIPartObjects.begin(), UIPartObjects.end());
		sort(vecSorted.begin(), vecSorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

		for (auto& [tag, pPart] : vecSorted)
		{
			string strTag = WstrToStr(tag);
			if (ImGui::CollapsingHeader(strTag.c_str()))
			{
				ImGui::PushID(pPart);
				Draw_Transform(pPart, strTag);
				ImGui::Separator();
				Draw_Properties(pPart);
				ImGui::PopID();
			}
		}
	}

	if (auto pMapStage = dynamic_cast<Client::CMapStage*>(pSelected))
		Draw_MapStageSections(pMapStage);

	End_Panel();
}

_bool CPanel_Inspector::Draw_Properties(IReflectable* pHolder)
{
	_bool bChanged = false;
	string strCurrentCategory = {};

	const _bool bSkipEnvObjectCategory =
		nullptr != dynamic_cast<CEnvObject*>(pHolder);
	const _bool bSkipMapSectionCategory =
		nullptr != dynamic_cast<CMapSection*>(pHolder)
		|| nullptr != dynamic_cast<CMapEvent_BreakWall*>(pHolder);

	for (auto& prop : pHolder->Get_Properties())
	{
		if (bSkipEnvObjectCategory && prop.strCategory == L"EnvObject")
			continue;

		if (bSkipMapSectionCategory && prop.strCategory == L"MapSection")
			continue;

		const string strPropCategory = WstrToStr(prop.strCategory);
		if (strPropCategory != strCurrentCategory)
		{
			strCurrentCategory = strPropCategory;
			ImGui::Text("[%s]", strCurrentCategory.c_str());
		}

		void* pData = pHolder->Get_PropertyPtr(prop.uOffset);
		if (!pData) continue;

		string strPropName = WstrToStr(prop.strName);

		switch (prop.eType)
		{
		case PROP_TYPE::INT:
		{
			ImGui::Text(strPropName.c_str());
			int* pValue = (int*)pData;
			const string strID = "##" + strPropName;
			if (const Engine::EFFECTPART_ENUM_ITEMS* pEnumItems = Engine::Find_EffectPartPropertyEnum(prop))
			{
				const wstring* pEnumName = Engine::Find_EffectPartEnumName(*pEnumItems, *pValue);
				string strPreview = pEnumName ? WstrToStr(*pEnumName) : to_string(*pValue);

				if (ImGui::BeginCombo(strID.c_str(), strPreview.c_str()))
				{
					for (const auto& Item : *pEnumItems)
					{
						const bool bSelected = Item.first == *pValue;
						string strItemLabel = WstrToStr(Item.second) + "##" + to_string(Item.first);
						if (ImGui::Selectable(strItemLabel.c_str(), bSelected))
						{
							*pValue = Item.first;
							bChanged = true;
						}
						if (bSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			else if (ImGui::InputInt(strID.c_str(), pValue))
				bChanged = true;
			break;
		}
		case PROP_TYPE::FLOAT:
			ImGui::Text(strPropName.c_str());
			if (ImGui::DragFloat(("##" + strPropName).c_str(), (float*)pData, 0.1f))
				bChanged = true;
			break;
		case PROP_TYPE::BOOL:
			ImGui::Text(strPropName.c_str());
			if (ImGui::Checkbox(("##" + strPropName).c_str(), (bool*)pData))
				bChanged = true;
			break;
		case PROP_TYPE::FLOAT2:
			ImGui::Text(strPropName.c_str());
			if (ImGui::DragFloat2(("##" + strPropName).c_str(), (float*)pData, 0.1f))
				bChanged = true;
			break;
		case PROP_TYPE::FLOAT3:
			ImGui::Text(strPropName.c_str());
			if (ImGui::DragFloat3(("##" + strPropName).c_str(), (float*)pData, 0.1f))
				bChanged = true;
			break;
		case PROP_TYPE::FLOAT4:
			ImGui::Text(strPropName.c_str());
			if (ImGui::DragFloat4(("##" + strPropName).c_str(), (float*)pData, 0.1f))
				bChanged = true;
			break;
		case PROP_TYPE::ANIM_INDEX:
		{
			int iVal = (int)*((_uint*)pData);
			CGameObject* pObject = dynamic_cast<CGameObject*>(pHolder);
			CModel* pModel = pObject ? pObject->Get_Component<CModel>(L"Com_Model") : nullptr;
			if (!pModel) break;

			_uint iNumAnims = pModel->Get_MaxAnimationIndex() + 1;
			string strItems;
			for (_uint i = 0; i < iNumAnims; ++i)
				strItems += to_string(i) + ": " + pModel->Get_AnimationName(i) + '\0';
			strItems += '\0';

			ImGui::Text(strPropName.c_str());
			ImGui::PushID(strPropName.c_str());
			if (ImGui::Combo(("##" + strPropName).c_str(), &iVal, strItems.c_str()))
			{
				*(_uint*)pData = (_uint)iVal;
				bChanged = true;
			}
			ImGui::PopID();
			break;
		}
		case PROP_TYPE::WSTRING:
		{
			wstring* pWstr = (wstring*)pData;
			string str = WstrToStr(*pWstr);
			char buf[256] = {};
			strcpy_s(buf, str.c_str());
			if (ImGui::InputText(("##" + strPropName).c_str(), buf, sizeof(buf)))
			{
				*pWstr = StrToWstr(buf);
				bChanged = true;
			}
			break;
		}
		}
	}

	return bChanged;
}

_bool CPanel_Inspector::Draw_Transform(CGameObject* pObject, const string& strSuffix)
{
	_bool bChanged = false;

	CTransform* pTransform = pObject->Get_Transform();

	_float4 vPos = {};
	XMStoreFloat4(&vPos, pTransform->Get_State(STATE::POSITION));
	if (ImGui::DragFloat3(("Position##" + strSuffix).c_str(), (float*)&vPos, 0.1f))
	{
		pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPos));
		bChanged = true;
	}

	_float3& vEuler = m_RotEditEuler[pObject];
	_float3 vBefore = vEuler;
	if (ImGui::DragFloat3(("Rotate##" + strSuffix).c_str(), (float*)&vEuler, 0.5f))
	{
		_float3 vDelta = { vEuler.x - vBefore.x, vEuler.y - vBefore.y, vEuler.z - vBefore.z };

		auto ApplyDelta = [pTransform](_fvector vAxis, _float fDegree)
			{
				if (fDegree == 0.f) return;
				_vector vQuat = XMQuaternionRotationNormal(vAxis, XMConvertToRadians(fDegree));
				pTransform->Rotate(vQuat);
			};

		_vector vR = XMVector3Normalize(pTransform->Get_State(STATE::RIGHT));
		_vector vU = XMVector3Normalize(pTransform->Get_State(STATE::UP));
		_vector vL = XMVector3Normalize(pTransform->Get_State(STATE::LOOK));

		ApplyDelta(vR, vDelta.x);
		ApplyDelta(vU, vDelta.y);
		ApplyDelta(vL, vDelta.z);
		bChanged = true;
	}

	_float3 vScale = pTransform->Get_Scaled();

	if (dynamic_cast<CUIObject*>(pObject))
	{
		static CGameObject* pPrevObj = nullptr;
		static _float fRatioScale = 1.f;
		static _float fBaseScaleX{}, fBaseScaleY{};

		if (pPrevObj != pObject)
		{
			fRatioScale = 1.f;
			fBaseScaleX = vScale.x;
			fBaseScaleY = vScale.y;
			pPrevObj = pObject;
		}

		_float2 vUIScale = { vScale.x, vScale.y };
		if (ImGui::DragFloat2(("Scale##" + strSuffix).c_str(), (float*)&vUIScale, 0.1f))
		{
			pTransform->Set_Scale(vUIScale.x, vUIScale.y, 1.f);
			bChanged = true;
		}

		if (ImGui::DragFloat(("Uniform Scale##" + strSuffix).c_str(), &fRatioScale, 0.01f, 0.01f, 100.f))
		{
			pTransform->Set_Scale(fBaseScaleX * fRatioScale, fBaseScaleY * fRatioScale, 1.f);
			bChanged = true;
		}
	}
	else
	{
		_float fUniformScale = vScale.x;
		if (ImGui::DragFloat(("Scale##" + strSuffix).c_str(), &fUniformScale, 0.1f))
		{
			pTransform->Set_Scale(fUniformScale, fUniformScale, fUniformScale);
			bChanged = true;
		}
	}

	return bChanged;
}

void CPanel_Inspector::Draw_EditableObjectPolicyPanel(CGameObject* pObject)
{
	IEditable* pEditable = dynamic_cast<IEditable*>(pObject);
	if (nullptr == pEditable)
		return;

	EDITABLE_DESC EditDesc{};
	if (!pEditable->Get_EditDesc(&EditDesc))
		return;

	const _uint iPolicyCaps =
		EDIT_CAP_RENDERABLE
		| EDIT_CAP_CULL_DISTANCE
		| EDIT_CAP_CULL_FRUSTUM
		| EDIT_CAP_COLLISION_MESH
		| EDIT_CAP_SHADOW;

	if (0u == (EditDesc.iCapabilities & iPolicyCaps))
		return;

	if (!ImGui::CollapsingHeader("Object Policy##Editable", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	const _wstring strStateKey = EditDesc.strStableKey.empty()
		? L"ptr|" + to_wstring(reinterpret_cast<size_t>(pObject))
		: EditDesc.strStableKey;

	auto IterPolicy = m_EditablePolicyDrafts.find(strStateKey);
	if (IterPolicy == m_EditablePolicyDrafts.end())
		IterPolicy = m_EditablePolicyDrafts.emplace(strStateKey, EditDesc.Policy).first;

	EDIT_OBJECT_POLICY& NewPolicy = IterPolicy->second;
	_bool bPolicyChanged = false;

	if (EditDesc.iCapabilities & EDIT_CAP_RENDERABLE)
		bPolicyChanged |= ImGui::Checkbox("Renderable##EditablePolicy", (bool*)&NewPolicy.bRenderable);

	if (EditDesc.iCapabilities & EDIT_CAP_CULL_DISTANCE)
		bPolicyChanged |= ImGui::Checkbox("Use Distance Culling##EditablePolicy", (bool*)&NewPolicy.bUseCullDistance);

	if (EditDesc.iCapabilities & EDIT_CAP_CULL_FRUSTUM)
		bPolicyChanged |= ImGui::Checkbox("Use Frustum Culling##EditablePolicy", (bool*)&NewPolicy.bUseCullFrustum);

	if (EditDesc.iCapabilities & EDIT_CAP_COLLISION_MESH)
		bPolicyChanged |= ImGui::Checkbox("Use Collision Mesh##EditablePolicy", (bool*)&NewPolicy.bUseCollMesh);

	if (EditDesc.iCapabilities & EDIT_CAP_SHADOW)
		bPolicyChanged |= ImGui::Checkbox("Use Shadow##EditablePolicy", (bool*)&NewPolicy.bUseShadow);

	auto ApplyPolicyDraft = [&]() -> _bool
	{
		if (FAILED(pEditable->Apply_EditPolicy(NewPolicy)))
			return false;

		EDITABLE_DESC AppliedDesc{};
		if (pEditable->Get_EditDesc(&AppliedDesc))
			NewPolicy = AppliedDesc.Policy;

		return true;
	};

	if (bPolicyChanged && !ApplyPolicyDraft())
		MSG_BOX("OBJECT POLICY APPLY FAILED");

	if (ImGui::Button("Apply Object Policy##EditablePolicy") && !ApplyPolicyDraft())
		MSG_BOX("OBJECT POLICY APPLY FAILED");

	ImGui::SameLine();
	if (ImGui::Button("Reset##EditablePolicy"))
	{
		NewPolicy = EditDesc.Policy;
		if (!ApplyPolicyDraft())
			MSG_BOX("OBJECT POLICY APPLY FAILED");
	}
}

void CPanel_Inspector::Draw_EnvObjectEditPanel(CLevel_Edit* pLevel, CGameObject* pObject)
{
	Client::CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject);
	if (nullptr == pLevel || nullptr == pEnvObject)
		return;

	_bool* pbUseNearDistAlpha = Resolve_EnvNearAlphaEditState(pLevel, pEnvObject);

	const ENV_OBJECT_DESC& Desc = pEnvObject->Get_Desc();

	ImGui::TextUnformatted("EnvObject Override");

	if (pbUseNearDistAlpha)
		ImGui::Checkbox("Use Near Dist Alpha##EnvEdit", (bool*)pbUseNearDistAlpha);

	ImGui::TextDisabled("Object Policy is handled by the common panel.");
	ImGui::TextDisabled("Apply Override stores these values in the edit session.");
	ImGui::TextDisabled("Save Override Now persists applied overrides.");

	auto CommitCurrentEnvEdit = [&]() -> _bool
		{
			if (!pLevel->Commit_MapEditObjectFromCurrentState(pObject))
				return false;

			const _bool bUseNearDistAlpha = (nullptr != pbUseNearDistAlpha)
				? *pbUseNearDistAlpha
				: Desc.tRender.bUseNearDistAlpha;

			EDIT_OBJECT_OVERRIDE_DESC Edit{};
			Edit.eKind = EDITABLE_OBJECT_KIND::ENV_OBJECT;
			pLevel->Try_GetMapPreviewEnvEdit(pObject, &Edit);

			EDIT_ENVOBJECT_OVERRIDE EnvOverride{};
			if (const EDIT_ENVOBJECT_OVERRIDE* pSavedEnvOverride = get_if<EDIT_ENVOBJECT_OVERRIDE>(&Edit.ClassOverride))
				EnvOverride = *pSavedEnvOverride;

			if (bUseNearDistAlpha != Desc.tRender.bUseNearDistAlpha)
			{
				EnvOverride.bHasNearDistAlpha = true;
				EnvOverride.bUseNearDistAlpha = bUseNearDistAlpha;
				Edit.ClassOverride = EnvOverride;
			}
			else
			{
				EnvOverride.bHasNearDistAlpha = false;
				EnvOverride.bUseNearDistAlpha = false;
				Edit.ClassOverride = (EnvOverride.bHasNearDistAlpha || EnvOverride.bHasDecalAlpha)
					? EDIT_CLASS_OVERRIDE{ EnvOverride }
					: EDIT_CLASS_OVERRIDE{ monostate{} };
			}

			if (Has_AnyEdit(Edit))
				return pLevel->Track_EditedMapPreviewEnvObject(pObject, Edit);

			pLevel->Clear_EditedMapPreviewEnvObject(pObject);
			Clear_EnvNearAlphaEditState(pObject);
			return true;
		};

	if (ImGui::Button("Apply Override##EnvEdit"))
	{
		if (!CommitCurrentEnvEdit())
			MSG_BOX("MAP EDIT APPLY FAILED");
	}

	ImGui::SameLine();

	if (ImGui::Button("Save Override Now##EnvEdit"))
	{
		if (!CommitCurrentEnvEdit())
		{
			MSG_BOX("MAP EDIT APPLY FAILED");
		}
		else if (FAILED(pLevel->Save_MapOverride()))
		{
			MSG_BOX("OBJECT OVERRIDE SAVE FAILED");
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear Override##EnvEdit"))
	{
		const CMap_EditSession* pSession = pLevel->Get_MapPreviewSession();
		const _int iPresetIndex =
			(nullptr != pSession) ? pSession->Get_EditData().iPresetIndex : -1;

		pLevel->Clear_EditedMapPreviewEnvObject(pObject);
		m_EnvNearAlphaEditStates.clear();

		if (0 <= iPresetIndex)
		{
			pLevel->Load_MapPreviewEnv(static_cast<_uint>(iPresetIndex));
			return;
		}
	}
}

void CPanel_Inspector::Draw_MapSectionEditPanel(CLevel_Edit* pLevel, CMapStage* pMapStage, CGameObject* pObject)
{
	if (nullptr == pLevel || nullptr == pObject)
		return;

	CMapSection* pMapSection = dynamic_cast<CMapSection*>(pObject);
	CMapEvent_BreakWall* pBreakWall = dynamic_cast<CMapEvent_BreakWall*>(pObject);

	if (nullptr == pMapSection && nullptr == pBreakWall)
		return;
	if (nullptr != pMapSection && nullptr == pMapStage)
		return;

	_bool* pbRenderable = nullptr;
	if (nullptr != pBreakWall)
		pbRenderable = FindBoolProperty(pBreakWall, L"Renderable", L"MapEvent_BreakWall");

	_wstring strStageName;
	_wstring strSectionName;

	if (nullptr != pMapSection)
	{
		strStageName = pMapStage->Get_StageName();
		strSectionName = pMapSection->Get_SectionName();
	}
	else
	{
		const CMap_EditSession* pSession = pLevel->Get_MapPreviewSession();
		strStageName = (nullptr != pSession && !pSession->Get_LoadedStageName().empty())
			? pSession->Get_LoadedStageName()
			: CMapEvent_BreakWall::STAGE12_STAGE_NAME;
		strSectionName = pBreakWall->Get_SectionName();
	}

	const string strStageNameText = WstrToStr(strStageName);
	const string strSectionNameText = WstrToStr(strSectionName);

	ImGui::TextUnformatted(nullptr != pBreakWall ? "MapEvent_BreakWall Edit" : "MapSection Edit");
	if (nullptr != pBreakWall)
		ImGui::TextDisabled("Type: MapEvent_BreakWall");
	ImGui::TextDisabled("Stage: %s", strStageNameText.empty() ? "<Unnamed Stage>" : strStageNameText.c_str());
	ImGui::TextDisabled("Section: %s", strSectionNameText.empty() ? "<Unnamed Section>" : strSectionNameText.c_str());

	const _char* pFlagsHeader = nullptr != pBreakWall ? "MapEvent_BreakWall Flags" : "Section Flags";
	const _char* pRenderableLabel = nullptr != pBreakWall ? "Renderable##MapEventBreakWallEdit" : "Renderable##SectionEdit";

	if (ImGui::CollapsingHeader(pFlagsHeader, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (nullptr != pMapSection)
		{
			if (!pMapSection->Has_CollMesh())
				ImGui::TextDisabled("Collision mesh unavailable.");

			ImGui::TextDisabled("Renderable, culling, and collision mesh are handled by Object Policy.");
			ImGui::TextDisabled("Shadow depth is always submitted.");
			ImGui::TextDisabled("Apply on reload.");
			ImGui::TextDisabled("Save to persist.");
		}
		else
		{
			if (pbRenderable)
				ImGui::Checkbox(pRenderableLabel, (bool*)pbRenderable);

			ImGui::TextDisabled("Transform and Renderable affect the current preview.");
			ImGui::TextDisabled("Culling, shadow, and section collision actor are not used by MapEvent_BreakWall.");
		}
	}

	if (nullptr != pBreakWall)
		return;

	const _wstring strSectionKey = CMap_EditFile::Make_SectionKey(strStageName, strSectionName);

	if (ImGui::Button("Apply##SectionEdit"))
	{
		if (!pLevel->Commit_MapEditObjectFromCurrentState(pMapSection))
			MSG_BOX("MAP EDIT APPLY FAILED");
	}

	ImGui::SameLine();

	if (ImGui::Button("Save##SectionEdit"))
	{
		pLevel->Commit_MapEditObjectFromCurrentState(pMapSection);

		if (FAILED(pLevel->Save_MapOverride()))
			MSG_BOX("OBJECT OVERRIDE SAVE FAILED");
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear##SectionEdit"))
	{
		const CMap_EditSession* pSession = pLevel->Get_MapPreviewSession();
		const _int iPresetIndex =
			(nullptr != pSession) ? pSession->Get_EditData().iPresetIndex : -1;

		pLevel->Clear_EditedMapPreviewSection(strSectionKey);

		if (0 <= iPresetIndex)
		{
			pLevel->Load_MapPreviewStage(static_cast<_uint>(iPresetIndex));
			return;
		}
	}
}

void CPanel_Inspector::Draw_MeshLayerPanel(CGameObject* pObject)
{
	if (nullptr == pObject)
		return;

	IEditable* pEditable = dynamic_cast<IEditable*>(pObject);
	EDITABLE_DESC EditDesc{};
	const _bool bEditable = nullptr != pEditable && pEditable->Get_EditDesc(&EditDesc);

	if (bEditable && 0u == (EditDesc.iCapabilities & EDIT_CAP_MESH_LAYER))
		return;

	auto MakeStateKey = [&](const EDITABLE_DESC& Desc) -> _wstring
		{
			return Desc.strStableKey.empty() ? L"ptr|" + to_wstring(reinterpret_cast<size_t>(pObject)) : Desc.strStableKey;
		};

	_uint iSelectedModelSlot = 0u;
	if (bEditable && !EditDesc.ModelSlots.empty())
	{
		_uint& iStoredSlot = m_SelectedModelSlotByEditableKey[MakeStateKey(EditDesc)];
		if (iStoredSlot >= EditDesc.ModelSlots.size())
			iStoredSlot = 0u;
		iSelectedModelSlot = iStoredSlot;
	}

	MESH_LAYER_UI_CONTEXT Ui = Resolve_MeshLayerUIContext(pObject, static_cast<int>(iSelectedModelSlot));
	CModel* pModel = Ui.pModel;
	const _bool bHasEditableModelSlots = nullptr != Ui.pEditable && !Ui.EditDesc.ModelSlots.empty();

	if (nullptr == pModel && !bHasEditableModelSlots)
		return;

	if (!ImGui::CollapsingHeader("Mesh Render Settings (per Model)"))
		return;

	ImGui::TextDisabled("Mesh Render Settings are saved per model sidecar.");
	ImGui::TextDisabled("All objects/sections using this model will be affected.");

	_bool bModelSlotChanged = false;
	if (Ui.pEditable && !Ui.EditDesc.ModelSlots.empty())
	{
		string strItems;
		for (const EDITABLE_MODEL_SLOT& Slot : Ui.EditDesc.ModelSlots)
		{
			strItems += WstrToStr(Slot.strLabel);
			strItems += EDITABLE_MODEL_KIND::ANIM == Slot.eKind ? " (Anim)" : " (NonAnim)";
			strItems.push_back('\0');
		}
		strItems.push_back('\0');

		int iSlot = static_cast<int>(Ui.iModelSlot);
		ImGui::SetNextItemWidth(220.f);
		if (ImGui::Combo("Model Slot##EditableMeshLayer", &iSlot, strItems.c_str()))
		{
			_uint& iStoredSlot = m_SelectedModelSlotByEditableKey[MakeStateKey(Ui.EditDesc)];
			iStoredSlot = static_cast<_uint>(iSlot);
			bModelSlotChanged = true;

			Ui = Resolve_MeshLayerUIContext(pObject, iSlot);
			pModel = Ui.pModel;
			if (nullptr == pModel)
				return;
		}
	}

	auto GetSelectedSlotLabel = [&]() -> string
		{
			if (nullptr == Ui.pEditable || Ui.iModelSlot >= Ui.EditDesc.ModelSlots.size())
				return {};

			const EDITABLE_MODEL_SLOT& Slot = Ui.EditDesc.ModelSlots[Ui.iModelSlot];
			return WstrToStr(Slot.strLabel) + (EDITABLE_MODEL_KIND::ANIM == Slot.eKind ? " (Anim)" : " (NonAnim)");
		};

	auto DrawSaveMeshLayerButton = [&]()
		{
			if (ImGui::Button("Save MeshLayer##EditableModelSlot"))
			{
				if (nullptr == pModel || FAILED(pModel->Save_MeshLayers()))
					MSG_BOX("MESH LAYER SAVE FAILED");
			}

			const string strSlotLabel = GetSelectedSlotLabel();
			if (!strSlotLabel.empty())
			{
				ImGui::SameLine();
				ImGui::TextDisabled("Slot: %s", strSlotLabel.c_str());
			}
		};

	auto MakeMeshFocusKey = [&]() -> _wstring
		{
			if (nullptr != Ui.pEditable)
				return MakeStateKey(Ui.EditDesc) + L"|slot|" + to_wstring(Ui.iModelSlot);

			return L"ptr|" + to_wstring(reinterpret_cast<size_t>(pObject)) + L"|slot|" + to_wstring(Ui.iModelSlot);
		};

	if (bHasEditableModelSlots)
	{
		const EDITABLE_MODEL_SLOT& Slot = Ui.EditDesc.ModelSlots[Ui.iModelSlot];
		if (nullptr == Slot.pModel)
		{
			ImGui::TextDisabled("Selected ModelSlot has no model.");
			return;
		}

		if (0u == Slot.iMeshCount)
		{
			ImGui::TextDisabled("Selected ModelSlot has no mesh.");
			DrawSaveMeshLayerButton();
			return;
		}
	}

	static const char* UvItems[] = { "TEXCOORD0", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3" };

	const size_t iNumMeshes = pModel->Get_NumMeshes();

	const _wstring strMeshFocusKey = MakeMeshFocusKey();
	_int& iFocusedMeshIndex = m_SelectedMeshByEditableSlotKey[strMeshFocusKey];

	_bool bFocusedMeshChanged = bModelSlotChanged;

	if (0 == iNumMeshes)
		iFocusedMeshIndex = -1;
	else if (iFocusedMeshIndex < 0 || iFocusedMeshIndex >= static_cast<_int>(iNumMeshes))
	{
		iFocusedMeshIndex = 0;
		bFocusedMeshChanged = true;
	}

#ifdef _DEBUG
	if (auto* pSection = dynamic_cast<CMapSection*>(pObject))
	{
		if (m_bEditorSoloMesh)
			pSection->Set_EditorSoloMeshIndex(iFocusedMeshIndex);
		else
			pSection->Clear_EditorSoloMesh();
	}
#endif

	if (0 == iNumMeshes)
	{
		ImGui::TextDisabled("No mesh available.");
		DrawSaveMeshLayerButton();
		return;
	}

	ImGui::BeginChild("MeshList", ImVec2(0.f, 140.f), true);
	for (size_t i = 0; i < iNumMeshes; ++i)
	{
		const _uint iMesh = static_cast<_uint>(i);
		const MESH_LAYER_IDX SummaryLayer = pModel->Get_MeshLayer(iMesh);
		const string strMeshName = pModel->Get_MeshName(iMesh);
		const string strLabel = to_string(i) + ": " + strMeshName;
		const _bool bSelected = (iFocusedMeshIndex == static_cast<_int>(i));

		ImGui::PushID(static_cast<int>(i));

		if (ImGui::Selectable(strLabel.c_str(), bSelected, ImGuiSelectableFlags_SpanAllColumns))
		{
			iFocusedMeshIndex = static_cast<_int>(i);
			bFocusedMeshChanged = true;

#ifdef _DEBUG
			if (auto* pSection = dynamic_cast<Client::CMapSection*>(pObject))
			{
				if (m_bEditorSoloMesh)
					pSection->Set_EditorSoloMeshIndex(iFocusedMeshIndex);
			}
#endif
		}

		ImGui::Indent();
		ImGui::TextDisabled("Pass:%d  UV:%s",
			SummaryLayer.iPass,
			SummaryLayer.bUseUVTransform ? "On" : "Off");
		ImGui::Unindent();

		ImGui::Separator();
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::Separator();

	const _uint iMesh = static_cast<_uint>(iFocusedMeshIndex);
	MESH_LAYER_IDX Layer = pModel->Get_MeshLayer(iMesh);

	_bool bChanged = false;
	_bool bAnyField = false;

	const _uint iUnknownType = ETOUI(MTEX_TYPE::UNKNOWN);
	if (bFocusedMeshChanged && Ui.bBushMeshUi && 0u == Layer.idx[iUnknownType])
	{
		const _uint iUnknownTextureCount = pModel->Get_MeshTextureCount(iMesh, MTEX_TYPE::UNKNOWN);
		if (1u < iUnknownTextureCount)
		{
			const _uint iBushDefaultUnknownSlot = (3u < iUnknownTextureCount) ? 3u : (iUnknownTextureCount - 1u);
			Layer.idx[iUnknownType] = iBushDefaultUnknownSlot;
			bChanged = true;
		}
	}

	ImGui::PushID(iFocusedMeshIndex);
	ImGui::Text("Editing Mesh: %d: %s",
		iFocusedMeshIndex,
		pModel->Get_MeshName(iMesh).c_str());

#ifdef _DEBUG
	if (auto* pSection = dynamic_cast<CMapSection*>(pObject))
	{
		if (m_bEditorSoloMesh)
		{
			ImGui::TextDisabled(
				"Solo Mesh: %d",
				pSection->Get_EditorSoloMeshIndex());
		}
	}
#endif

	if (Ui.bMapObjectMeshUi)
	{
		ImGui::SeparatorText("Mode");

		bool bUseLayerEx = Layer.bUseLayerEx;
		if (ImGui::Checkbox("Advanced Layer##UseLayerEx", &bUseLayerEx))
		{
			Layer.bUseLayerEx = bUseLayerEx;
			bChanged = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("Init From Legacy##LayerExInit"))
		{
			InitializeLayerExFromLegacy(Layer);
			bChanged = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("Disable##LayerExDisable"))
		{
			Layer.bUseLayerEx = false;

			for (_uint g = 0; g < MESH_LAYER_EX_GROUP_COUNT; ++g)
			{
				for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
					Layer.LayerEx[g][e] = {};
			}

			bChanged = true;
		}

		if (Layer.bUseLayerEx)
			ImGui::TextDisabled("Advanced Layer is active. Settings are saved per model sidecar.");
	}

	auto DrawUVCombo = [&](const _char* pLabel, _uint& iUVIndex)
		{
			int iValue = (iUVIndex <= 3u) ? static_cast<int>(iUVIndex) : 0;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::Combo(pLabel, &iValue, UvItems, IM_ARRAYSIZE(UvItems)))
			{
				iUVIndex = static_cast<_uint>(iValue);
				bChanged = true;
			}
		};

	auto DrawCompactUVCombo = [&](const _char* pLabel, _uint& iUVIndex)
		{
			int iValue = (iUVIndex <= 3u) ? static_cast<int>(iUVIndex) : 0;
			static const char* CompactUvItems = "0\0""1\0""2\0""3\0\0";

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::Combo(pLabel, &iValue, CompactUvItems))
			{
				iUVIndex = static_cast<_uint>(iValue);
				bChanged = true;
			}
		};

	auto BuildCompactComboItems = [&](int iMinValue, _int iMaxValue)
		{
			string strItems;

			for (int i = iMinValue; i <= iMaxValue; ++i)
			{
				strItems += to_string(i);
				strItems.push_back('\0');
			}

			strItems.push_back('\0');
			return strItems;
		};

	auto DrawMapUVCompactGrid = [&]()
		{
			ImGui::TextUnformatted("UV");

			if (ImGui::BeginTable("MapMeshUVTop", 2, ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("Base", ImGuiTableColumnFlags_WidthStretch, 1.2f);
				ImGui::TableSetupColumn("Unk", ImGuiTableColumnFlags_WidthStretch, 0.4f);
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Base");
				DrawCompactUVCombo("##BaseUV", Layer.iUVIndex);

				ImGui::TableNextColumn();
				ImGui::TextUnformatted("Unk");
				DrawCompactUVCombo("##UnknownUV", Layer.iUnknownUVIndex);

				ImGui::EndTable();
			}

			if (ImGui::BeginTable("MapMeshUVExtra", 4, ImGuiTableFlags_SizingStretchSame))
			{
				const char* Labels[] = { "ExR", "ExG", "ExB", "ExA" };
				const char* Ids[] = { "##ExtraRUV", "##ExtraGUV", "##ExtraBUV", "##ExtraAUV" };
				_uint* Values[] =
				{
						&Layer.iExtraUVIndex[0],
						&Layer.iExtraUVIndex[1],
						&Layer.iExtraUVIndex[2],
						&Layer.iExtraUVIndex[3],
				};

				ImGui::TableNextRow();
				for (int i = 0; i < 4; ++i)
				{
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(Labels[i]);
					DrawCompactUVCombo(Ids[i], *Values[i]);
				}

				ImGui::EndTable();
			}
		};

	auto DrawCompactLayerSlotCell = [&](const _char* pLabel, const _char* pId, MTEX_TYPE eType)
		{
			bAnyField = true;

			const _uint iType = ETOUI(eType);
			_uint& iLayerIndex = Layer.idx[iType];
			const int iCount = static_cast<int>(pModel->Get_MeshTextureCount(iMesh, eType));

			ImGui::TextUnformatted(pLabel);
			ImGui::SetNextItemWidth(-FLT_MIN);

			if (iCount <= 0)
			{
				int iDummy = 0;
				ImGui::BeginDisabled();
				ImGui::Combo(pId, &iDummy, "N/A\0\0");
				ImGui::EndDisabled();
				return;
			}

			if (iLayerIndex >= static_cast<_uint>(iCount))
			{
				iLayerIndex = static_cast<_uint>(iCount - 1);
				bChanged = true;
			}

			if (1 == iCount)
			{
				int iValue = 0;

				if (iLayerIndex != 0u)
				{
					iLayerIndex = 0u;
					bChanged = true;
				}

				ImGui::BeginDisabled();
				ImGui::Combo(pId, &iValue, "0\0\0");
				ImGui::EndDisabled();
				return;
			}

			int iValue = static_cast<int>(iLayerIndex);
			const string strItems = BuildCompactComboItems(0, iCount - 1);
			if (ImGui::Combo(pId, &iValue, strItems.c_str()))
			{
				iLayerIndex = static_cast<_uint>(iValue);
				bChanged = true;
			}
		};

	auto DrawCompactExtraCell = [&](const _char* pLabel, const _char* pIdBase, _int& iBindIndex, _uint& iTexType)
		{
			bAnyField = true;

			static const MTEX_TYPE kTypes[] = { MTEX_TYPE::UNKNOWN, MTEX_TYPE::DIFFUSE };

			ImGui::TextUnformatted(pLabel);

			// --- 소스 타입 콤보 (Unk / Dif) ---
			int iTypeCombo = (iTexType == ETOUI(MTEX_TYPE::DIFFUSE)) ? 1 : 0;
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::Combo((string(pIdBase) + "Type").c_str(), &iTypeCombo, "Unk\0Dif\0\0"))
			{
				iTexType = ETOUI(kTypes[iTypeCombo]);
				bChanged = true;
			}

			const MTEX_TYPE eType = kTypes[iTypeCombo];
			const int iCount = static_cast<int>(pModel->Get_MeshTextureCount(iMesh, eType));

			// --- 슬롯 콤보 (선택 타입 기준 카운트) ---
			ImGui::SetNextItemWidth(-FLT_MIN);

			if (iCount <= 0)
			{
				int iDummy = 0;
				if (iBindIndex != -1) { iBindIndex = -1; bChanged = true; }
				ImGui::BeginDisabled();
				ImGui::Combo((string(pIdBase) + "Slot").c_str(), &iDummy, "N/A\0\0");
				ImGui::EndDisabled();
				return;
			}

			if (iBindIndex < -1) { iBindIndex = -1;          bChanged = true; }
			else if (iBindIndex >= iCount) { iBindIndex = iCount - 1;  bChanged = true; }

			int iComboIndex = iBindIndex + 1;
			const string strItems = BuildCompactComboItems(-1, iCount - 1);
			if (ImGui::Combo((string(pIdBase) + "Slot").c_str(), &iComboIndex, strItems.c_str()))
			{
				iBindIndex = iComboIndex - 1;
				bChanged = true;
			}
		};

	auto DrawLayerExSlotCell = [&](const _char* pLabel, MESH_LAYER_TEX_BIND_EX& Bind, _uint iEntry)
		{
			bAnyField = true;

			ImGui::TextUnformatted(pLabel);

			if (ImGui::Checkbox("##Use", &Bind.bEnable))
				bChanged = true;

			static const MTEX_TYPE kSelectableTypes[] =
			{
				  MTEX_TYPE::DIFFUSE,
				  MTEX_TYPE::METALNESS,
				  MTEX_TYPE::NORMALS,
				  MTEX_TYPE::UNKNOWN
			};

			int iTypeCombo = 0;
			for (int i = 0; i < IM_ARRAYSIZE(kSelectableTypes); ++i)
			{
				if (Bind.iTexType == ETOUI(kSelectableTypes[i]))
				{
					iTypeCombo = i;
					break;
				}
			}

			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::Combo("##TexType", &iTypeCombo, "Diff\0MRA\0Norm\0Ukwn\0\0"))
			{
				Bind.iTexType = ETOUI(kSelectableTypes[iTypeCombo]);
				Bind.iSlot = -1;
				Bind.bEnable = false;
				bChanged = true;
			}

			const MTEX_TYPE eType = static_cast<MTEX_TYPE>(Bind.iTexType);
			const int iCount = static_cast<int>(pModel->Get_MeshTextureCount(iMesh, eType));

			ImGui::SetNextItemWidth(-FLT_MIN);

			if (iCount <= 0)
			{
				int iDummy = 0;
				if (Bind.iSlot != -1) { Bind.iSlot = -1; bChanged = true; }
				ImGui::BeginDisabled();
				ImGui::Combo("##Slot", &iDummy, "N/A\0\0");
				ImGui::EndDisabled();
				return;
			}

			if (Bind.iSlot < -1) { Bind.iSlot = -1; bChanged = true; }
			else if (Bind.iSlot >= iCount) { Bind.iSlot = iCount - 1; bChanged = true; }

			int iComboIndex = Bind.iSlot + 1;
			const string strItems = BuildCompactComboItems(-1, iCount - 1);
			if (ImGui::Combo("##Slot", &iComboIndex, strItems.c_str()))
			{
				Bind.iSlot = iComboIndex - 1;
				Bind.bEnable = Bind.iSlot >= 0;
				bChanged = true;
			}
		};

	auto DrawLayerExTextureGrid = [&](_uint iGroup)
		{
			ImGui::SeparatorText("Advanced Layer Texture");

			if (!ImGui::BeginTable("LayerExTexGrid", 4, ImGuiTableFlags_SizingStretchSame))
				return;

			ImGui::TableNextRow();

			for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(static_cast<int>(iGroup * 10u + e));

				MESH_LAYER_TEX_BIND_EX& Bind = Layer.LayerEx[iGroup][e];
				if (Bind.iTexType == 0u)
					Bind.iTexType = ETOUI(kLayerExTexType[e]);

				DrawLayerExSlotCell(kLayerExEntryName[e], Bind, e);

				ImGui::PopID();
			}

			ImGui::EndTable();
		};

	auto DrawLayerExUVCell = [&](const _char* pLabel, MESH_LAYER_TEX_BIND_EX& Bind)
		{
			bAnyField = true;

			ImGui::TextUnformatted(pLabel);
			DrawCompactUVCombo("##UV", Bind.iUVIndex);

			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::DragFloat2("##Scale", (float*)&Bind.vUVScale, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::DragFloat2("##Offset", (float*)&Bind.vUVOffset, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::DragFloat("##Rotate", &Bind.fUVRotate, 0.01f))
				bChanged = true;

			if (ImGui::Button("Reset UV"))
			{
				Bind.iUVIndex = 0u;
				Bind.vUVScale = XMFLOAT2{ 1.f, 1.f };
				Bind.vUVOffset = XMFLOAT2{ 0.f, 0.f };
				Bind.fUVRotate = 0.f;
				bChanged = true;
			}
		};
	
	auto DrawLayerExUVGrid = [&](_uint iGroup)
		{
			ImGui::SeparatorText("Advanced Layer UV");

			if (!ImGui::BeginTable("LayerExUVGrid", 4, ImGuiTableFlags_SizingStretchSame))
				return;

			ImGui::TableNextRow();

			for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
			{
				ImGui::TableNextColumn();
				ImGui::PushID(static_cast<int>(100 + iGroup * 10u + e));

				MESH_LAYER_TEX_BIND_EX& Bind = Layer.LayerEx[iGroup][e];
				DrawLayerExUVCell(kLayerExEntryName[e], Bind);

				ImGui::PopID();
			}

			ImGui::EndTable();
		};

	auto DrawLayerExGroupSummary = [&](_uint iGroup)
		{
			_uint iEnabledCount = 0;
			_uint iSlotCount = 0;

			for (_uint e = 0; e < MESH_LAYER_EX_ENTRY_COUNT; ++e)
			{
				const MESH_LAYER_TEX_BIND_EX& Bind = Layer.LayerEx[iGroup][e];

				if (Bind.bEnable)
					++iEnabledCount;

				if (Bind.iSlot >= 0)
					++iSlotCount;
			}

			ImGui::TextDisabled(
				"LayerEx %s: enabled %u / slots %u. Saved per model sidecar.",
				kLayerExGroupName[iGroup],
				iEnabledCount,
				iSlotCount);
		};

	auto DrawMapTextureCompactGrid = [&]()
		{
			ImGui::TextUnformatted("Tex");

			if (!ImGui::BeginTable("MapMeshTexGrid", 4, ImGuiTableFlags_SizingStretchSame))
				return;

			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("Dif", "##TexDiffuse", MTEX_TYPE::DIFFUSE);

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("Nrm", "##TexNormal", MTEX_TYPE::NORMALS);

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("MRA", "##TexMRA", MTEX_TYPE::METALNESS);

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("Unk", "##TexUnknown", MTEX_TYPE::UNKNOWN);

			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			DrawCompactExtraCell("ExR", "##ExR", Layer.iExtraBind[0], Layer.iExtraTexType[0]);

			ImGui::TableNextColumn();
			DrawCompactExtraCell("ExG", "##ExG", Layer.iExtraBind[1], Layer.iExtraTexType[1]);

			ImGui::TableNextColumn();
			DrawCompactExtraCell("ExB", "##ExB", Layer.iExtraBind[2], Layer.iExtraTexType[2]);

			ImGui::TableNextColumn();
			DrawCompactExtraCell("ExA", "##ExA", Layer.iExtraBind[3], Layer.iExtraTexType[3]);

			ImGui::EndTable();
		};

	if (Ui.bWorldPassMeshUi)
	{
		int iPassCombo = Get_WorldShaderPassComboIndex(Layer.iPass);

		if (ImGui::Combo("Pass",
			&iPassCombo,
			GetWorldShaderPassComboItem,
			nullptr,
			static_cast<int>(_countof(g_WorldShaderPassMetas))))
		{
			Layer.iPass = Get_WorldShaderPassFromComboIndex(iPassCombo);
			bChanged = true;
		}

		ImGui::SeparatorText("Constant Material");

		const ImGuiColorEditFlags iColorEditFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;

		ImGui::SetNextItemWidth(240.f);
		if (ImGui::ColorEdit4("Render Color##MeshLayer", (float*)&Layer.vRenderColor, iColorEditFlags))
		{
			bChanged = true;
		}

		ImGui::SetNextItemWidth(240.f);
		if (ImGui::ColorEdit3("Emissive Color##MeshLayer", (float*)&Layer.vEmissiveColor, iColorEditFlags))
		{
			bChanged = true;
		}

		ImGui::SetNextItemWidth(240.f);
		if (ImGui::ColorEdit3("MRA (M/R/A)##MeshLayer", (float*)&Layer.vMRA, iColorEditFlags))
		{
			bChanged = true;
		}

		if (Ui.bEnvObjectMeshUi)
			ImGui::TextDisabled("Dither is controlled per object in EnvObject Edit.");
		else
			ImGui::TextDisabled("LevelDesign uses WORLD_PASS domain.");
	}
	else if (Ui.bMapObjectMeshUi)
	{
		int iPassCombo = Get_MapShaderPassComboIndex(Layer.iPass);

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::Combo("Pass",
			&iPassCombo,
			GetMapShaderPassComboItem,
			nullptr,
			static_cast<int>(_countof(g_MapShaderPassMetas))))
		{
			Layer.iPass = Get_MapShaderPassFromComboIndex(iPassCombo);
			bChanged = true;
		}

		if (Layer.bUseLayerEx)
		{
			if (ImGui::BeginTabBar("LayerExGroups"))
			{
				for (_uint g = 0; g < MESH_LAYER_EX_GROUP_COUNT; ++g)
				{
					if (ImGui::BeginTabItem(kLayerExGroupName[g]))
					{
						DrawLayerExTextureGrid(g);
						DrawLayerExUVGrid(g);
						DrawLayerExGroupSummary(g);
						ImGui::EndTabItem();
					}
				}

				ImGui::EndTabBar();
			}

			ImGui::SeparatorText("Advanced Strength");

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("Normal Strength##LayerEx", &Layer.fNormalStrength, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("Mask Strength##LayerEx", &Layer.fMaskStrength, 0.01f))
				bChanged = true;
		}
		else
		{
			ImGui::TextUnformatted("UV Slots");
			DrawMapUVCompactGrid();

			if (ImGui::Checkbox("Use UV Transform", (bool*)&Layer.bUseUVTransform))
				bChanged = true;

			ImGui::BeginDisabled(!Layer.bUseUVTransform);

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat2("UV Scale", (float*)&Layer.vUVScale, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat2("UV ScaleNormal", (float*)&Layer.vUVScaleNormal, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat2("UV ScaleMaterial", (float*)&Layer.vUVScaleMaterial, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("UV Rotate", &Layer.fUVRotate, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat2("UV Offset", (float*)&Layer.vUVOffset, 0.01f))
				bChanged = true;

			ImGui::EndDisabled();

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("Normal Strength", &Layer.fNormalStrength, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("Mask Strength", &Layer.fMaskStrength, 0.01f))
				bChanged = true;

			ImGui::Spacing();
			DrawMapUVCompactGrid();
			ImGui::Spacing();
			DrawMapTextureCompactGrid();
		}
	}
	else
	{
		int iPass = Layer.iPass;
		ImGui::SetNextItemWidth(120.f);
		if (ImGui::InputInt("Pass", &iPass))
		{
			if (iPass < -1)
				iPass = -1;

			Layer.iPass = iPass;
			bChanged = true;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("(-1 = default)");
	}

	if (!Ui.bMapObjectMeshUi)
	{
		ImGui::SeparatorText("UV");

		DrawUVCombo("Base UV", Layer.iUVIndex);

		if (Ui.bWorldPassMeshUi)
			DrawUVCombo("Unknown UV", Layer.iUnknownUVIndex);

		if (ImGui::Checkbox("Use UV Transform", (bool*)&Layer.bUseUVTransform))
			bChanged = true;

		ImGui::BeginDisabled(!Layer.bUseUVTransform);

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::DragFloat2("UV Scale", (float*)&Layer.vUVScale, 0.01f))
			bChanged = true;

		if (Ui.bWorldPassMeshUi)
		{
			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat2("UV ScaleNormal", (float*)&Layer.vUVScaleNormal, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat2("UV ScaleMaterial", (float*)&Layer.vUVScaleMaterial, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("UV Rotate", &Layer.fUVRotate, 0.01f))
				bChanged = true;
		}

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::DragFloat2("UV Offset", (float*)&Layer.vUVOffset, 0.01f))
			bChanged = true;

		ImGui::EndDisabled();

		if (Ui.bWorldPassMeshUi)
		{
			ImGui::SeparatorText("Strength");

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("Normal Strength", &Layer.fNormalStrength, 0.01f))
				bChanged = true;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::DragFloat("Mask Strength", &Layer.fMaskStrength, 0.01f))
				bChanged = true;
		}

		ImGui::SeparatorText("Texture Slots");

		if (ImGui::BeginTable("WorldMeshTexGrid", 4, ImGuiTableFlags_SizingStretchSame))
		{
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("Dif", "##WorldTexDiffuse", MTEX_TYPE::DIFFUSE);

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("Nrm", "##WorldTexNormal", MTEX_TYPE::NORMALS);

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("MRA", "##WorldTexMRA", MTEX_TYPE::METALNESS);

			ImGui::TableNextColumn();
			DrawCompactLayerSlotCell("Unk", "##WorldTexUnknown", MTEX_TYPE::UNKNOWN);

			ImGui::EndTable();
		}
	}

	if (!bAnyField)
		ImGui::TextDisabled("  (no texture slot override)");

	if (bChanged)
	{
		if (nullptr != Ui.pEditable)
		{
			if (FAILED(Ui.pEditable->Apply_EditMeshLayer(Ui.iModelSlot, iMesh, Layer)))
				MSG_BOX("MESH LAYER APPLY FAILED");
		}
		else
		{
			pModel->Set_MeshLayer(iMesh, Layer);
		}
	}

	ImGui::Separator();
	ImGui::PopID();

	DrawSaveMeshLayerButton();
}

void CPanel_Inspector::Draw_MapStageSections(Client::CMapStage* pMapStage)
{
	if (nullptr == pMapStage)
		return;

	CLevel_Edit* pLevel = CEditInstance::GetInstance()->Get_Level();
	if (nullptr == pLevel)
		return;

	const auto& Sections = pMapStage->Get_Sections();

	ImGui::Separator();
	ImGui::Text("Sections (%d)", static_cast<int>(Sections.size()));

	_bool bFocusedStillValid = false;
	for (Client::CMapSection* pSection : Sections)
	{
		if (pSection == m_pFocusedMapSection)
		{
			bFocusedStillValid = true;
			break;
		}
	}

	if (!bFocusedStillValid)
	{
		m_pFocusedMapSection = nullptr;

		for (Client::CMapSection* pSection : Sections)
		{
			if (nullptr != pSection)
			{
				m_pFocusedMapSection = pSection;
				break;
			}
		}
	}

	const ImVec2 vAvail = ImGui::GetContentRegionAvail();
	float fListWidth = vAvail.x * 0.18f;
	if (fListWidth < 140.f)
		fListWidth = 140.f;
	else if (fListWidth > 190.f)
		fListWidth = 190.f;

	ImGui::BeginChild("SectionList", ImVec2(fListWidth, 0.f), true);
	{
		for (Client::CMapSection* pSection : Sections)
		{
			if (nullptr == pSection)
				continue;

			ImGui::PushID(pSection);

			const string strName = WstrToStr(pSection->Get_SectionName());
			const string strLabel = strName.empty() ? "<Unnamed Section>" : strName;
			const _bool bSelected = (pSection == m_pFocusedMapSection);

			if (ImGui::Selectable(strLabel.c_str(), bSelected, ImGuiSelectableFlags_SpanAllColumns))
			{
				m_pFocusedMapSection = pSection;

#ifdef _DEBUG
				if (nullptr != pMapStage)
				{
					if (m_bEditorSoloSection)
						pMapStage->Set_EditorSoloSection(m_pFocusedMapSection);
					else
						pMapStage->Clear_EditorSoloSection();
				}

				if (nullptr != m_pFocusedMapSection)
				{
					if (m_bEditorSoloMesh)
						m_pFocusedMapSection->Set_EditorSoloMeshIndex(-1);
					else
						m_pFocusedMapSection->Clear_EditorSoloMesh();
				}
#endif
			}

			const _bool bRenderable = ReadBoolProperty(pSection, L"Renderable", L"MapSection", pSection->Get_Desc().bRenderable);
			const _bool bEnableCulling = ReadBoolProperty(pSection, L"Enable Culling", L"MapSection", pSection->Get_Desc().bEnableCulling);
			const _bool bUseCollMesh = pSection->Has_CollMesh() && pSection->Is_UseCollMesh();

			ImGui::TextDisabled("R:%s  C:%s",
				bRenderable ? "On" : "Off",
				bEnableCulling ? "On" : "Off");
			ImGui::TextDisabled("Shadow: Forced  CollMesh:%s",
				bUseCollMesh ? "On" : "Off");

			ImGui::Separator();
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("SectionDetail", ImVec2(0.f, 0.f), true);
	{
		if (nullptr == m_pFocusedMapSection)
		{
			ImGui::TextDisabled("No section available.");
		}
		else
		{
			const string strName = WstrToStr(m_pFocusedMapSection->Get_SectionName());

			ImGui::Text("Selected Section: %s", strName.empty() ? "<Unnamed Section>" : strName.c_str());
			ImGui::Separator();

			ImGui::PushID(m_pFocusedMapSection);

			const _bool bTransformChanged = Draw_Transform(m_pFocusedMapSection, strName);
			if (bTransformChanged)
				m_pFocusedMapSection->On_EditTransformChanged();

			ImGui::Separator();
			Draw_EditableObjectPolicyPanel(m_pFocusedMapSection);

			ImGui::Separator();
			Draw_MeshLayerPanel(m_pFocusedMapSection);

			ImGui::Separator();
			Draw_MapSectionEditPanel(pLevel, pMapStage, m_pFocusedMapSection);
			ImGui::Separator();
#ifdef _DEBUG
			Draw_MapSectionViewFilter(pMapStage, m_pFocusedMapSection, -1);
			ImGui::Separator();
#endif
			Draw_Properties(m_pFocusedMapSection);
			ImGui::Separator();
			Draw_MapSectionRenderOptions(m_pFocusedMapSection);

			ImGui::PopID();
		}
	}
	ImGui::EndChild();
}

void CPanel_Inspector::Draw_MapSectionRenderOptions(Client::CMapSection* pSection)
{
	if (nullptr == pSection)
		return;

	if (!ImGui::CollapsingHeader("Render Group (per Section)", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	int iRenderGroup = ToMapRenderGroupIndex(pSection->Get_RenderID());
	const char* RenderGroups[] = { "NONBLEND", "BLEND" };

	if (ImGui::Combo("Render Group", &iRenderGroup, RenderGroups, IM_ARRAYSIZE(RenderGroups)))
		pSection->Set_RenderID(FromMapRenderGroupIndex(iRenderGroup));
}

#ifdef _DEBUG
void CPanel_Inspector::Draw_MapSectionViewFilter(CMapStage* pMapStage, CMapSection* pSection, _int iSelectedMeshIndex)
{
	if (nullptr == pMapStage || nullptr == pSection)
		return;

	if (!ImGui::CollapsingHeader("View Filter", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	if (ImGui::Checkbox("Solo Section", (bool*)&m_bEditorSoloSection))
	{
		if (m_bEditorSoloSection)
			pMapStage->Set_EditorSoloSection(pSection);
		else
			pMapStage->Clear_EditorSoloSection();
	}

	if (ImGui::Checkbox("Solo Mesh", (bool*)&m_bEditorSoloMesh))
	{
		if (!m_bEditorSoloMesh)
			pMapStage->Clear_EditorSoloMeshAllSections();
		// ON인 경우 실제 index는 MeshList 선택 로직에서 즉시 세팅
	}

	ImGui::TextDisabled("Solo Mesh uses the focused mesh in Mesh Render Settings.");
}
#endif

_bool* CPanel_Inspector::Resolve_EnvNearAlphaEditState(CLevel_Edit* pLevel, CEnvObject* pEnvObject)
{
	if (nullptr == pLevel || nullptr == pEnvObject)
		return nullptr;

	auto Iter = m_EnvNearAlphaEditStates.find(pEnvObject);
	if (Iter == m_EnvNearAlphaEditStates.end())
	{
		EDIT_OBJECT_OVERRIDE_DESC SavedEdit{};
		EDIT_ENVOBJECT_OVERRIDE* pSavedEnvOverride = nullptr;
		const _bool bUseNearDistAlpha =
			pLevel->Try_GetMapPreviewEnvEdit(pEnvObject, &SavedEdit)
			&& nullptr != (pSavedEnvOverride = get_if<EDIT_ENVOBJECT_OVERRIDE>(&SavedEdit.ClassOverride))
			&& pSavedEnvOverride->bHasNearDistAlpha
			? pSavedEnvOverride->bUseNearDistAlpha
			: pEnvObject->Get_Desc().tRender.bUseNearDistAlpha;

		Iter = m_EnvNearAlphaEditStates.emplace(
			static_cast<CGameObject*>(pEnvObject),
			bUseNearDistAlpha).first;
	}

	return &Iter->second;
}

void CPanel_Inspector::Clear_EnvNearAlphaEditState(CGameObject* pObject)
{
	if (nullptr != pObject)
		m_EnvNearAlphaEditStates.erase(pObject);
}

CPanel_Inspector* CPanel_Inspector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CPanel_Inspector(pDevice, pContext);
}

void CPanel_Inspector::Free()
{
	__super::Free();
}
