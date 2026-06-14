#include "Panel_Inspector.h"
#include "EditInstance.h"
#include "Level_Edit.h"
#include "Map_EditSession.h"

#include "MapStage.h"
#include "MapSection.h"
#include "Map_EditFile.h"
#include "EnvObject.h"

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
	const char* TexTypeName(_uint t)
	{
		static const char* names[MTEX_TYPE_MAX] = {
				"None","Diffuse","Specular","Ambient","Emissive","Height","Normals","Shininess",
				"Opacity","Displacement","Lightmap","Reflection","BaseColor","NormalCamera",
				"EmissionColor","Metalness(MRA)","Roughness","AO","Unknown(Mask)","Sheen","Clearcoat",
				"Transmission","MayaBase","MayaSpecular","MayaSpecColor","MayaSpecRough","Anisotropy"
		};

		return (t < MTEX_TYPE_MAX) ? names[t] : "?";
	}

	int ToMapRenderGroupIndex(RENDERID eRenderID)
	{
		return (eRenderID == RENDERID::BLEND) ? 1 : 0;
	}

	RENDERID FromMapRenderGroupIndex(int iIndex)
	{
		return (iIndex == 1) ? RENDERID::BLEND : RENDERID::NONBLEND;
	}

	_bool* FindBoolProperty(IReflectable* pHolder, const _wstring& strName, const _wstring&
		strCategory)
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

	void Fill_EditWorldMatrix(CGameObject* pObject, MAP_ENV_EDITED_DESC* pOutEdit)
	{
		if (nullptr == pObject || nullptr == pOutEdit)
			return;

		pOutEdit->bHasWorldMatrix = true;
		pOutEdit->matWorld = *pObject->Get_Transform()->Get_WorldMatrixPtr();
	}

	_bool Has_AnyMapEnvEdit(const MAP_ENV_EDITED_DESC& Edit)
	{
		return Edit.bHasRenderable
			|| Edit.bHasEnableCulling
			|| Edit.bHasCastShadow
			|| Edit.bHasWorldMatrix
			|| Edit.bDisableCollisionMesh;
	}

	_matrix Build_EnvBaseWorldMatrix(const ENV_OBJECT_DESC& Desc)
	{
		if (Desc.bHasWorldMatrix)
			return XMLoadFloat4x4(&Desc.matWorld);

		const _vector vScale = XMLoadFloat3(&Desc.vScale);
		const _vector vRotation = XMLoadFloat4(&Desc.vRotation);
		const _vector vPosition = XMVectorSet(
			Desc.vPosition.x,
			Desc.vPosition.y,
			Desc.vPosition.z,
			1.f);

		return XMMatrixScalingFromVector(vScale)
			* XMMatrixRotationQuaternion(vRotation)
			* XMMatrixTranslationFromVector(vPosition);
	}

	_bool IsNearlyEqualFloat4x4(
		const _float4x4& A,
		const _float4x4& B,
		_float fEpsilon = 0.0001f)
	{
		for (_uint iRow = 0; iRow < 4; ++iRow)
		{
			for (_uint iCol = 0; iCol < 4; ++iCol)
			{
				if (fabsf(A.m[iRow][iCol] - B.m[iRow][iCol]) > fEpsilon)
					return false;
			}
		}

		return true;
	}

	MAP_ENV_EDITED_DESC Build_EnvEditFromCurrentObject(Client::CEnvObject* pEnvObject)
	{
		MAP_ENV_EDITED_DESC Edit{};
		if (nullptr == pEnvObject)
			return Edit;

		const ENV_OBJECT_DESC& Desc = pEnvObject->Get_Desc();

		const _bool bRenderable =
			ReadBoolProperty(pEnvObject, L"Renderable", L"EnvObject", true);
		const _bool bBaseRenderable = !Desc.tCollision.bInvisibleCollision;
		if (bRenderable != bBaseRenderable)
		{
			Edit.bHasRenderable = true;
			Edit.bRenderable = bRenderable;
		}

		const _bool bEnableCulling =
			ReadBoolProperty(pEnvObject, L"Enable Culling", L"EnvObject", true);
		const _bool bBaseEnableCulling = Desc.tRender.bUseLodCulling;
		if (bEnableCulling != bBaseEnableCulling)
		{
			Edit.bHasEnableCulling = true;
			Edit.bEnableCulling = bEnableCulling;
		}

		const _bool bCastShadow =
			ReadBoolProperty(pEnvObject, L"Cast Shadow", L"EnvObject", false);
		const _bool bBaseCastShadow = Desc.tRender.bShadowMappingCaster;
		if (bCastShadow != bBaseCastShadow)
		{
			Edit.bHasCastShadow = true;
			Edit.bCastShadow = bCastShadow;
		}

		_float4x4 BaseWorld = {};
		XMStoreFloat4x4(&BaseWorld, Build_EnvBaseWorldMatrix(Desc));

		const _float4x4& CurrentWorld =
			*pEnvObject->Get_Transform()->Get_WorldMatrixPtr();

		if (!IsNearlyEqualFloat4x4(CurrentWorld, BaseWorld))
		{
			Edit.bHasWorldMatrix = true;
			Edit.matWorld = CurrentWorld;
		}

		return Edit;
	}

	MAP_ENV_EDITED_DESC Build_SectionEditFromCurrentSection(Client::CMapSection* pSection)
	{
		MAP_ENV_EDITED_DESC Edit{};
		if (nullptr == pSection)
			return Edit;

		Edit.bHasRenderable = true;
		Edit.bRenderable = ReadBoolProperty(pSection, L"Renderable", L"MapSection", true);

		Edit.bHasEnableCulling = true;
		Edit.bEnableCulling = ReadBoolProperty(pSection, L"Enable Culling", L"MapSection", true);

		Edit.bHasCastShadow = true;
		Edit.bCastShadow = ReadBoolProperty(pSection, L"Cast Shadow", L"MapSection", false);

		Fill_EditWorldMatrix(pSection, &Edit);
		return Edit;
	}
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

	Draw_Transform(pSelected);

	if (dynamic_cast<Client::CEnvObject*>(pSelected))
	{
		ImGui::Separator();
		Draw_EnvObjectEditPanel(pLevel, pSelected);
	}

	ImGui::Separator();
	Draw_MeshLayerPanel(pSelected);
	ImGui::Separator();
	ImGui::Separator();
	Draw_Properties(pSelected);

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

