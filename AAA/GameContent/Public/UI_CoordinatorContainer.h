#pragma once
#include "GameContent_Defines.h"
#include "UI_MovableContainer.h"

NS_BEGIN(Client)

// 자식 컨테이너를 소유/구동/직렬화하는 코디네이터(프리팹) 베이스.
// 자식은 그룹행렬을 물려받아 코디네이터를 따라 통째로 움직임.
// 그대로 프로토타입으로 써도 되고, 특수 코디네이터(미션보드 등)의 베이스로 상속해도 됨.
class CLIENT_DLL CUICoordinatorContainer : public CUIMovableContainer
{
    GENERATED_BODY(CUICoordinatorContainer)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_Coordinator";

protected:
    CUICoordinatorContainer(ID3D11Device*, ID3D11DeviceContext*);
    CUICoordinatorContainer(const CUICoordinatorContainer&);
    virtual ~CUICoordinatorContainer() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float) override;
    virtual void Update(_float) override;
    virtual void Late_Update(_float) override;

    virtual json Serialize() const override;
    virtual void Deserialize_Internal(const json& j) override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    // 자식 컨테이너 추가/조회 (파츠의 Add_Part 대응)
    HRESULT              Add_Child(_uint iProtoLevel, const _wstring& strProtoTag, const _wstring& strChildTag);
    CUIMovableContainer* Find_Child(const _wstring& strChildTag) const;
    const vector<_wstring>& Get_ChildOrder() const { return m_ChildOrder; }

    HRESULT  Rename_Child(const _wstring& strOldTag, const _wstring& strNewTag);
    HRESULT  Remove_Child(const _wstring& strChildTag);

protected:
    typedef struct { _uint iLevel; _wstring strProtoTag; } CHILD_PROTO;
    unordered_map<_wstring, CUIMovableContainer*> m_Children;
    unordered_map<_wstring, CHILD_PROTO>          m_ChildProto;
    vector<_wstring>                              m_ChildOrder;
    void Clear_Children();

public:
    static CUICoordinatorContainer* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END