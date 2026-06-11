#pragma once
#include "Panel.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(MapTool)
class CLevel_Edit;

class CPanel_Hierarchy final : public CPanel
{
private:
	static constexpr _uint	SEARCH_BUF_SIZE = { 128 };

private:
	CPanel_Hierarchy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPanel_Hierarchy() = default;

public:
	virtual void	Render() override;

private:
	struct HIERARCHY_OBJECT_CACHE
	{
		string          strPrototypeUtf8;
		string          strNameUtf8;
		string          strSearchKeyLower;
		CGameObject* pObject = nullptr;
	};

	struct HIERARCHY_LAYER_CACHE
	{
		wstring                         strLayerTag;
		string                          strLayerNameUtf8;
		string                          strLayerNameLower;
		vector<HIERARCHY_OBJECT_CACHE>  Objects;
	};

	struct HIERARCHY_FILTER_CACHE
	{
		vector<_uint> MatchedObjectIndices;
	};

private:
	_char   m_szSearch[SEARCH_BUF_SIZE] = {};
	_uint   m_iCachedHierarchyRevision = static_cast<_uint>(-1);
	string  m_strCachedSearchLower = {};

	vector<HIERARCHY_LAYER_CACHE>  m_HierarchyCache;
	vector<HIERARCHY_FILTER_CACHE> m_FilterCache;

	_uint   m_iCachedVisibleLayerCount = {};
	_uint   m_iCachedMatchedObjectCount = {};
	_uint	m_iCachedSelectionRevision = static_cast<_uint>(-1);
	CGameObject* m_pPendingScrollTarget = nullptr;

private:
	void Sync_HierarchyCache(CLevel_Edit* pLevel);
	void Rebuild_FilterCache(const string& strLowerSearch);
	void Invalidate_SearchCache();

public:
	static CPanel_Hierarchy* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

protected:
	virtual void	Free() override;
};

NS_END