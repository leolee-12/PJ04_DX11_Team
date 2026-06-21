#include "LevelDesign_Parser.h"
#include "DataLoader.h"
#include "Parsing_Utils.h"

#include <exception>
#include <filesystem>

NS_BEGIN(Client)

namespace
{
	_bool Equals_NoCase(const _wstring& strLeft, const wchar_t* pRight)
	{
		return 0 == _wcsicmp(strLeft.c_str(), pRight);
	}

	_bool Is_BreakableObject(const _wstring& strObjectName)
	{
		return Equals_NoCase(strObjectName, L"StarBlock")
			|| Equals_NoCase(strObjectName, L"StarBlockBig")
			|| Equals_NoCase(strObjectName, L"WoodBox");
	}

	LD_RAIL_TYPE Resolve_RailType(const _wstring& strErpType)
	{
		if (Equals_NoCase(strErpType, L"Line")) return LD_RAIL_TYPE::LINE;
		if (Equals_NoCase(strErpType, L"Circle")) return LD_RAIL_TYPE::CIRCLE;
		if (Equals_NoCase(strErpType, L"Bezier")) return LD_RAIL_TYPE::BEZIER;
		return LD_RAIL_TYPE::UNKNOWN;
	}

	LD_VOLUME_TYPE Resolve_VolumeType(const _wstring& strObjectName)
	{
		if (Equals_NoCase(strObjectName, L"InvisibleCollision"))                return LD_VOLUME_TYPE::INVISIBLE_COLLISION;
		if (Equals_NoCase(strObjectName, L"InvisibleCollisionBox"))     return LD_VOLUME_TYPE::INVISIBLE_COLLISION_BOX;
		if (Equals_NoCase(strObjectName, L"FallBorder"))                                return LD_VOLUME_TYPE::FALL_BORDER;
		if (Equals_NoCase(strObjectName, L"WaterArea"))                         return LD_VOLUME_TYPE::WATER_AREA;
		return LD_VOLUME_TYPE::UNKNOWN;
	}

	LD_AUDIO_AREA_TYPE Resolve_AudioAreaType(const _wstring& strObjectName)
	{
		if (Equals_NoCase(strObjectName, L"AreaBgmRequestor"))
			return LD_AUDIO_AREA_TYPE::BGM_REQUESTOR;

		if (Equals_NoCase(strObjectName, L"AreaSeJungle")
			|| Equals_NoCase(strObjectName, L"AreaSeSeaWave"))
		{
			return LD_AUDIO_AREA_TYPE::AREA_SE;
		}

		return LD_AUDIO_AREA_TYPE::UNKNOWN;
	}

	_bool Try_ReadRailNodes(const json& jEntry, _float fDefaultControlLength, vector<LD_RAIL_NODE_DESC>* pOutNodes)
	{
		if (nullptr == pOutNodes)
			return false;

		vector<_float3> LegacyNodes;
		if (JsonUtils::Try_ReadFloat3List(jEntry, "Nodes", &LegacyNodes))
		{
			pOutNodes->clear();
			pOutNodes->reserve(LegacyNodes.size());

			for (const _float3& vPosition : LegacyNodes)
				pOutNodes->push_back({ vPosition, fDefaultControlLength });

			return true;
		}

		const json* pNodeArray = JsonUtils::Find_JsonValue(jEntry, "Node");
		if (nullptr == pNodeArray || !pNodeArray->is_array())
			return false;

		vector<LD_RAIL_NODE_DESC> ParsedNodes;
		ParsedNodes.reserve(pNodeArray->size());

		for (const json& jNode : *pNodeArray)
		{
			if (!jNode.is_object())
				return false;

			LD_RAIL_NODE_DESC Node{};
			Node.fBezierControlLength = fDefaultControlLength;

			if (!JsonUtils::Try_ReadFloat3Array(jNode, "Pos", &Node.vPosition))
				return false;

			JsonUtils::Try_ReadFloat(jNode, "BezierControlLength", &Node.fBezierControlLength);
			ParsedNodes.push_back(Node);
		}

		*pOutNodes = std::move(ParsedNodes);
		return true;
	}
}

HRESULT CLevelDesign_Parser::Parse_File(const _wstring& strJsonPath, LD_PACKAGE* pOutPackage)
{
	if (nullptr == pOutPackage)
		return E_FAIL;

	*pOutPackage = {};

	string strContent;
	if (FAILED(CDataLoader::Read_Json(strJsonPath.c_str(), &strContent)))
		return E_FAIL;

	json jRoot;
	try
	{
		jRoot = json::parse(strContent);
	}
	catch (const std::exception&)
	{
		return E_FAIL;
	}

	return Parse_Root(jRoot, strJsonPath, pOutPackage);
}