void CPanel_Inspector::Draw_Properties(IReflectable* pHolder)
{
	string strCurrentCategory = {};

	const _bool bSkipEnvObjectCategory =
		nullptr != dynamic_cast<Client::CEnvObject*>(pHolder);
	const _bool bSkipMapSectionCategory =
		nullptr != dynamic_cast<Client::CMapSection*>(pHolder);

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
			ImGui::Text(strPropName.c_str());
			ImGui::InputInt(("##" + strPropName).c_str(), (int*)pData);
			break;
		case PROP_TYPE::FLOAT:
			ImGui::Text(strPropName.c_str());
			ImGui::DragFloat(("##" + strPropName).c_str(), (float*)pData, 0.1f);
			break;
		case PROP_TYPE::BOOL:
			ImGui::Text(strPropName.c_str());
			ImGui::Checkbox(("##" + strPropName).c_str(), (bool*)pData);
			break;
		case PROP_TYPE::FLOAT2:
			ImGui::Text(strPropName.c_str());
			ImGui::DragFloat2(("##" + strPropName).c_str(), (float*)pData, 0.1f);
			break;
		case PROP_TYPE::FLOAT3:
			ImGui::Text(strPropName.c_str());
			ImGui::DragFloat3(("##" + strPropName).c_str(), (float*)pData, 0.1f);
			break;
		case PROP_TYPE::FLOAT4:
			ImGui::Text(strPropName.c_str());
			ImGui::DragFloat4(("##" + strPropName).c_str(), (float*)pData, 0.1f);
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
				*(_uint*)pData = (_uint)iVal;
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
				*pWstr = StrToWstr(buf);
			break;
		}
		}
	}
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
			pTransform->Set_Scale(vUIScale.x, vUIScale.y, 1.f);

		if (ImGui::DragFloat(("Uniform Scale##" + strSuffix).c_str(), &fRatioScale, 0.01f, 0.01f, 100.f))
			pTransform->Set_Scale(fBaseScaleX * fRatioScale, fBaseScaleY * fRatioScale, 1.f);
	}
	else
	{
		_float fUniformScale = vScale.x;
		if (ImGui::DragFloat(("Scale##" + strSuffix).c_str(), &fUniformScale, 0.1f))
			pTransform->Set_Scale(fUniformScale, fUniformScale, fUniformScale);
	}

	return bChanged;
}

