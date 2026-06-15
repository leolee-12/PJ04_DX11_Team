#pragma once
#include "AnimUITool_Defines.h"
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
class CUIContainerObject;
class CUIPartObject;
NS_END

NS_BEGIN(physx)
class PxTriangleMesh;
class PxRigidStatic;
NS_END

NS_BEGIN(AnimUITool)

class CEditCamera;
class CEdit_Grid;

class CLevel_Tool final : public Engine::CLevel
{
private:
    CLevel_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CLevel_Tool() = default;

public:
    virtual HRESULT             Initialize() override;
    virtual void                Update(_float fTimeDelta) override;
    virtual HRESULT             Render() override;

    const vector<UI_CONTAINER_ENTRY>& Get_UIContainerEntries() const { return m_UIContainers; }     // 읽기 전용
    vector<UI_CONTAINER_ENTRY>& Get_UIContainerEntries() { return m_UIContainers; }

    void                        Set_CameraActive(_bool b);
    void                        Set_GridVisible(_bool bVisible) { m_bGridVisible = bVisible; }
    void                        Set_PreviewVisible(_bool bVisible);

    CGameObject*                Get_Preview() const { return m_pPreview; }

    CGameObject*                Load_Preview(const _wstring& strYshPath);
    CGameObject*                Load_Kirby();
    void                        Clear_Preview();
    void                        Recalc_CameraProj();

    HRESULT                     Save_UIContainer(CGameObject* pContainer, const _float2& vDesignSize, const _wstring& strFileName);
    CGameObject*                Load_UIContainerByPath(const _wstring& strFullPath, _float2& vOutDesignSize);

    void                        Delete_UIContainer(CUIContainerObject* pContainer);
    CGameObject*                Add_UIContainer();
    CUIPartObject*              Add_UIPart(CGameObject* pContainer,UI_PART_TYPE eType = UI_PART_TYPE::IMAGE, _wstring* pOutPartTag = nullptr);
    HRESULT                     Remove_UIPart(CGameObject* pContainer, const _wstring& strPartTag);
    HRESULT                     Rename_UIPart(CGameObject* pContainer, const _wstring& strOldTag, const _wstring& strNewTag);

    HRESULT                     Save_UIManifest(const _wstring& strManifestPath);
    HRESULT                     Load_UIManifest(const _wstring& strManifestPath);

    _wstring                    Get_AuthoredProtoTag(CGameObject* pContainer);
    void                        Set_AuthoredProtoTag(CGameObject* pContainer, const _wstring& strProtoTag);

    _wstring                    Register_TextureProto(const _wstring& strTexturePath);
    _wstring                    Register_TextureProto(const _wstring& strTextureProtoTag, const _wstring& strTexturePath, _uint iLevelIndex);

    CGameObject*                Spawn_Object(const _wstring& strProtoTag, const _wstring& strLayerTag, const _wstring& strName, void* pArg = nullptr);
    void                        Clear_Spawned();

    const vector<CGameObject*>& Get_SpawnedObjects() const { return m_SpawnedObjects; }
    void                        Destroy_Spawned(CGameObject* pObject);

private:
    CEditCamera*                m_pCamera = { nullptr };
    CEdit_Grid*                 m_pGrid = { nullptr };
    physx::PxTriangleMesh*      m_pTestGroundMesh = { nullptr };
    physx::PxRigidStatic*       m_pTestGroundActor = { nullptr };

    CGameObject*                m_pPreview = { nullptr };
    map<_wstring, _wstring>     m_ModelTags;
    _uint                       m_iTagCounter = { 0 };
    _bool                       m_bGridVisible = { true };
    vector<UI_CONTAINER_ENTRY> m_UIContainers;

    unordered_map<CGameObject*, _wstring> m_AuthoredProtoTags;
    unordered_map<_wstring, _wstring> m_TextureProtoPaths;  // ProtoTag -> Path
    _uint                       m_iUIContainerCounter = { 0 };
    _uint                       m_iUIPartCounter = { 0 };

    vector<CGameObject*>        m_SpawnedObjects;

private:
    virtual HRESULT             Ready_Events() override { return S_OK; }
    HRESULT                     Ready_Lights();
    HRESULT                     Ready_Camera();
    HRESULT                     Ready_Grid();
    HRESULT                     Ready_TestGround();
    void                        Release_TestGround();
    HRESULT                     Ready_PreviewShaders();

    void                        Track_UIContainer(CGameObject* pObject, const _wstring& strPath, const _float2& vDesignSize);
    void                        UnTrack_UIContainer(CGameObject* pObject);



public:
    static CLevel_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void                Free() override;
};

NS_END