HRESULT CLevelDesign_Parser::Parse_Root(const json& jRoot, const _wstring& wstrSourcePath, LD_PACKAGE* pOutPackage)
{
	if (nullptr == pOutPackage || !jRoot.is_object())
		return E_FAIL;

	*pOutPackage = {};
	pOutPackage->wstrSourcePath = wstrSourcePath;

	_wstring strSourceFile = std::filesystem::path(wstrSourcePath).filename().wstring();
	JsonUtils::Try_ReadString(jRoot, "source_file", &strSourceFile);

	const json* pData = JsonUtils::Find_JsonValue(jRoot, "data");
	if (nullptr == pData || !pData->is_object())
		return E_FAIL;

	for (auto Iter = pData->begin(); Iter != pData->end(); ++Iter)
	{
		const _wstring strSection = StrToWstr(Iter.key());
		const json& jSection = Iter.value();

		if (Equals_NoCase(strSection, L"StepLinkInfo"))
		{
			Parse_StepLinkInfo(jSection, &pOutPackage->StepLinks);
			continue;
		}

		if (jSection.is_object())
		{
			Parse_ObjectSection(
				wstrSourcePath,
				strSourceFile,
				strSection,
				jSection,
				&pOutPackage->ObjectDescs);
		}
	}

	return S_OK;
}

void CLevelDesign_Parser::Parse_ObjectSection(
	const _wstring& wstrSourcePath,
	const _wstring& strSourceFile,
	const _wstring& strSection,
	const json& jSection,
	vector<LD_OBJECT_ENTRY>* pOutDescs)
{
	if (nullptr == pOutDescs || !jSection.is_object())
		return;

	for (auto Iter = jSection.begin(); Iter != jSection.end(); ++Iter)
	{
		if (!Iter.value().is_object())
			continue;

		LD_OBJECT_DESC CommonDesc = Make_BaseDesc(
			wstrSourcePath,
			strSourceFile,
			strSection,
			StrToWstr(Iter.key()),
			Iter.value());

		Fill_Common(Iter.value(), &CommonDesc);

		if (Equals_NoCase(CommonDesc.strObjectName, L"Ladder"))
		{
			LD_LADDER_DESC Desc{};
			static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
			Fill_LadderFields(Iter.value(), &Desc);

			pOutDescs->emplace_back(std::move(Desc));
			continue;
		}

		if (Is_BreakableObject(CommonDesc.strObjectName))
		{
			LD_BREAKABLE_OBJECT_DESC Desc{};
			static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
			Desc.eCategory = LD_CATEGORY::BREAKABLE;

			pOutDescs->emplace_back(std::move(Desc));
			continue;
		}

		LD_PARSED_OBJECT Desc{};
		static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
		Fill_SpecialFields(Iter.value(), &Desc);

		pOutDescs->emplace_back(std::move(Desc));
	}
}

void CLevelDesign_Parser::Parse_StepLinkInfo(
	const json& jArray,
	vector<LD_STEP_LINK_INFO>* pOutStepLinks)
{
	if (nullptr == pOutStepLinks || !jArray.is_array())
		return;

	pOutStepLinks->clear();
	pOutStepLinks->reserve(jArray.size());

	for (const json& jEntry : jArray)
	{
		if (!jEntry.is_object())
			continue;

		LD_STEP_LINK_INFO Info{};
		JsonUtils::Try_ReadBoolFromNumeric(jEntry, "IgnoreWarningAtOneWay", &Info.bIgnoreWarningAtOneWay);
		JsonUtils::Try_ReadBoolFromNumeric(jEntry, "RoundTrip", &Info.bRoundTrip);

		if (!JsonUtils::Try_ReadInt(jEntry, "MoveStepValue", &Info.iMoveStepValue))
			JsonUtils::Try_ReadInt(jEntry, "MoveStep", &Info.iMoveStepValue);

		Info.jRaw = jEntry;
		pOutStepLinks->push_back(Info);
	}
}

LD_OBJECT_DESC CLevelDesign_Parser::Make_BaseDesc(
	const _wstring& wstrSourcePath,
	const _wstring& strSourceFile,
	const _wstring& strSection,
	const _wstring& strEntryKey,
	const json& jEntry)
{
	UNREFERENCED_PARAMETER(jEntry);

	LD_OBJECT_DESC Desc{};
	Desc.wstrSourcePath = wstrSourcePath;
	Desc.strSourceFile = strSourceFile;
	Desc.strSection = strSection;
	Desc.strEntryKey = strEntryKey;
	return Desc;
}