void CPanel_Inspector::Draw_EnvObjectEditPanel(CLevel_Edit* pLevel, CGameObject* pObject)
{
	Client::CEnvObject* pEnvObject = dynamic_cast<Client::CEnvObject*>(pObject);
	if (nullptr == pLevel || nullptr == pEnvObject)
		return;

	_bool* pbRenderable = FindBoolProperty(pEnvObject, L"Renderable", L"EnvObject");
	_bool* pbEnableCulling = FindBoolProperty(pEnvObject, L"Enable Culling", L"EnvObject");
	_bool* pbCastShadow = FindBoolProperty(pEnvObject, L"Cast Shadow", L"EnvObject");
	_bool* pbDisableCollisionMesh = Resolve_EnvCollisionEditState(pLevel, pEnvObject);

	ImGui::TextUnformatted("EnvObject Edit");

	if (pbRenderable)
		ImGui::Checkbox("Renderable##EnvEdit", (bool*)pbRenderable);
	if (pbEnableCulling)
		ImGui::Checkbox("Enable Culling##EnvEdit", (bool*)pbEnableCulling);
	if (pbCastShadow)
		ImGui::Checkbox("Cast Shadow##EnvEdit", (bool*)pbCastShadow);
	if (pbDisableCollisionMesh)
		ImGui::Checkbox("Disable Collision On Reload##EnvEdit", (bool*)pbDisableCollisionMesh);

	ImGui::TextDisabled("Applied on next env reload.");
	ImGui::TextDisabled("Restart persistence requires Save Override Now or toolbar Map Edit Save.");

	if (ImGui::Button("Apply Current##EnvEdit"))
	{
		MAP_ENV_EDITED_DESC Edit = Build_EnvEditFromCurrentObject(pEnvObject);

		const _bool bBaseDisableCollisionMesh =
			pEnvObject->Get_Desc().tCollision.bInvalidCollision;
		const _bool bDisableCollisionMesh =
			(nullptr != pbDisableCollisionMesh)
			? *pbDisableCollisionMesh
			: bBaseDisableCollisionMesh;

		Edit.bDisableCollisionMesh =
			(!bBaseDisableCollisionMesh && bDisableCollisionMesh);

		if (Has_AnyMapEnvEdit(Edit))
		{
			pLevel->Track_EditedMapPreviewEnvObject(pObject, Edit);
		}
		else
		{
			pLevel->Clear_EditedMapPreviewEnvObject(pObject);
			Clear_EnvCollisionEditState(pObject);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Save Override Now##EnvEdit"))
	{
		if (FAILED(pLevel->Save_MapOverride()))
			MSG_BOX("MAP EDIT SAVE FAILED");
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear Saved Edit##EnvEdit"))
	{
		const CMap_EditSession* pSession = pLevel->Get_MapPreviewSession();
		const _int iPresetIndex =
			(nullptr != pSession) ? pSession->Get_EditData().iPresetIndex : -1;

		pLevel->Clear_EditedMapPreviewEnvObject(pObject);
		m_EnvCollisionEditStates.clear();

		if (0 <= iPresetIndex)
		{
			pLevel->Load_MapPreviewEnv(static_cast<_uint>(iPresetIndex));
			return;
		}
	}
}

void CPanel_Inspector::Draw_MapSectionEditPanel(CLevel_Edit* pLevel, CMapStage* pMapStage,
	CMapSection* pSection)
{
	if (nullptr == pLevel || nullptr == pMapStage || nullptr == pSection)
		return;

	_bool* pbRenderable = FindBoolProperty(pSection, L"Renderable", L"MapSection");
	_bool* pbEnableCulling = FindBoolProperty(pSection, L"Enable Culling", L"MapSection");
	_bool* pbCastShadow = FindBoolProperty(pSection, L"Cast Shadow", L"MapSection");

	const _wstring strSectionKey = CMap_EditFile::Make_SectionKey(
		pMapStage->Get_StageName(),
		pSection->Get_SectionName());

	_bool* pbCreateCollisionActor =
		Resolve_SectionCollisionEditState(pLevel, pMapStage, pSection);

	ImGui::TextUnformatted("MapSection Edit");

	if (pbRenderable)
		ImGui::Checkbox("Renderable##SectionEdit", (bool*)pbRenderable);
	if (pbEnableCulling)
		ImGui::Checkbox("Enable Culling##SectionEdit", (bool*)pbEnableCulling);
	if (pbCastShadow)
		ImGui::Checkbox("Cast Shadow##SectionEdit", (bool*)pbCastShadow);
	if (pbCreateCollisionActor)
		ImGui::Checkbox("Create Collision Actor On Reload##SectionEdit", (bool*)pbCreateCollisionActor);

	ImGui::TextDisabled("Applied on next stage reload.");
	ImGui::TextDisabled("Restart persistence requires Save Override Now or toolbar Map Edit Save.");

	if (ImGui::Button("Apply Current##SectionEdit"))
	{
		MAP_ENV_EDITED_DESC Edit = Build_SectionEditFromCurrentSection(pSection);

		const _bool bCreateCollisionActor =
			(nullptr != pbCreateCollisionActor) ? *pbCreateCollisionActor : true;

		Edit.bDisableCollisionMesh = !bCreateCollisionActor;
		pSection->Set_CollisionActorEnabled(bCreateCollisionActor);

		pLevel->Track_EditedMapPreviewSection(strSectionKey, Edit);
	}

	ImGui::SameLine();

	if (ImGui::Button("Save Override Now##SectionEdit"))
	{
		if (FAILED(pLevel->Save_MapOverride()))
			MSG_BOX("MAP EDIT SAVE FAILED");
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear Saved Edit##SectionEdit"))
	{
		const CMap_EditSession* pSession = pLevel->Get_MapPreviewSession();
		const _int iPresetIndex =
			(nullptr != pSession) ? pSession->Get_EditData().iPresetIndex : -1;

		pLevel->Clear_EditedMapPreviewSection(strSectionKey);
		m_SectionCollisionEditStates.clear();

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

	CModel* pModel = pObject->Get_Component<CModel>(L"Com_Model");
	if (nullptr == pModel)
		return;

	if (!ImGui::CollapsingHeader("Mesh Render Settings (per Model)"))
		return;

	const size_t iNumMeshes = pModel->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; ++i)
	{
		MESH_LAYER_IDX Layer = pModel->Get_MeshLayer((_uint)i);

		ImGui::PushID((int)i);
		ImGui::Text("%zu: %s", i, pModel->Get_MeshName((_uint)i).c_str());

		_bool bChanged = false;
		_bool bAnyField = false;

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

		for (_uint t = 0; t < MTEX_TYPE_MAX; ++t)
		{
			int iCount = (int)pModel->Get_MeshTextureCount((_uint)i, (MTEX_TYPE)t);
			if (iCount <= 1)
				continue;

			bAnyField = true;

			int iValue = (int)Layer.idx[t];
			ImGui::SetNextItemWidth(120.f);
			if (ImGui::InputInt(TexTypeName(t), &iValue))
			{
				if (iValue < 0)
					iValue = 0;
				if (iValue >= iCount)
					iValue = iCount - 1;

				Layer.idx[t] = (_uint)iValue;
				bChanged = true;
			}
			ImGui::SameLine();
			ImGui::Text("/ %d", iCount);
		}

		if (!bAnyField)
			ImGui::TextDisabled("  (no texture slot override)");

		if (bChanged)
			pModel->Set_MeshLayer((_uint)i, Layer);

		ImGui::Separator();
		ImGui::PopID();
	}

	if (ImGui::Button("Bake (Save sidecar)"))
	{
		if (FAILED(pModel->Save_MeshLayers()))
			MSG_BOX("MESH LAYER SAVE FAILED");
	}
}

void CPanel_Inspector::Draw_MapStageSections(Client::CMapStage* pMapStage)
{
	if (nullptr == pMapStage)
		return;

	const auto& Sections = pMapStage->Get_Sections();

	ImGui::Separator();
	ImGui::Text("Sections (%d)", (int)Sections.size());

	for (Client::CMapSection* pSection : Sections)
	{
		if (nullptr == pSection)
			continue;

		string strName = WstrToStr(pSection->Get_SectionName());
		string strHeader = strName + "##Section_" + to_string((uintptr_t)pSection);
		CLevel_Edit* pLevel = CEditInstance::GetInstance()->Get_Level();

		if (ImGui::CollapsingHeader(strHeader.c_str()))
		{
			ImGui::PushID(pSection);

			const _bool bTransformChanged = Draw_Transform(pSection, strName);
			if (bTransformChanged)
				pSection->Notify_EditTransformChanged();

			ImGui::Separator();
			Draw_MapSectionEditPanel(pLevel, pMapStage, pSection);
			ImGui::Separator();
			Draw_Properties(pSection);
			ImGui::Separator();
			Draw_MeshLayerPanel(pSection);
			ImGui::Separator();
			Draw_MapSectionRenderOptions(pSection);

			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::Separator();
	}
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

_bool* CPanel_Inspector::Resolve_EnvCollisionEditState(CLevel_Edit* pLevel, Client::CEnvObject* pEnvObject)
{
	if (nullptr == pLevel || nullptr == pEnvObject)
		return nullptr;

	auto Iter = m_EnvCollisionEditStates.find(pEnvObject);
	if (Iter == m_EnvCollisionEditStates.end())
	{
		MAP_ENV_EDITED_DESC SavedEdit{};
		const _bool bDisableCollisionMesh =
			pLevel->Try_GetMapPreviewEnvEdit(pEnvObject, &SavedEdit)
			? SavedEdit.bDisableCollisionMesh
			: pEnvObject->Get_Desc().tCollision.bInvalidCollision;

		Iter = m_EnvCollisionEditStates.emplace(
			static_cast<CGameObject*>(pEnvObject),
			bDisableCollisionMesh).first;
	}

	return &Iter->second;
}

_bool* CPanel_Inspector::Resolve_SectionCollisionEditState(CLevel_Edit* pLevel, CMapStage* pMapStage, CMapSection* pSection)
{
	if (nullptr == pLevel || nullptr == pMapStage || nullptr == pSection)
		return nullptr;

	auto Iter = m_SectionCollisionEditStates.find(pSection);
	if (Iter == m_SectionCollisionEditStates.end())
	{
		const _wstring strSectionKey = CMap_EditFile::Make_SectionKey(
			pMapStage->Get_StageName(),
			pSection->Get_SectionName());

		MAP_ENV_EDITED_DESC SavedEdit{};
		const _bool bCreateCollisionActor =
			pLevel->Try_GetMapPreviewSectionEdit(strSectionKey, &SavedEdit)
			? !SavedEdit.bDisableCollisionMesh
			: pSection->Is_CollisionActorEnabled();

		Iter = m_SectionCollisionEditStates.emplace(
			pSection,
			bCreateCollisionActor).first;
	}

	return &Iter->second;
}

void CPanel_Inspector::Clear_EnvCollisionEditState(CGameObject* pObject)
{
	if (nullptr != pObject)
		m_EnvCollisionEditStates.erase(pObject);
}

void CPanel_Inspector::Clear_SectionCollisionEditState(CMapSection* pSection)
{
	if (nullptr != pSection)
		m_SectionCollisionEditStates.erase(pSection);
}

CPanel_Inspector* CPanel_Inspector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CPanel_Inspector(pDevice, pContext);
}

void CPanel_Inspector::Free()
{
	__super::Free();
}