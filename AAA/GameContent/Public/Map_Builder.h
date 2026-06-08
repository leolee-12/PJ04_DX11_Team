#pragma once
#include "Base.h"
#include "Map_LoadTypes.h"

NS_BEGIN(Client)
class CMap_ModelResolver;

class CMap_Builder final : public CBase
{
private:
	CMap_Builder(CMap_ModelResolver* pResolver);
	virtual ~CMap_Builder() = default;

public:
	HRESULT Build_FromManifest(const _wstring& strManifestPath, MAP_PACKAGE* pOutPackage);
	HRESULT Build_FromPreset_ForTool(_uint iPresetIndex, MAP_PACKAGE* pOutPackage);

private:
	CMap_ModelResolver* m_pResolver = { nullptr };

private:
	HRESULT Build_StageDesc(const MAP_MANIFEST_DESC& Manifest, MAP_STAGE_DESC* pOutStageDesc);
	HRESULT Build_EnvDescs(const MAP_MANIFEST_DESC& Manifest, vector<ENV_OBJECT_DESC>* pOutEnvDescs, vector<_wstring>* pOutJsonPaths);
	HRESULT Validate_And_Filter(MAP_PACKAGE* pPackage);

public:
	static CMap_Builder* Create(CMap_ModelResolver* pResolver);

private:
	virtual void Free() override;
};

NS_END