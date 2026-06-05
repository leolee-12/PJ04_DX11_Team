#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"
#include "UI_Image.h"

NS_BEGIN(Client)

class CUI_Title final : public CUIContainerObject
{
	GENERATED_BODY(CUI_Title)

public:
	typedef struct tagUITitleDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_bool							bCreateTitleImage = { false };
		const _tchar*					szTitleImagePartTag = { L"TitleImage" };
		CUI_Image::UI_IMAGE_DESC		TitleImageDesc = {};
	}UI_TITLE_DESC;

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_Title";
	static constexpr const wchar_t* PART_TAG_TITLE_IMAGE = L"TitleImage";

private:
	CUI_Title(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Title(const CUI_Title& Prototype);
	virtual ~CUI_Title() = default;

public:
	virtual HRESULT						Initialize_Prototype() override;
	virtual HRESULT						Initialize(void* pArg) override;

	virtual void						Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	static CUI_Title*					Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject*						Clone(void* pArg) override;
	
protected:
	virtual void						Free() override;
};

NS_END
