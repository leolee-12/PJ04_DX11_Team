#pragma once
#include "Editor_Defines.h"
#include "Level.h"
#include "Camera_AreaData.h"   // Client::CAreaCameraSolver, CAM_*

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)
class CMapStage;
NS_END

NS_BEGIN(Editor)
class CEditCamera;

class CLevel_Edit final : public CLevel
{
public:
    typedef struct tagObjectHandle
    {
        _wstring     strPrototypeTag;
        _wstring     strName;
        CGameObject* pObject;
    }EDITOR_OBJECT_HANDLE;

    enum class SEL { NONE, KIRBY, AREA, NODE };

private:
    CLevel_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CLevel_Edit() = default;

public:
    virtual HRESULT Initialize();
    virtual void    Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

public: // camera document (slim json round-trip)
    Client::CAreaCameraSolver& Get_Solver() { return m_Solver; }
    _bool Load_CameraDoc(const wstring& strPath);
    _bool Save_CameraDoc(const wstring& strPath);

public: // kirby proxy
    _float3& Kirby() { return m_vKirby; }

public: // selection
    SEL  Get_SelType() const { return m_eSel; }
    _int Get_SelArea() const { return m_iSelArea; }
    _int Get_SelRail() const { return m_iSelRail; }
    _int Get_SelNode() const { return m_iSelNode; }
    void Select_None() { m_eSel = SEL::NONE; }
    void Select_Kirby() { m_eSel = SEL::KIRBY; }
    void Select_Area(_int i) { m_eSel = SEL::AREA; m_iSelArea = i; }
    void Select_Node(_int r, _int n) { m_eSel = SEL::NODE; m_iSelRail = r; m_iSelNode = n; }

public: // preview / nav
    _bool Is_Preview() const { return m_bPreview; }
    void  Set_Preview(_bool b);
    void  Set_CameraActive(_bool b);
    void  Get_PreviewViewProj(_float4x4* pView, _float4x4* pProj);

public: // map preview (kept)
    void    Add_Layer(const wstring& strLayerTag);
    HRESULT Load_MapPreview(_uint iPresetIndex);
    HRESULT Load_MapPreviewStage(_uint iPresetIndex);
    HRESULT Load_MapPreviewEnv(_uint iPresetIndex);
    void    Clear_MapPreview();
    void    Clear_MapPreviewStage();
    void    Clear_MapPreviewEnv();
    _uint   Get_MapPreviewPresetCount() const;
    const _char* Get_MapPreviewPresetLabel(_uint iPresetIndex) const;
    _bool   Is_MapPreviewLoaded() const { return nullptr != m_pMapStage || 0 != m_iEnvObjCreatedCount; }
    const _wstring& Get_MapPreviewStatus() const { return m_strMapPreviewStatus; }
    const _wstring& Get_LoadedMapPreviewStageName() const { return m_strLoadedMapStageName; }
    _uint   Get_MapPreviewEnvCreatedCount() const { return m_iEnvObjCreatedCount; }

public: // play mode
    _bool Is_Play() const { return m_bPlay; }
    void  Set_Play(_bool b);

private:
    CEditCamera* m_pCamera = { nullptr };

    Client::CAreaCameraSolver m_Solver;
    _float3 m_vKirby = { 0.f, 1.f, 0.f };

    SEL  m_eSel = { SEL::NONE };
    _int m_iSelArea = { -1 };
    _int m_iSelRail = { -1 };
    _int m_iSelNode = { -1 };

    _bool m_bPreview = { false };

    // map preview members
    CGameObject* m_pSelected = { nullptr };
    unordered_map<wstring, vector<EDITOR_OBJECT_HANDLE>> m_Layers;
    CMapStage* m_pMapStage = { nullptr };
    _wstring    m_strMapPreviewStatus = { L"Map preset not loaded." };
    _wstring    m_strLoadedMapStageName = {};
    _uint       m_iEnvObjCreatedCount = {};
    _int        m_iLoadedMapPresetIndex = { -1 };
    unordered_set<CGameObject*> m_MapPreviewObjects;

    _bool m_bPlay = { false };
    CGameObject* m_pKirby = { nullptr };

private:
    virtual HRESULT Ready_Events() override { return S_OK; }
    HRESULT Ready_EditLights();
    HRESULT Ready_EditCamera();
    HRESULT Ready_Kirby();

    void Clear_MapPreviewLayer(const _wstring& strLayerTag);
    void Add_MapPreviewObjectHandle(const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring&
        strObjectTag, CGameObject* pObject);
    static void On_MapPreviewObjectCreated(void* pContext, CGameObject* pObject,
        const _wstring& strPrototypeTag, const _wstring& strLayerTag, const _wstring& strObjectTag);


public:
    static CLevel_Edit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END