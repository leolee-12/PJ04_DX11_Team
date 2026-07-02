#include "Panel_Inspector.h"
#include "EditInstance.h"
#include "Level_Edit.h"

#include "Shader_PassMeta.h"
#include "MapStage.h"
#include "MapSection.h"
#include "Map_EditFile.h"
#include "Map_EditSession.h"
#include "EnvObject.h"
#include "EnvTrigger_RenderGlobals.h"
#include "LevelDesign_Starblock.h"
#include "LevelDesign_Breakable.h"
#include "LevelDesign_Bush.h"

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

	_bool IsNearlyEqualFloat4x4(const _float4x4& A, const _float4x4& B, _float fEpsilon = 0.0001f)
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

		const _bool bUseCullDistance = ReadBoolProperty(pEnvObject, L"Use Distance Culling", L"EnvObject", Desc.tRender.bUseCullDistance);
		if (bUseCullDistance != Desc.tRender.bUseCullDistance)
		{
			Edit.bHasUseCullDistance = true;
			Edit.bUseCullDistance = bUseCullDistance;
		}

		const _bool bUseCullFrustum = ReadBoolProperty(pEnvObject, L"Use Frustum Culling", L"EnvObject", Desc.tRender.bUseCullFrustum);
		if (bUseCullFrustum != Desc.tRender.bUseCullFrustum)
		{
			Edit.bHasUseCullFrustum = true;
			Edit.bUseCullFrustum = bUseCullFrustum;
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
			Edit.bHasShadow = true;
			Edit.bUseShadow = bCastShadow;
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

	struct MESH_LAYER_UI_CONTEXT
	{
		CModel* pModel = { nullptr };
		const _tchar* pModelComponentTag = { L"Com_Model" };

		_bool bEnvObjectMeshUi = { false };
		_bool bMapObjectMeshUi = { false };
		_bool bEnvPassMeshUi = { false };
		_bool bBushMeshUi = { false };
		_bool bBushBasicMeshUi = { false };
		_bool bBushCutMeshUi = { false };
	};

	MESH_LAYER_UI_CONTEXT Resolve_MeshLayerUIContext(CGameObject* pObject, int iBushMeshSlot)
	{
		MESH_LAYER_UI_CONTEXT Ctx{};

		if (nullptr == pObject)
			return Ctx;

		Ctx.bBushMeshUi = nullptr != dynamic_cast<CLevelDesign_Bush*>(pObject);
		Ctx.bBushBasicMeshUi = Ctx.bBushMeshUi && 0 == iBushMeshSlot;
		Ctx.bBushCutMeshUi = Ctx.bBushMeshUi && 1 == iBushMeshSlot;

		Ctx.pModelComponentTag = Ctx.bBushMeshUi
			? (Ctx.bBushBasicMeshUi ? L"Com_Model_Basic" : L"Com_Model_Cut")
			: L"Com_Model";

		Ctx.pModel = pObject->Get_Component<CModel>(Ctx.pModelComponentTag);

		Ctx.bEnvObjectMeshUi = nullptr != dynamic_cast<CEnvObject*>(pObject);
		Ctx.bMapObjectMeshUi = nullptr != dynamic_cast<Client::CMapObject*>(pObject);

		const _bool bStarblockMeshUi = nullptr != dynamic_cast<CLevelDesign_Starblock*>(pObject);

		const CLevelDesign_Breakable* pBreakable = dynamic_cast<CLevelDesign_Breakable*>(pObject);
		const _bool bBreakableNonAnimMeshUi = nullptr != pBreakable && MODEL::NONANIM == pBreakable->Get_BreakableDesc().eModelType;

		Ctx.bEnvPassMeshUi = Ctx.bEnvObjectMeshUi || bStarblockMeshUi || bBreakableNonAnimMeshUi || Ctx.bBushCutMeshUi;

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

	Draw_Transform(pSelected);

	if (dynamic_cast<CEnvObject*>(pSelected))
	{
		ImGui::Separator();
		Draw_EnvObjectEditPanel(pLevel, pSelected);
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
		nullptr != dynamic_cast<CMapSection*>(pHolder);

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
			if (ImGui::InputInt(("##" + strPropName).c_str(), (int*)pData))
				bChanged = true;
			break;
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

void CPanel_Inspector::Draw_EnvObjectEditPanel(CLevel_Edit* pLevel, CGameObject* pObject)
{
	Client::CEnvObject* pEnvObject = dynamic_cast<CEnvObject*>(pObject);
	if (nullptr == pLevel || nullptr == pEnvObject)
		return;

	_bool* pbRenderable = FindBoolProperty(pEnvObject, L"Renderable", L"EnvObject");
	_bool* pbUseCullDistance = FindBoolProperty(pEnvObject, L"Use Distance Culling", L"EnvObject");
	_bool* pbUseCullFrustum = FindBoolProperty(pEnvObject, L"Use Frustum Culling", L"EnvObject");
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
	if (pbUseCullDistance)
		ImGui::Checkbox("Distance Culling##EnvEdit", (bool*)pbUseCullDistance);
	if (pbUseCullFrustum)
		ImGui::Checkbox("Frustum Culling##EnvEdit", (bool*)pbUseCullFrustum);

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

		const _bool bBaseUseShadow = false;
		if (bHasShadow && nullptr != pbUseShadow && *pbUseShadow != bBaseUseShadow)
		{
			Edit.bHasShadow = true;
			Edit.bUseShadow = *pbUseShadow;
		}

		const _bool bBaseUseCollMesh = false;
		if (bHasCollMesh && nullptr != pbUseCollMesh && *pbUseCollMesh != bBaseUseCollMesh)
		{
			Edit.bHasCollMesh = true;
			Edit.bUseCollMesh = *pbUseCollMesh;
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

		const _bool bBaseUseCollMesh = bSourceCanCreateCollisionActor;

		Edit.bHasCollMesh = false;
		Edit.bUseCollMesh = bBaseUseCollMesh;

		if (bSourceCanCreateCollisionActor && bCreateCollisionActorValue != bBaseUseCollMesh)
		{
			Edit.bHasCollMesh = true;
			Edit.bUseCollMesh = bCreateCollisionActorValue;
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

	static CGameObject* s_pBushMeshOwner = nullptr;
	static int s_iBushMeshSlot = 1;

	const _bool bBushMeshUi = nullptr != dynamic_cast<CLevelDesign_Bush*>(pObject);

	if (bBushMeshUi && s_pBushMeshOwner != pObject)
	{
		s_pBushMeshOwner = pObject;
		s_iBushMeshSlot = 1;
	}

	MESH_LAYER_UI_CONTEXT Ui = Resolve_MeshLayerUIContext(pObject, s_iBushMeshSlot);
	CModel* pModel = Ui.pModel;
	if (nullptr == pModel)
		return;

	if (!ImGui::CollapsingHeader("Mesh Render Settings (per Model)"))
		return;

	ImGui::TextDisabled("Mesh Render Settings are saved per model sidecar.");
	ImGui::TextDisabled("All objects/sections using this model will be affected.");

	_bool bBushSlotChanged = false;

	if (bBushMeshUi)
	{
		int iBushSlot = s_iBushMeshSlot;

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::Combo("Bush Model", &iBushSlot, "Basic\0Cut\0\0"))
		{
			s_iBushMeshSlot = (iBushSlot <= 0) ? 0 : 1;
			bBushSlotChanged = true;

			Ui = Resolve_MeshLayerUIContext(pObject, s_iBushMeshSlot);
			pModel = Ui.pModel;
			if (nullptr == pModel)
			{
				ImGui::TextDisabled("Selected Bush model slot is unavailable.");
				return;
			}
		}

		ImGui::TextDisabled((0 == s_iBushMeshSlot) ? "Basic uses fixed anim pass and Bush texture defaults." : "Cut uses ENV_PASS domain and Bush texture defaults.");
	}

	static const char* UvItems[] = { "TEXCOORD0", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3" };

	const size_t iNumMeshes = pModel->Get_NumMeshes();

	static CGameObject* s_pFocusedMeshOwner = nullptr;
	static _int s_iFocusedMeshIndex = -1;

	_bool bFocusedMeshChanged = (s_pFocusedMeshOwner != pObject) || bBushSlotChanged;

	if (bFocusedMeshChanged)
	{
		s_pFocusedMeshOwner = pObject;
		s_iFocusedMeshIndex = (iNumMeshes > 0) ? 0 : -1;

#ifdef _DEBUG
		if (auto* pSection = dynamic_cast<Client::CMapSection*>(pObject))
		{
			if (m_bEditorSoloMesh)
				pSection->Set_EditorSoloMeshIndex(s_iFocusedMeshIndex);
			else
				pSection->Clear_EditorSoloMesh();
		}
#endif
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
	{
		s_iFocusedMeshIndex = 0;
		bFocusedMeshChanged = true;
	}

#ifdef _DEBUG
	if (auto* pSection = dynamic_cast<Client::CMapSection*>(pObject))
	{
		if (m_bEditorSoloMesh)
			pSection->Set_EditorSoloMeshIndex(s_iFocusedMeshIndex);
		else
			pSection->Clear_EditorSoloMesh();
	}
#endif

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
		{
			s_iFocusedMeshIndex = static_cast<_int>(i);
			bFocusedMeshChanged = true;

#ifdef _DEBUG
			if (auto* pSection = dynamic_cast<Client::CMapSection*>(pObject))
			{
				if (m_bEditorSoloMesh)
					pSection->Set_EditorSoloMeshIndex(s_iFocusedMeshIndex);
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

	const _uint iMesh = static_cast<_uint>(s_iFocusedMeshIndex);
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

	ImGui::PushID(s_iFocusedMeshIndex);
	ImGui::Text("Editing Mesh: %d: %s",
		s_iFocusedMeshIndex,
		pModel->Get_MeshName(iMesh).c_str());

#ifdef _DEBUG
	if (auto* pSection = dynamic_cast<Client::CMapSection*>(pObject))
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

	if (Ui.bEnvPassMeshUi)
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

		if (Layer.iPass == ETOI(ENV_PASS::COLOR))
		{
			ImGui::SetNextItemWidth(180.f);
			if (ImGui::ColorEdit4("Render Color##MeshLayer", (float*)&Layer.vRenderColor))
				bChanged = true;
		}

		if (Ui.bEnvObjectMeshUi)
			ImGui::TextDisabled("Dither is controlled per object in EnvObject Edit.");
		else
			ImGui::TextDisabled("NonAnim LevelDesign uses ENV_PASS domain.");
	}
	else if (Ui.bBushBasicMeshUi)
	{
		ImGui::TextDisabled("Pass is fixed by Bush Basic anim render path.");
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

		if (Ui.bEnvPassMeshUi)
			DrawUVCombo("Unknown UV", Layer.iUnknownUVIndex);

		if (ImGui::Checkbox("Use UV Transform", (bool*)&Layer.bUseUVTransform))
			bChanged = true;

		ImGui::BeginDisabled(!Layer.bUseUVTransform);

		ImGui::SetNextItemWidth(160.f);
		if (ImGui::DragFloat2("UV Scale", (float*)&Layer.vUVScale, 0.01f))
			bChanged = true;

		if (Ui.bEnvPassMeshUi)
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

		if (Ui.bEnvPassMeshUi)
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

		if (Ui.bBushBasicMeshUi)
		{
			if (ImGui::BeginTable("BushBasicTexGrid", 2, ImGuiTableFlags_SizingStretchSame))
			{
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				DrawCompactLayerSlotCell("Nrm", "##BushBasicTexNormal", MTEX_TYPE::NORMALS);

				ImGui::TableNextColumn();
				DrawCompactLayerSlotCell("Unk", "##BushBasicTexUnknown", MTEX_TYPE::UNKNOWN);

				ImGui::EndTable();
			}
		}
		else
		{
			if (ImGui::BeginTable("EnvMeshTexGrid", 4, ImGuiTableFlags_SizingStretchSame))
			{
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				DrawCompactLayerSlotCell("Dif", "##EnvTexDiffuse", MTEX_TYPE::DIFFUSE);

				ImGui::TableNextColumn();
				DrawCompactLayerSlotCell("Nrm", "##EnvTexNormal", MTEX_TYPE::NORMALS);

				ImGui::TableNextColumn();
				DrawCompactLayerSlotCell("MRA", "##EnvTexMRA", MTEX_TYPE::METALNESS);

				ImGui::TableNextColumn();
				DrawCompactLayerSlotCell("Unk", "##EnvTexUnknown", MTEX_TYPE::UNKNOWN);

				ImGui::EndTable();
			}
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

			ImGui::TextDisabled("R:%s  C:%s",
				bRenderable ? "On" : "Off",
				bEnableCulling ? "On" : "Off");
			ImGui::TextDisabled("S:%s  Coll:%s",
				bCastShadow ? "On" : "Off",
				bCreateCollision ? "On" : "Off");

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
#ifdef _DEBUG
			Draw_MapSectionViewFilter(pMapStage, m_pFocusedMapSection, -1);
			ImGui::Separator();
#endif
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
				bUseShadow = bHasShadow && SavedEdit.bUseShadow;
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
				bUseCollMesh = bHasCollMesh && SavedEdit.bUseCollMesh;
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
		if (pLevel->Try_GetMapPreviewSectionEdit(strSectionKey, &SavedEdit)
			&& SavedEdit.bHasCollMesh)
		{
			bCreateCollisionActor =
				bSourceCanCreateCollisionActor && SavedEdit.bUseCollMesh;
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
