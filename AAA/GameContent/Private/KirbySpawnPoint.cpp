#include "KirbySpawnPoint.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"
#include "GameObject_Factory.h"
#include "Kirby.h"

CKirbySpawnPoint::CKirbySpawnPoint(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CKirbySpawnPoint::CKirbySpawnPoint(const CKirbySpawnPoint& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CKirbySpawnPoint::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CKirbySpawnPoint::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_bClone = true;
    return S_OK;
}

void CKirbySpawnPoint::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_bWarped && !m_bHudRefreshed)
    {
        m_bHudRefreshed = true;
        m_pGameInstance_Proxy->Publish(EventTag::Kirby_HUD_Refresh, nullptr);
    }
}

void CKirbySpawnPoint::On_Deserialized()
{
    if (FAILED(Ensure_Kirby()))
        return;

    KIRBY_LEVEL_SPAWN_DESC desc{};
    XMStoreFloat3(&desc.vPosition, m_pTransformCom->Get_State(STATE::POSITION));
    XMStoreFloat3(&desc.vLook, XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)));
    desc.pSpawner = this;
    desc.iLevelIndex = Get_LevelIndex();
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_LevelSpawn, &desc);

    m_bWarped = true;
}

HRESULT CKirbySpawnPoint::Ensure_Kirby()
{
    // 이미 살아있는 커비가 있으면 그대로 사용
    PLAYER_QUERY q{};
    m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &q);
    if (nullptr != q.pPlayer)
        return S_OK;

    // 최초 1회: STATIC 인덱스에 리소스와 프로토타입을 등록하고 생성
    // (CameraTool의 Ready_Kirby와 동일한 패턴)
    auto* pFactory = CGameObject_Factory::GetInstance();
    if (nullptr == pFactory)
        return E_FAIL;

    const _uint iStatic = ETOUI(LEVEL::STATIC);

    pFactory->LoadResource(CKirby::PROTOTYPE_TAG, m_pGameInstance_Proxy, m_pDevice, m_pContext, iStatic);

    if (!m_pGameInstance_Proxy->Has_Prototype(iStatic, CKirby::PROTOTYPE_TAG))
    {
        auto* pReg = pFactory->Get_Registration(CKirby::PROTOTYPE_TAG);
        if (nullptr == pReg)
            return E_FAIL;

        m_pGameInstance_Proxy->Add_Prototype(iStatic, CKirby::PROTOTYPE_TAG,
            dynamic_cast<CGameObject*>(pReg->CreatorFunc(m_pDevice, m_pContext)));
    }

    CKirby::KIRBY_BODY_DESC KirbyDesc{};
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject(iStatic, CKirby::PROTOTYPE_TAG,
        iStatic, L"Layer_Kirby", L"Kirby", &KirbyDesc)))
        return E_FAIL;

    return S_OK;
}

CKirbySpawnPoint* CKirbySpawnPoint::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirbySpawnPoint* pInstance = new CKirbySpawnPoint(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CKirbySpawnPoint");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CKirbySpawnPoint::Clone(void* pArg)
{
    CKirbySpawnPoint* pInstance = new CKirbySpawnPoint(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CKirbySpawnPoint");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CKirbySpawnPoint::Free()
{
    if (m_bClone && m_pGameInstance_Proxy)
    {
        KIRBY_LEVEL_SLEEP_DESC desc{};
        desc.pSpawner = this;
        m_pGameInstance_Proxy->Publish(EventTag::Kirby_LevelSleep, &desc);
    }

    __super::Free();
}