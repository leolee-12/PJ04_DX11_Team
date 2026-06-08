#pragma once
#ifndef AnimUITool_Struct_h__
#define AnimUITool_Struct_h__

NS_BEGIN(Engine)
class CModel;
class CAnimator;
class CUIContainerObject;
class CUIPartObject;
class CGameObject;
NS_END

namespace AnimUITool
{
    typedef struct tagLogDesc
    {
        LOG_LEVEL       eLevel = { LOG_LEVEL::INFO };
        std::string     strMessage = {};
    } LOG_DESC;

    typedef struct tagAnimContext
    {
        CGameObject* pOwner = { nullptr };
        CModel* pModel = { nullptr };
        CAnimator* pAnimator = { nullptr };

        _wstring                    strName = {};           // 모델 표시명 
        _wstring                    strModelPath = {};      // json 경로 파생용

        _int                        iClip = { 0 };
        _bool                       bPlaying = { true };
        _bool                       bLoop = { true };
        _float                      fProgress = { 0.f };

        _int                        iRootBone = { -1 };
        _int                        iSelBone = { -1 };

        _bool                       Valid() const { return  pModel && pAnimator; }
    }ANIM_CONTEXT;

    typedef struct tagUISelection
    {
        Engine::CUIContainerObject* pContainer = { nullptr };
        Engine::CUIPartObject* pPart = { nullptr };
        _wstring                    strPartTag = {};

        _bool Valid() const
        {
            return pContainer && pPart && !strPartTag.empty();
        }

        void Clear()
        {
            pContainer = nullptr;
            pPart = nullptr;
            strPartTag.clear();
        }
    }UI_SELECTION;

    typedef struct tagUIContext
    {
        UI_SELECTION    Selection = {};

        _float2         vDesignSize = { 1600.f, 900.f };
        _float          fGridStep = { 100.f };
        _bool           bShowGrid = { true };

        _bool           bDirty = { false };
        _wstring        strCurrentJsonPath = {};
    }UI_CONTEXT;

    typedef struct tagUIContainerEntry
    {
        Engine::CUIContainerObject* pContainer = { nullptr };
        _wstring        strPath = L""; // 개별 UIContainer json 경로
        _wstring        strLayerTag = L"Layer_UI";
        _float2         vDesignSize = { 1600.f, 900.f };
        _bool           bExport = true;
    }UI_CONTAINER_ENTRY;
}

#endif // AnimUITool_Struct_h__