#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

// 레벨마다 배치되어 STATIC 레이어의 커비를 자기 위치로 재배치하는 객체.
// 최초 진입 시 커비가 없으면 STATIC 인덱스에 생성까지 담당한다.
class CKirbySpawnPoint final : public CGameObject
{
    GENERATED_BODY(CKirbySpawnPoint)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_KirbySpawnPoint";

private:
    CKirbySpawnPoint(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CKirbySpawnPoint(const CKirbySpawnPoint& Prototype);
    virtual ~CKirbySpawnPoint() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    // 베이스 Deserialize가 Transform을 채운 직후 1회 호출됨
    virtual void On_Deserialized() override;

private:
    HRESULT Ensure_Kirby();

private:
    _bool m_bClone = { false };
    _bool m_bWarped = { false };
    _bool m_bHudRefreshed = { false };

public:
    static CKirbySpawnPoint* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END