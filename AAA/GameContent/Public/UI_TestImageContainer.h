#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"
#include "UI_Image.h"

NS_BEGIN(Client)

class CUI_TestImageContainer final : public CUIContainerObject
{
    GENERATED_BODY(CUI_TestImageContainer)

public:
    typedef struct tagUITestImageContainerDesc : public CGameObject::GAMEOBJECT_DESC
    {
        _bool                                           bCreateImagePart = { false };
        const _tchar* szPartTag = { L"Image" };
        CUI_Image::UI_IMAGE_DESC        ImageDesc = {};
    } UI_TESTIMAGE_CONTAINER_DESC;

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_TestImageContainer";

private:
    CUI_TestImageContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_TestImageContainer(const CUI_TestImageContainer& Prototype);
    virtual ~CUI_TestImageContainer() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    static CUI_TestImageContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

private:
    virtual void Free() override;
};

NS_END