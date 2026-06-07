#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_GenericContainer final : public CUIContainerObject
{
	GENERATED_BODY(CUI_GenericContainer)

public:
	typedef struct tagUIGenericContainerDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}UI_GENERIC_CONTAINER_DESC;

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_GenericContainer";

private:
	CUI_GenericContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_GenericContainer(const CUI_GenericContainer& Prototype);
	virtual ~CUI_GenericContainer() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    // 저작 전용: 선택 컨테이너에 파트 생성·편입 (protected 래퍼)
    HRESULT Add_Part(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg = nullptr)
    {
        return Add_UIPartObject(iPrototypeLevelIndex, strPrototypeTag, strPartTag, pArg);
    }
    HRESULT Remove_Part(const _wstring& strPartTag);

public:
    static CUI_GenericContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END