void CLevelDesign_Parser::Fill_Common(const json& jEntry, LD_OBJECT_DESC* pDesc)
{
	if (nullptr == pDesc)
		return;

	JsonUtils::Try_ReadString(jEntry, "Basic.ObjectName", &pDesc->strObjectName);
	if (pDesc->strObjectName.empty())
		JsonUtils::Try_ReadString(jEntry, "Basic.BasicInfo.ObjectName", &pDesc->strObjectName);

	JsonUtils::Try_ReadString(jEntry, "Kind", &pDesc->strKind);

	if (!JsonUtils::Try_ReadUInt(jEntry, "Uid", &pDesc->iUid))
		JsonUtils::Try_ReadUInt(jEntry, "Basic.BasicInfo.Uid", &pDesc->iUid);

	if (!JsonUtils::Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.Position", &pDesc->vParsedPosition))
		if (!JsonUtils::Try_ReadFloat3Array(jEntry, "Pos", &pDesc->vParsedPosition))
			if (!JsonUtils::Try_ReadFloat3Array(jEntry, "Center", &pDesc->vParsedPosition))
				JsonUtils::Try_ReadFloat3Array(jEntry, "AreaCenter", &pDesc->vParsedPosition);

	if (!JsonUtils::Try_ReadFloat4Array(jEntry, "Basic.BasicInfo.Rotation", &pDesc->qParsedRotation))
		if (!JsonUtils::Try_ReadFloat4Array(jEntry, "Rot", &pDesc->qParsedRotation))
			JsonUtils::Try_ReadFloat4Array(jEntry, "AreaRot", &pDesc->qParsedRotation);

	if (!JsonUtils::Try_ReadFloat3Array(jEntry, "Basic.BasicInfo.SceneScale", &pDesc->vParsedScale))
		JsonUtils::Try_ReadFloat3Array(jEntry, "Scale", &pDesc->vParsedScale);

	JsonUtils::Try_ReadUInt(jEntry, "Basic.RailUser.TargetRailUid", &pDesc->iTargetRailUid);
	JsonUtils::Try_ReadInt(jEntry, "TargetLandGroupIndex", &pDesc->iTargetLandGroupIndex);
	JsonUtils::Try_ReadUInt(jEntry, "EventReceiverId", &pDesc->iEventReceiverId);
	JsonUtils::Try_ReadUInt(jEntry, "EventSenderId", &pDesc->iEventSenderId);

	pDesc->jRaw = jEntry;
	Build_TransformDesc(pDesc);
}

void CLevelDesign_Parser::Fill_SpecialFields(const json& jEntry, LD_PARSED_OBJECT* pDesc)
{
	if (nullptr == pDesc)
		return;

	const _wstring& strObjectName = pDesc->strObjectName;

	if (Equals_NoCase(strObjectName, L"StartPortal"))
	{
		pDesc->eCategory = LD_CATEGORY::PORTAL;
		JsonUtils::Try_ReadInt(jEntry, "PortalNo", &pDesc->Portal.iPortalNo);
		JsonUtils::Try_ReadBoolFromNumeric(jEntry, "UseRestartCheck", &pDesc->Portal.bUseRestartCheck);
		JsonUtils::Try_ReadFloat3Array(jEntry, "RestartAreaOffs", &pDesc->Portal.vRestartAreaOffs);
		JsonUtils::Try_ReadFloat4Array(jEntry, "RestartAreaRot", &pDesc->Portal.qRestartAreaRot);
		JsonUtils::Try_ReadFloat3Array(jEntry, "RestartAreaSize", &pDesc->Portal.vRestartAreaSize);
		return;
	}

	if (Equals_NoCase(strObjectName, L"Rail"))
	{
		pDesc->eCategory = LD_CATEGORY::RAIL;

		if (!JsonUtils::Try_ReadFloat3Array(jEntry, "CenterPos", &pDesc->Rail.vCenterPos))
		{
			JsonUtils::Try_ReadFloat3Array(jEntry, "Center", &pDesc->Rail.vCenterPos);
		}

		JsonUtils::Try_ReadFloat(jEntry, "Radius", &pDesc->Rail.fRadius);
		JsonUtils::Try_ReadFloat(jEntry, "BezierControlLength", &pDesc->Rail.fBezierControlLength);
		
		_wstring strErpType;
		JsonUtils::Try_ReadString(jEntry, "ErpType", &strErpType);
		pDesc->Rail.eType = Resolve_RailType(strErpType);

		JsonUtils::Try_ReadBoolFromNumeric(jEntry, "IsClockwise", &pDesc->Rail.bClockwise);
		JsonUtils::Try_ReadBoolFromNumeric(jEntry, "IsClose", &pDesc->Rail.bClose);

		Try_ReadRailNodes(jEntry, pDesc->Rail.fBezierControlLength, &pDesc->Rail.Nodes);

		// 런타임에서는 실제 파싱된 노드 목록을 기준으로 한다.
		pDesc->Rail.iNodeCount = static_cast<_uint>(pDesc->Rail.Nodes.size());

		if (0.f == pDesc->Rail.vCenterPos.x
			&& 0.f == pDesc->Rail.vCenterPos.y
			&& 0.f == pDesc->Rail.vCenterPos.z)
		{
			pDesc->Rail.vCenterPos = pDesc->vParsedPosition;
		}

		return;
	}

	const LD_VOLUME_TYPE eVolumeType = Resolve_VolumeType(strObjectName);
	if (eVolumeType != LD_VOLUME_TYPE::UNKNOWN)
	{
		pDesc->eCategory = LD_CATEGORY::VOLUME;
		pDesc->Volume.eVolumeType = eVolumeType;

		_string strMainPath;
		switch (eVolumeType)
		{
		case LD_VOLUME_TYPE::INVISIBLE_COLLISION:
			strMainPath = "Gimmick.InvisibleCollision.MainComponent";
			break;
		case LD_VOLUME_TYPE::INVISIBLE_COLLISION_BOX:
			strMainPath = "Gimmick.InvisibleCollisionBox.MainComponent";
			break;
		case LD_VOLUME_TYPE::FALL_BORDER:
			strMainPath = "Gimmick.FallBorder.MainComponent";
			break;
		case LD_VOLUME_TYPE::WATER_AREA:
			strMainPath = "Gimmick.WaterArea.MainComponent";
			break;
		default:
			break;
		}

		if (!JsonUtils::Try_ReadFloat3Array(jEntry, "AreaCenter", &pDesc->Volume.vAreaCenter))
		{
			if (!JsonUtils::Try_ReadFloat3Array(jEntry, "Center", &pDesc->Volume.vAreaCenter))
			{
				if (!strMainPath.empty())
					JsonUtils::Try_ReadFloat3Array(jEntry, strMainPath + ".AreaCenter", &pDesc->Volume.vAreaCenter);

				if (0.f == pDesc->Volume.vAreaCenter.x
					&& 0.f == pDesc->Volume.vAreaCenter.y
					&& 0.f == pDesc->Volume.vAreaCenter.z)
				{
					pDesc->Volume.vAreaCenter = pDesc->vParsedPosition;
				}
			}
		}

		if (!JsonUtils::Try_ReadFloat4Array(jEntry, "AreaRot", &pDesc->Volume.qAreaRot))
		{
			if (!strMainPath.empty())
				JsonUtils::Try_ReadFloat4Array(jEntry, strMainPath + ".AreaRot", &pDesc->Volume.qAreaRot);

			if (0.f == pDesc->Volume.qAreaRot.x
				&& 0.f == pDesc->Volume.qAreaRot.y
				&& 0.f == pDesc->Volume.qAreaRot.z
				&& 0.f == pDesc->Volume.qAreaRot.w)
			{
				pDesc->Volume.qAreaRot = pDesc->qParsedRotation;
			}
		}

		if (!JsonUtils::Try_ReadFloat3Array(jEntry, "AreaSize", &pDesc->Volume.vAreaSize))
			if (!JsonUtils::Try_ReadFloat3Array(jEntry, "Size", &pDesc->Volume.vAreaSize))
				if (!strMainPath.empty())
					JsonUtils::Try_ReadFloat3Array(jEntry, strMainPath + ".Size", &pDesc->Volume.vAreaSize);

		JsonUtils::Try_ReadFloat(jEntry, "Height", &pDesc->Volume.fHeight);
		if (!strMainPath.empty() && 0.f == pDesc->Volume.fHeight)
			JsonUtils::Try_ReadFloat(jEntry, strMainPath + ".Height", &pDesc->Volume.fHeight);

		JsonUtils::Try_ReadFloat3List(jEntry, "Points", &pDesc->Volume.Points);
		if (!strMainPath.empty() && pDesc->Volume.Points.empty())
			JsonUtils::Try_ReadFloat3List(jEntry, strMainPath + ".Points", &pDesc->Volume.Points);

		JsonUtils::Try_ReadString(jEntry, "TargetKind", &pDesc->Volume.strTargetKind);
		if (!strMainPath.empty() && pDesc->Volume.strTargetKind.empty())
			JsonUtils::Try_ReadString(jEntry, strMainPath + ".TargetKind", &pDesc->Volume.strTargetKind);

		return;
	}

	if (Equals_NoCase(strObjectName, L"GuideMovieArea")
		|| Equals_NoCase(strObjectName, L"SlideInfoArea"))
	{
		pDesc->eCategory = LD_CATEGORY::GUIDE_AREA;
		JsonUtils::Try_ReadString(jEntry, "GuideMovieKind", &pDesc->GuideArea.strGuideMovieKind);

		if (!JsonUtils::Try_ReadFloat3Array(jEntry, "AreaCenter", &pDesc->GuideArea.vAreaCenter))
			pDesc->GuideArea.vAreaCenter = pDesc->vParsedPosition;

		if (!JsonUtils::Try_ReadFloat4Array(jEntry, "AreaRot", &pDesc->GuideArea.qAreaRot))
			pDesc->GuideArea.qAreaRot = pDesc->qParsedRotation;

		JsonUtils::Try_ReadFloat3Array(jEntry, "AreaSize", &pDesc->GuideArea.vAreaSize);
		JsonUtils::Try_ReadUInt(jEntry, "MinPlayFrame", &pDesc->GuideArea.iMinPlayFrame);
		JsonUtils::Try_ReadUInt(jEntry, "WaitFrame", &pDesc->GuideArea.iWaitFrame);
		JsonUtils::Try_ReadUInt(jEntry, "RelationActorUid", &pDesc->GuideArea.iRelationActorUid);
		return;
	}

	const LD_AUDIO_AREA_TYPE eAudioAreaType = Resolve_AudioAreaType(strObjectName);
	if (eAudioAreaType != LD_AUDIO_AREA_TYPE::UNKNOWN)
	{
		pDesc->eCategory = LD_CATEGORY::AUDIO_AREA;
		pDesc->AudioArea.eAudioAreaType = eAudioAreaType;

		JsonUtils::Try_ReadUInt(jEntry, "SoundId", &pDesc->AudioArea.iSoundId);
		JsonUtils::Try_ReadString(jEntry, "VariationType", &pDesc->AudioArea.strVariationType);
		JsonUtils::Try_ReadString(jEntry, "ShapeType", &pDesc->AudioArea.strShapeType);

		if (!JsonUtils::Try_ReadFloat3Array(jEntry, "AreaCenter", &pDesc->AudioArea.vAreaCenter))
			pDesc->AudioArea.vAreaCenter = pDesc->vParsedPosition;

		if (!JsonUtils::Try_ReadFloat4Array(jEntry, "AreaRot", &pDesc->AudioArea.qAreaRot))
			pDesc->AudioArea.qAreaRot = pDesc->qParsedRotation;

		JsonUtils::Try_ReadFloat3Array(jEntry, "AreaSize", &pDesc->AudioArea.vAreaSize);
		JsonUtils::Try_ReadUInt(jEntry, "FadeInFrame", &pDesc->AudioArea.iFadeInFrame);
		JsonUtils::Try_ReadUInt(jEntry, "InactivateFrame", &pDesc->AudioArea.iInactivateFrame);
		return;
	}

	pDesc->eCategory = LD_CATEGORY::UNSUPPORTED;
}

void CLevelDesign_Parser::Fill_LadderFields(const json& jEntry, LD_LADDER_DESC* pDesc)
{
	if (nullptr == pDesc)
		return;

	pDesc->eCategory = LD_CATEGORY::GIMMICK;
	JsonUtils::Try_ReadUInt(jEntry, "Length", &pDesc->iLength);
}

void CLevelDesign_Parser::Build_TransformDesc(LD_OBJECT_DESC* pDesc)
{
	if (nullptr == pDesc)
		return;

	const _float3& S = pDesc->vParsedScale;
	const _float4& Q = pDesc->qParsedRotation;
	const _float3& T = pDesc->vParsedPosition;

	const _matrix matScale = XMMatrixScaling(S.x, S.y, S.z);
	const _matrix matRot = XMMatrixRotationQuaternion(XMLoadFloat4(&Q));
	const _matrix matTrans = XMMatrixTranslation(T.x, T.y, T.z);
	const _matrix matWorld = matScale * matRot * matTrans;

	XMStoreFloat4(&pDesc->vRight, matWorld.r[0]);
	XMStoreFloat4(&pDesc->vUp, matWorld.r[1]);
	XMStoreFloat4(&pDesc->vLook, matWorld.r[2]);
	XMStoreFloat4(&pDesc->vPosition, matWorld.r[3]);

	pDesc->fScale = 1.f;
}

NS_END