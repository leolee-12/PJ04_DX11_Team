#include "Panel_Inspector.h"
#include "EditInstance.h"
#include "Level_Edit.h"
#include "Map_EditSession.h"

#include "Shader_PassMeta.h"
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

	MAP_ENV_EDITED_DESC Build_EnvEditFromCurrentObject(CEnvObject* pEnvObject, _bool bUseNearDistAlpha)
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

		_float4x4 BaseWorld = {};
		XMStoreFloat4x4(&BaseWorld, Build_EnvBaseWorldMatrix(Desc));

		const _float4x4& CurrentWorld =
			*pEnvObject->Get_Transform()->Get_WorldMatrixPtr();

		if (!IsNearlyEqualFloat4x4(CurrentWorld, BaseWorld))
		{
			Edit.bHasWorldMatrix = true;
			Edit.matWorld = CurrentWorld;
		}

		const _bool bBaseUseNearDistAlpha = Desc.tRender.bUseNearDistAlpha;
		if (bUseNearDistAlpha != bBaseUseNearDistAlpha)
		{
			Edit.bHasNearDistAlpha = true;
			Edit.bUseNearDistAlpha = bUseNearDistAlpha;
		}

		return Edit;
	}

	_matrix Build_SectionBaseWorldMatrix(const MAP_SECTION_DESC& Desc)
	{
		_float4x4 Mat{};

		Mat.m[0][0] = Desc.vRight.x;
		Mat.m[0][1] = Desc.vRight.y;
		Mat.m[0][2] = Desc.vRight.z;
		Mat.m[0][3] = Desc.vRight.w;

		Mat.m[1][0] = Desc.vUp.x;
		Mat.m[1][1] = Desc.vUp.y;
		Mat.m[1][2] = Desc.vUp.z;
		Mat.m[1][3] = Desc.vUp.w;

		Mat.m[2][0] = Desc.vLook.x;
		Mat.m[2][1] = Desc.vLook.y;
		Mat.m[2][2] = Desc.vLook.z;
		Mat.m[2][3] = Desc.vLook.w;

		Mat.m[3][0] = Desc.vPosition.x;
		Mat.m[3][1] = Desc.vPosition.y;
		Mat.m[3][2] = Desc.vPosition.z;
		Mat.m[3][3] = Desc.vPosition.w;

		return XMLoadFloat4x4(&Mat);
	}

	MAP_ENV_EDITED_DESC Build_SectionEditFromCurrentSection(Client::CMapSection* pSection)
	{
		MAP_ENV_EDITED_DESC Edit{};
		if (nullptr == pSection)
			return Edit;

		const MAP_SECTION_DESC& Desc = pSection->Get_Desc();

		const _bool bRenderable =
			ReadBoolProperty(pSection, L"Renderable", L"MapSection", Desc.bRenderable);
		if (bRenderable != Desc.bRenderable)
		{
			Edit.bHasRenderable = true;
			Edit.bRenderable = bRenderable;
		}

		const _bool bEnableCulling =
			ReadBoolProperty(pSection, L"Enable Culling", L"MapSection", Desc.bEnableCulling);
		if (bEnableCulling != Desc.bEnableCulling)
		{
			Edit.bHasEnableCulling = true;
			Edit.bEnableCulling = bEnableCulling;
		}

		const _bool bCastShadow =
			ReadBoolProperty(pSection, L"Cast Shadow", L"MapSection", Desc.bCastShadow);
		if (bCastShadow != Desc.bCastShadow)
		{
			Edit.bHasCastShadow = true;
			Edit.bCastShadow = bCastShadow;
		}

		_float4x4 BaseWorld = {};
		XMStoreFloat4x4(&BaseWorld, Build_SectionBaseWorldMatrix(Desc));

		const _float4x4& CurrentWorld =
			*pSection->Get_Transform()->Get_WorldMatrixPtr();

		if (!IsNearlyEqualFloat4x4(CurrentWorld, BaseWorld))
			Fill_EditWorldMatrix(pSection, &Edit);

		return Edit;
	}

	const _char* GetEnvShaderPassComboItem(void*, _int idx)
	{
		if (idx < 0 || idx >= static_cast<int>(_countof(g_EnvShaderPassMetas)))
			return nullptr;

		return g_EnvShaderPassMetas[idx].szName;
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
	Client::CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject);
	if (nullptr == pLevel || nullptr == pEnvObject)
		return;

	_bool* pbRenderable = FindBoolProperty(pEnvObject, L"Renderable", L"EnvObject");
	_bool* pbEnableCulling = FindBoolProperty(pEnvObject, L"Enable Culling", L"EnvObject");
	_bool* pbUseShadow = Resolve_EnvShadowEditState(pLevel, pEnvObject);
	_bool* pbUseCollMesh = Resolve_EnvCollMeshEditState(pLevel, pEnvObject);
	_bool* pbUseNearDistAlpha = Resolve_EnvNearAlphaEditState(pLevel, pEnvObject);

	const ENV_OBJECT_DESC& Desc = pEnvObject->Get_Desc();
	const auto& Collision = Desc.tCollision;
	const auto& Render = Desc.tRender;

	const _bool bHasShadow = Render.bHasShadow;
	const _bool bHasCollMesh = Collision.bHasCollMesh;

	ImGui::TextUnformatted("EnvObject Edit");

	if (pbRenderable)
		ImGui::Checkbox("Renderable##EnvEdit", (bool*)pbRenderable);
	if (pbEnableCulling)
		ImGui::Checkbox("Enable Culling##EnvEdit", (bool*)pbEnableCulling);

	if (pbUseShadow)
	{
		ImGui::BeginDisabled(!bHasShadow);
		ImGui::Checkbox("Use Shadow On Reload##EnvEdit", (bool*)pbUseShadow);
		ImGui::EndDisabled();
	}

	if (pbUseCollMesh)
	{
		ImGui::BeginDisabled(!bHasCollMesh);
		ImGui::Checkbox("Use Collision Mesh On Reload##EnvEdit", (bool*)pbUseCollMesh);
		ImGui::EndDisabled();
	}

	if (!bHasShadow)
		ImGui::TextDisabled("Source data does not provide shadow capability for this object.");

	if (!bHasCollMesh)
		ImGui::TextDisabled("Source data does not provide collision mesh capability for this object.");

	if (pbUseNearDistAlpha)
		ImGui::Checkbox("Use Near Dist Alpha##EnvEdit", (bool*)pbUseNearDistAlpha);

	ImGui::TextDisabled("Applied on next env reload.");
	ImGui::TextDisabled("Restart persistence requires Save Override Now or toolbar Map Edit Save.");

	if (ImGui::Button("Apply Current##EnvEdit"))
	{
		MAP_ENV_EDITED_DESC Edit = Build_EnvEditFromCurrentObject(
			pEnvObject,
			(nullptr != pbUseNearDistAlpha)
			? *pbUseNearDistAlpha
			: Desc.tRender.bUseNearDistAlpha);

		if (bHasShadow)
		{
			Edit.bHasShadow = true;
			Edit.bUseShadow = (nullptr != pbUseShadow) ? *pbUseShadow : false;
		}

		if (bHasCollMesh)
		{
			Edit.bHasCollMesh = true;
			Edit.bUseCollMesh = (nullptr != pbUseCollMesh) ? *pbUseCollMesh : false;
		}

		if (Has_AnyMapEnvEdit(Edit))
		{
			pLevel->Track_EditedMapPreviewEnvObject(pObject, Edit);
		}
		else
		{
			pLevel->Clear_EditedMapPreviewEnvObject(pObject);
			Clear_EnvShadowEditState(pObject);
			Clear_EnvCollMeshEditState(pObject);
			Clear_EnvNearAlphaEditState(pObject);
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
		m_EnvShadowEditStates.clear();
		m_EnvCollMeshEditStates.clear();
		m_EnvNearAlphaEditStates.clear();

		if (0 <= iPresetIndex)
		{
			pLevel->Load_MapPreviewEnv(static_cast<_uint>(iPresetIndex));
			return;
		}
	}
}

