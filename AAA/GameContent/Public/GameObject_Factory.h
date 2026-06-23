#pragma once
#include "GameContent_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameObject;
class CGameInstance_Proxy;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CGameObject_Factory final : public CBase
{
	DECLARE_SINGLETON(CGameObject_Factory)

public:
	using Creator			= function<CBase* (ID3D11Device*, ID3D11DeviceContext*)>;
	using ResourceLoader    = function<void(CGameInstance_Proxy*, ID3D11Device*, ID3D11DeviceContext*, _uint)>;

	typedef struct tagGameObjectRegistration
	{
		Creator			CreatorFunc;
		ResourceLoader  ResourceLoader;
        wstring         strCategory;
    }GAMEOBJECT_REGISTRATION;

private:
	CGameObject_Factory() = default;
	virtual ~CGameObject_Factory() = default;

public:
    void Register(const wstring& strTag, const wstring& strCategory, Creator creator, ResourceLoader loader)
    {
        OutputDebugStringW(L"Register called!\n");
        auto& reg = m_Registrations[strTag];
        reg.CreatorFunc     = creator;
        reg.ResourceLoader  = loader;
        reg.strCategory     = strCategory;
        m_Registrations[strTag] = { creator, loader, strCategory };
    }

    GAMEOBJECT_REGISTRATION* Get_Registration(const wstring& strTag)
    {
        auto iter = m_Registrations.find(strTag);
        if (iter == m_Registrations.end())
            return nullptr;
        return &iter->second;
    }

    void LoadResource(const wstring& strTag, CGameInstance_Proxy* pProxy, ID3D11Device* pDevice, ID3D11DeviceContext*
        pContext, _uint iLevelIndex)
    {
        auto iter = m_Registrations.find(strTag);
        if (iter == m_Registrations.end())
            return;

        iter->second.ResourceLoader(pProxy, pDevice, pContext, iLevelIndex);
    }

    void Copy_RegisteredTags(vector<wstring>* pOutTags);
    void Copy_TagsByCategory(map<wstring, vector<wstring>>* pOutMap);

    void RegisterAll();

private:
    unordered_map<wstring, GAMEOBJECT_REGISTRATION> m_Registrations;

private:
    void Register_UI();
	void Register_Camera();
	void Register_Test();
    void Register_Container();
    void Register_UIContainer();
	void Register_NonAnimObject();
    void Register_AnimObject();
	void Register_Effect();

    void Register_MiniBoss();
    void Register_MainBoss();

public:
    virtual void Free() override;
};

NS_END
