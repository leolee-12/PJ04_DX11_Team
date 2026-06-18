#include "LevelDesign_Unsupported.h"

#include "GameInstance.h"

namespace
{
    constexpr _float kMinDebugAxis = 0.001f;

    _float AbsAxis(_float fValue)
    {
        return fValue < 0.f ? -fValue : fValue;
    }

    _bool Has_UsableDebugSize(const _float3& vSize)
    {
        return AbsAxis(vSize.x) > kMinDebugAxis
            && AbsAxis(vSize.y) > kMinDebugAxis
            && AbsAxis(vSize.z) > kMinDebugAxis;
    }

    _float3 Make_SafeDebugSize(const _float3& vSize)
    {
        _float3 Result =
        {
                AbsAxis(vSize.x),
                AbsAxis(vSize.y),
                AbsAxis(vSize.z)
        };

        if (Result.x <= kMinDebugAxis) Result.x = 1.f;
        if (Result.y <= kMinDebugAxis) Result.y = 1.f;
        if (Result.z <= kMinDebugAxis) Result.z = 1.f;

        return Result;
    }

    void Resolve_DebugBoxDesc(const Client::LD_PARSED_OBJECT& Parsed, _float3* pOutCenterLocal, _float3* pOutSize)
    {
        if (nullptr == pOutCenterLocal || nullptr == pOutSize)
            return;

        *pOutCenterLocal = { 0.f, 0.f, 0.f };
        *pOutSize = Make_SafeDebugSize(Parsed.vParsedScale);

        auto Try_ApplyArea = [&](const _float3& vAreaCenter, const _float3& vAreaSize) -> _bool
            {
                if (!Has_UsableDebugSize(vAreaSize))
                    return false;

                pOutCenterLocal->x = vAreaCenter.x - Parsed.vParsedPosition.x;
                pOutCenterLocal->y = vAreaCenter.y - Parsed.vParsedPosition.y;
                pOutCenterLocal->z = vAreaCenter.z - Parsed.vParsedPosition.z;

                *pOutSize = Make_SafeDebugSize(vAreaSize);
                return true;
            };

        if (Try_ApplyArea(Parsed.Volume.vAreaCenter, Parsed.Volume.vAreaSize))
            return;

        if (Try_ApplyArea(Parsed.GuideArea.vAreaCenter, Parsed.GuideArea.vAreaSize))
            return;

        if (Try_ApplyArea(Parsed.AudioArea.vAreaCenter, Parsed.AudioArea.vAreaSize))
            return;

        if (Has_UsableDebugSize(Parsed.Portal.vRestartAreaSize))
        {
            *pOutCenterLocal = Parsed.Portal.vRestartAreaOffs;
            *pOutSize = Make_SafeDebugSize(Parsed.Portal.vRestartAreaSize);
        }
    }
}

NS_BEGIN(Client)

CLevelDesign_Unsupported::CLevelDesign_Unsupported(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevelDesignObject(pDevice, pContext)
{
}

CLevelDesign_Unsupported::CLevelDesign_Unsupported(const CLevelDesign_Unsupported& Prototype)
	: CLevelDesignObject(Prototype)
{
}

HRESULT CLevelDesign_Unsupported::Initialize_Prototype()
{
	return __super::Initialize_Prototype();
}

HRESULT CLevelDesign_Unsupported::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return E_FAIL;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

#ifdef _DEBUG
    if (FAILED(Ready_DebugCollider(static_cast<const LD_PARSED_OBJECT*>(pArg))))
        return E_FAIL;
#endif

    return S_OK;
}

void CLevelDesign_Unsupported::Late_Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

#ifdef _DEBUG
    if (m_pDebugCollider && m_pTransformCom)
    {
        m_pDebugCollider->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
        m_pGameInstance_Proxy->Add_DebugComponent(m_pDebugCollider);
    }
#endif
}

void CLevelDesign_Unsupported::Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData)
{
	if (nullptr == pOutData)
		return;

	pOutData->strPrototypeTag = PROTOTYPE_TAG;
}

#ifdef _DEBUG
HRESULT CLevelDesign_Unsupported::Ready_DebugCollider(const LD_PARSED_OBJECT* pParsedDesc)
{
    if (nullptr == pParsedDesc)
        return S_OK;

    _float3 vCenterLocal = {};
    _float3 vSize = {};
    Resolve_DebugBoxDesc(*pParsedDesc, &vCenterLocal, &vSize);

    m_pDebugCollider = Add_Component<CCollider>(
        L"Com_DebugCollider",
        CCollider::Create(m_pDevice, m_pContext, COLLIDER::AABB));

    if (nullptr == m_pDebugCollider)
        return E_FAIL;

    CCollider::COLLIDER_DESC ColliderDesc{};
    ColliderDesc.pOwner = this;
    ColliderDesc.vCenter = vCenterLocal;
    ColliderDesc.vSize = vSize;

    if (FAILED(m_pDebugCollider->Initialize(&ColliderDesc)))
        return E_FAIL;

    return S_OK;
}
#endif

CLevelDesign_Unsupported* CLevelDesign_Unsupported::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelDesign_Unsupported* pInstance = new CLevelDesign_Unsupported(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CLevelDesign_Unsupported");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelDesign_Unsupported::Clone(void* pArg)
{
	CLevelDesign_Unsupported* pInstance = new CLevelDesign_Unsupported(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CLevelDesign_Unsupported");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelDesign_Unsupported::Free()
{
	__super::Free();
}

NS_END