void CPanel_Inspector::Draw_MapSectionEditPanel(CLevel_Edit* pLevel, CMapStage* pMapStage, CMapSection* pSection)
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
		Resolve_MapCollMeshEditState(pLevel, pMapStage, pSection);

	const _bool bSourceCanCreateCollisionActor =
		pSection->Get_Desc().bSourceCreateCollisionActor;

	const string strStageName = WstrToStr(pMapStage->Get_StageName());
	const string strSectionName = WstrToStr(pSection->Get_SectionName());

	ImGui::TextUnformatted("MapSection Edit");
	ImGui::TextDisabled("Stage: %s", strStageName.empty() ? "<Unnamed Stage>" : strStageName.c_str());
	ImGui::TextDisabled("Section: %s", strSectionName.empty() ? "<Unnamed Section>" : strSectionName.c_str());

	if (ImGui::CollapsingHeader("Section Flags", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginGroup();
		if (pbRenderable)
			ImGui::Checkbox("Renderable##SectionEdit", (bool*)pbRenderable);
		if (pbCastShadow)
			ImGui::Checkbox("Cast Shadow##SectionEdit", (bool*)pbCastShadow);
		ImGui::EndGroup();

		ImGui::SameLine(0.f, 20.f);

		ImGui::BeginGroup();
		if (pbEnableCulling)
			ImGui::Checkbox("Enable Culling##SectionEdit", (bool*)pbEnableCulling);
		if (pbCreateCollisionActor)
		{
			ImGui::BeginDisabled(!bSourceCanCreateCollisionActor);
			ImGui::Checkbox("Create Collision Actor##SectionEdit", (bool*)pbCreateCollisionActor);
			ImGui::EndDisabled();
		}
		ImGui::EndGroup();

		if (!bSourceCanCreateCollisionActor)
			ImGui::TextDisabled("Coll actor unavailable.");

		ImGui::TextDisabled("Apply on reload.");
		ImGui::TextDisabled("Save to persist.");
	}

	if (ImGui::Button("Apply##SectionEdit"))
	{
		MAP_ENV_EDITED_DESC Edit = Build_SectionEditFromCurrentSection(pSection);

		const _bool bCreateCollisionActorValue =
			(nullptr != pbCreateCollisionActor)
			? *pbCreateCollisionActor
			: bSourceCanCreateCollisionActor;

		Edit.bHasCollMeshEdited = false;
		Edit.bCreateCollMesh = true;
		Edit.bDisableCollMesh = false;

		if (bSourceCanCreateCollisionActor)
		{
			Edit.bHasCollMeshEdited = (bCreateCollisionActorValue != true);
			Edit.bCreateCollMesh = bCreateCollisionActorValue;
			Edit.bDisableCollMesh = !bCreateCollisionActorValue;
		}

		pSection->Set_CollisionActorEnabled(
			bSourceCanCreateCollisionActor ? bCreateCollisionActorValue : false);

		if (Has_AnyMapEnvEdit(Edit))
		{
			pLevel->Track_EditedMapPreviewSection(strSectionKey, Edit);
		}
		else
		{
			pLevel->Clear_EditedMapPreviewSection(strSectionKey);
			Clear_MapCollMeshEditState(pSection);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Save##SectionEdit"))
	{
		if (FAILED(pLevel->Save_MapOverride()))
			MSG_BOX("MAP EDIT SAVE FAILED");
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear##SectionEdit"))
	{
		const CMap_EditSession* pSession = pLevel->Get_MapPreviewSession();
		const _int iPresetIndex =
			(nullptr != pSession) ? pSession->Get_EditData().iPresetIndex : -1;

		pLevel->Clear_EditedMapPreviewSection(strSectionKey);
		m_MapCollMeshEditStates.clear();

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

	ImGui::TextDisabled("Mesh Render Settings are saved per model sidecar.");
	ImGui::TextDisabled("All objects/sections using this model will be affected.");

	const _bool bEnvObjectMeshUi =
		nullptr != dynamic_cast<Client::CEnvObject*>(pObject);
	const _bool bMapObjectMeshUi =
		nullptr != dynamic_cast<Client::CMapObject*>(pObject);

	static const char* UvItems[] = { "TEXCOORD0", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3" };

	const size_t iNumMeshes = pModel->Get_NumMeshes();

	static CGameObject* s_pFocusedMeshOwner = nullptr;
	static _int s_iFocusedMeshIndex = -1;

	if (s_pFocusedMeshOwner != pObject)
	{
		s_pFocusedMeshOwner = pObject;
		s_iFocusedMeshIndex = (iNumMeshes > 0) ? 0 : -1;
	}

	if (0 == iNumMeshes)
	{
		ImGui::TextDisabled("No mesh available.");

		if (ImGui::Button("Bake (Save sidecar)"))
		{
			if (FAILED(pModel->Save_MeshLayers()))
				MSG_BOX("MESH LAYER SAVE FAILED");
		}
		return;
	}

	if (s_iFocusedMeshIndex < 0 || s_iFocusedMeshIndex >= static_cast<_int>(iNumMeshes))
		s_iFocusedMeshIndex = 0;

	ImGui::BeginChild("MeshList", ImVec2(0.f, 140.f), true);
	for (size_t i = 0; i < iNumMeshes; ++i)
	{
		const _uint iMesh = static_cast<_uint>(i);
		const MESH_LAYER_IDX SummaryLayer = pModel->Get_MeshLayer(iMesh);
		const string strMeshName = pModel->Get_MeshName(iMesh);
		const string strLabel = to_string(i) + ": " + strMeshName;
		const _bool bSelected = (s_iFocusedMeshIndex == static_cast<_int>(i));

		ImGui::PushID(static_cast<int>(i));

		if (ImGui::Selectable(strLabel.c_str(), bSelected, ImGuiSelectableFlags_SpanAllColumns))
			s_iFocusedMeshIndex = static_cast<_int>(i);

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

	const _uint iMesh = static_cast<_uint>(s_iFocusedMeshIndex);
	MESH_LAYER_IDX Layer = pModel->Get_MeshLayer(iMesh);

	ImGui::PushID(s_iFocusedMeshIndex);
	ImGui::Text("Editing Mesh: %d: %s",
		s_iFocusedMeshIndex,
		pModel->Get_MeshName(iMesh).c_str());

	_bool bChanged = false;
	_bool bAnyField = false;

	auto DrawUVCombo = [&](const char* pLabel, _uint& iUVIndex)
		{
			int iValue = (iUVIndex <= 3u) ? static_cast<int>(iUVIndex) : 0;

			ImGui::SetNextItemWidth(160.f);
			if (ImGui::Combo(pLabel, &iValue, UvItems, IM_ARRAYSIZE(UvItems)))
			{
				iUVIndex = static_cast<_uint>(iValue);
				bChanged = true;
			}
		};

	auto DrawCompactUVCombo = [&](const char* pLabel, _uint& iUVIndex)
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

	auto BuildCompactComboItems = [&](int iMinValue, int iMaxValue)
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

	auto DrawCompactLayerSlotCell = [&](const char* pLabel, const char* pId, MTEX_TYPE eType)
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

	auto DrawCompactExtraCell = [&](const char* pLabel, const char* pIdBase, int& iBindIndex, unsigned int& iTexType)
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

	if (bEnvObjectMeshUi)
	{
		int iPassCombo = Get_EnvShaderPassComboIndex(Layer.iPass);

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::Combo("Pass",
			&iPassCombo,
			GetEnvShaderPassComboItem,
			nullptr,
			static_cast<int>(_countof(g_EnvShaderPassMetas))))
		{
			Layer.iPass = Get_EnvShaderPassFromComboIndex(iPassCombo);
			bChanged = true;
		}

		DrawUVCombo("UV", Layer.iUVIndex);
		ImGui::TextDisabled("Dither is controlled per object in EnvObject Edit.");
	}
	else if (bMapObjectMeshUi)
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

	if (!bMapObjectMeshUi)
	{
		if (ImGui::Checkbox("Use UV Transform", (bool*)&Layer.bUseUVTransform))
			bChanged = true;

		ImGui::BeginDisabled(!Layer.bUseUVTransform);

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::DragFloat2("UV Scale", (float*)&Layer.vUVScale, 0.01f))
			bChanged = true;

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::DragFloat2("UV Offset", (float*)&Layer.vUVOffset, 0.01f))
			bChanged = true;

		ImGui::EndDisabled();

		for (_uint t = 0; t < MTEX_TYPE_MAX; ++t)
		{
			const MTEX_TYPE eType = static_cast<MTEX_TYPE>(t);
			const int iCount = static_cast<int>(pModel->Get_MeshTextureCount(iMesh, eType));
			const _uint iLayerIndex = Layer.idx[t];
			const _bool bImportant = Is_EnvImportantTexType(eType);
			const _bool bOutOfRange = (iCount > 0) && (iLayerIndex >= static_cast<_uint>(iCount));

			if (0 == iCount)
			{
				if (bEnvObjectMeshUi && bImportant)
				{
					bAnyField = true;
					ImGui::TextDisabled("%s: no texture", TexTypeName(t));
				}
				continue;
			}

			if (1 == iCount)
			{
				bAnyField = true;
				ImGui::TextDisabled("%s: single texture (index 0)%s",
					TexTypeName(t),
					bOutOfRange ? "  (layer index out of range)" : "");
				continue;
			}

			bAnyField = true;

			int iValue = static_cast<int>(iLayerIndex);
			ImGui::SetNextItemWidth(120.f);
			if (ImGui::InputInt(TexTypeName(t), &iValue))
			{
				if (iValue < 0)
					iValue = 0;
				if (iValue >= iCount)
					iValue = iCount - 1;

				Layer.idx[t] = static_cast<_uint>(iValue);
				bChanged = true;
			}
			ImGui::SameLine();
			ImGui::Text("/ %d", iCount);
		}
	}

	if (!bAnyField)
		ImGui::TextDisabled("  (no texture slot override)");

	if (bChanged)
		pModel->Set_MeshLayer(iMesh, Layer);

	ImGui::Separator();
	ImGui::PopID();

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
	float fListWidth = vAvail.x * 0.26f;
	if (fListWidth < 180.f)
		fListWidth = 180.f;
	else if (fListWidth > 260.f)
		fListWidth = 260.f;

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
				m_pFocusedMapSection = pSection;

			const _bool bRenderable =
				ReadBoolProperty(pSection, L"Renderable", L"MapSection", pSection->Get_Desc().bRenderable);
			const _bool bEnableCulling =
				ReadBoolProperty(pSection, L"Enable Culling", L"MapSection", pSection->Get_Desc().bEnableCulling);
			const _bool bCastShadow =
				ReadBoolProperty(pSection, L"Cast Shadow", L"MapSection", pSection->Get_Desc().bCastShadow);
			_bool* pbCreateCollisionActor =
				Resolve_MapCollMeshEditState(pLevel, pMapStage, pSection);

			const _bool bCreateCollision =
				(nullptr != pbCreateCollisionActor)
				? *pbCreateCollisionActor
				: pSection->Get_Desc().bSourceCreateCollisionActor;

			ImGui::Indent();
			ImGui::TextDisabled("R:%s  C:%s  S:%s  Coll:%s",
				bRenderable ? "On" : "Off",
				bEnableCulling ? "On" : "Off",
				bCastShadow ? "On" : "Off",
				bCreateCollision ? "On" : "Off");
			ImGui::Unindent();

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
				m_pFocusedMapSection->Notify_EditTransformChanged();

			ImGui::Separator();
			Draw_MapSectionEditPanel(pLevel, pMapStage, m_pFocusedMapSection);
			ImGui::Separator();
			Draw_Properties(m_pFocusedMapSection);
			ImGui::Separator();
			Draw_MeshLayerPanel(m_pFocusedMapSection);
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

_bool* CPanel_Inspector::Resolve_EnvShadowEditState(CLevel_Edit* pLevel, Client::CEnvObject* pEnvObject)
{
	if (nullptr == pLevel || nullptr == pEnvObject)
		return nullptr;

	auto Iter = m_EnvShadowEditStates.find(pEnvObject);
	if (Iter == m_EnvShadowEditStates.end())
	{
		MAP_ENV_EDITED_DESC SavedEdit{};
		const _bool bHasShadow = pEnvObject->Get_Desc().tRender.bHasShadow;

		_bool bUseShadow = false;
		if (pLevel->Try_GetMapPreviewEnvEdit(pEnvObject, &SavedEdit))
		{
			if (SavedEdit.bHasShadow)
				bUseShadow = SavedEdit.bUseShadow;
			else if (SavedEdit.bHasCastShadow)
				bUseShadow = SavedEdit.bCastShadow;
		}

		if (!bHasShadow)
			bUseShadow = false;

		Iter = m_EnvShadowEditStates.emplace(
			static_cast<CGameObject*>(pEnvObject),
			bUseShadow).first;
	}

	return &Iter->second;
}

_bool* CPanel_Inspector::Resolve_EnvCollMeshEditState(CLevel_Edit* pLevel, Client::CEnvObject* pEnvObject)
{
	if (nullptr == pLevel || nullptr == pEnvObject)
		return nullptr;

	auto Iter = m_EnvCollMeshEditStates.find(pEnvObject);
	if (Iter == m_EnvCollMeshEditStates.end())
	{
		MAP_ENV_EDITED_DESC SavedEdit{};
		const auto& Collision = pEnvObject->Get_Desc().tCollision;
		const _bool bHasCollMesh = Collision.bHasCollMesh;

		_bool bUseCollMesh = false;
		if (pLevel->Try_GetMapPreviewEnvEdit(pEnvObject, &SavedEdit))
		{
			if (SavedEdit.bHasCollMesh)
			{
				bUseCollMesh = SavedEdit.bUseCollMesh;
			}
			else if (SavedEdit.bHasCollMeshEdited || SavedEdit.bDisableCollMesh)
			{
				// Legacy compatibility only:
				// - CollisionMesh.Create
				// - CollisionMeshDisabled
				bUseCollMesh = SavedEdit.bCreateCollMesh && !SavedEdit.bDisableCollMesh;
			}
		}

		if (!bHasCollMesh)
			bUseCollMesh = false;

		Iter = m_EnvCollMeshEditStates.emplace(
			static_cast<CGameObject*>(pEnvObject),
			bUseCollMesh).first;
	}

	return &Iter->second;
}

_bool* CPanel_Inspector::Resolve_MapCollMeshEditState(CLevel_Edit* pLevel, CMapStage* pMapStage,
	CMapSection* pSection)
{
	if (nullptr == pLevel || nullptr == pMapStage || nullptr == pSection)
		return nullptr;

	auto Iter = m_MapCollMeshEditStates.find(pSection);
	if (Iter == m_MapCollMeshEditStates.end())
	{
		const _wstring strSectionKey = CMap_EditFile::Make_SectionKey(
			pMapStage->Get_StageName(),
			pSection->Get_SectionName());

		MAP_ENV_EDITED_DESC SavedEdit{};
		const _bool bSourceCanCreateCollisionActor =
			pSection->Get_Desc().bSourceCreateCollisionActor;

		_bool bCreateCollisionActor = bSourceCanCreateCollisionActor;
		if (pLevel->Try_GetMapPreviewSectionEdit(strSectionKey, &SavedEdit))
		{
			if (SavedEdit.bHasCollMeshEdited)
				bCreateCollisionActor = SavedEdit.bCreateCollMesh;
			else
				bCreateCollisionActor = !SavedEdit.bDisableCollMesh;
		}

		if (!bSourceCanCreateCollisionActor)
			bCreateCollisionActor = false;

		Iter = m_MapCollMeshEditStates.emplace(
			pSection,
			bCreateCollisionActor).first;
	}

	return &Iter->second;
}

_bool* CPanel_Inspector::Resolve_EnvNearAlphaEditState(CLevel_Edit* pLevel, CEnvObject* pEnvObject)
{
	if (nullptr == pLevel || nullptr == pEnvObject)
		return nullptr;

	auto Iter = m_EnvNearAlphaEditStates.find(pEnvObject);
	if (Iter == m_EnvNearAlphaEditStates.end())
	{
		MAP_ENV_EDITED_DESC SavedEdit{};
		const _bool bUseNearDistAlpha =
			pLevel->Try_GetMapPreviewEnvEdit(pEnvObject, &SavedEdit) && SavedEdit.bHasNearDistAlpha
			? SavedEdit.bUseNearDistAlpha
			: pEnvObject->Get_Desc().tRender.bUseNearDistAlpha;

		Iter = m_EnvNearAlphaEditStates.emplace(
			static_cast<CGameObject*>(pEnvObject),
			bUseNearDistAlpha).first;
	}

	return &Iter->second;
}

void CPanel_Inspector::Clear_EnvShadowEditState(CGameObject* pObject)
{
	if (nullptr != pObject)
		m_EnvShadowEditStates.erase(pObject);
}

void CPanel_Inspector::Clear_EnvCollMeshEditState(CGameObject* pObject)
{
	if (nullptr != pObject)
		m_EnvCollMeshEditStates.erase(pObject);
}

void CPanel_Inspector::Clear_MapCollMeshEditState(CMapSection* pSection)
{
	if (nullptr != pSection)
		m_MapCollMeshEditStates.erase(pSection);
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