#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"
#include "UIAnimatorCom.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_GenericContainer final : public CUIContainerObject, public IUIAnimatorOwner
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
	virtual void Update(_float fTimeDelta) override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

	virtual CUIAnimatorCom* Get_UIAnimatorCom() override { return m_pUIAnimatorCom; }
	virtual const CUIAnimatorCom* Get_UIAnimatorCom() const override { return m_pUIAnimatorCom; }

protected:
	virtual void On_Deserialized() override;
	virtual void On_UIPartsChanged() override;

private:
	void Bind_UIAnimator();

private:
	CUIAnimatorCom* m_pUIAnimatorCom = { nullptr };

public:
    static CUI_GenericContainer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END