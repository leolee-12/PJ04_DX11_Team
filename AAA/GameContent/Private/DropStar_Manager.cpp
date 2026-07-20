#include "DropStar_Manager.h"
#include "GameInstance.h"
#include "GameContent_const.h"
#include "DropStar.h"
#include "GameObject_Factory.h"

IMPLEMENT_SINGLETON(CDropStar_Manager)

CDropStar_Manager::CDropStar_Manager()
	: m_pGameInstance_Proxy { CGameInstance::GetProxy() }
{
}

HRESULT CDropStar_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (FAILED(Register_At_Static(CDropStar::PROTOTYPE_TAG, pDevice, pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDropStar_Manager::Register_At_Static(const _tchar* szProtoTag, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	const _uint iStatic = ETOUI(LEVEL::STATIC);
	if (m_pGameInstance_Proxy->Has_Prototype(iStatic, szProtoTag))
		return S_OK;

	auto* pReg = CGameObject_Factory::GetInstance()->Get_Registration(szProtoTag);
	if (nullptr == pReg)
		return E_FAIL;

	CGameObject_Factory::GetInstance()->LoadResource(szProtoTag, m_pGameInstance_Proxy, pDevice, pContext, iStatic);

	return m_pGameInstance_Proxy->Add_Prototype(iStatic, szProtoTag, pReg->CreatorFunc(pDevice, pContext));
}

HRESULT CDropStar_Manager::Spawn(_uint iTargetLevel, const _float3& vPos, const _float3& vLook, const _float3& vDir, _float fDelay , CDropStar** ppOut)
{
    const _wstring strKey = CDropStar::PROTOTYPE_TAG;

    auto it = m_Dormant.find(POOL_KEY{ iTargetLevel, strKey });
    if (it != m_Dormant.end() && !it->second.empty())
    {
        CDropStar* p = it->second.back();
        it->second.pop_back();
        p->Activate(vPos, vLook, vDir, fDelay);
        if (ppOut) *ppOut = p;
        return S_OK;
    }

    const _wstring strObjTag =
        strKey + L"#" + std::to_wstring(m_iSpawnCounter++);

    CGameObject* pObj = nullptr;
    if (FAILED(m_pGameInstance_Proxy->Add_GameObject_Return(
        &pObj, ETOUI(LEVEL::STATIC), CDropStar::PROTOTYPE_TAG,
        iTargetLevel, DROPSTAR_LAYER_TAG, strObjTag, nullptr)))
        return E_FAIL;

    CDropStar* pStar = dynamic_cast<CDropStar*>(pObj);
    if (!pStar)
    {
        m_pGameInstance_Proxy->Destroy_GameObject(pObj);
        return E_FAIL;
    }

    pStar->Set_Pool(this, iTargetLevel, strKey);
    pStar->Activate(vPos, vLook, vDir, fDelay);
    if (ppOut)
        *ppOut = pStar;

    return S_OK;
}

HRESULT CDropStar_Manager::Spawn_Pattern(_uint iLayerLevel, _fmatrix matCaster, const STAR_SPAWN_DESC& Desc)
{
    if (Desc.iCount == 0)
        return S_OK;

    _vector vCenter = XMVector3TransformCoord(XMLoadFloat3(&Desc.vLocalOffset), matCaster);
    _vector vLook = XMVector3Normalize(matCaster.r[2]);

    //XMStoreFloat3(&vFace, XMVectorNegate(
    //    XMLoadFloat4(m_pGameInstance_Proxy->Get_CamLook())));

    _float3 vFace{};
    XMStoreFloat3(&vFace, vLook);

    for (_uint i = 0; i < Desc.iCount; ++i)
    {
        _vector vPos = Compute_StarPos(vCenter, vLook, Desc, i);

        _float fDelay =
            (Desc.eType == STAR_SPAWN_TYPE::CIRCLE)
            ? Desc.fDelayStart
            + m_pGameInstance_Proxy->RandomFloat(
                0.f, Desc.fDelayStep * Desc.iCount)
            : Desc.fDelayStart + Desc.fDelayStep * i;

        _float3 vPos3{};
        XMStoreFloat3(&vPos3, vPos);

        _float3 vLaunch{};
        if (Desc.fLaunchSpeed != 0.f)
        {
            _vector vOut = vPos - vCenter;
            vOut = XMVectorSetY(vOut, 0.f);
            if (XMVectorGetX(XMVector3LengthSq(vOut)) > 1e-4f)
            {
                vOut = XMVector3Normalize(vOut) * Desc.fLaunchSpeed;
                XMStoreFloat3(&vLaunch, vOut);
            }
        }

        Spawn(iLayerLevel, vPos3, vFace, vLaunch, fDelay);
    }

    return S_OK;
}

HRESULT CDropStar_Manager::Spawn_Preset(_uint iLayerLevel, _fmatrix matCaster, _uint iPreset, const _float3& vOffset)
{
    return S_OK;
}

CDropStar_Manager::STAR_SPAWN_DESC CDropStar_Manager::Get_Preset(STAR_SPAWN_PRESET ePreset) const
{
    return STAR_SPAWN_DESC();
}

void CDropStar_Manager::Return(_uint iLevel, const _wstring& strKey, CDropStar* pStar)
{
	m_Dormant[POOL_KEY{ iLevel, strKey }].push_back(pStar);
}

void CDropStar_Manager::Clear_Level(_uint iLevel)
{
	for (auto it = m_Dormant.begin(); it != m_Dormant.end(); )
		it = (it->first.iLevel == iLevel) ? m_Dormant.erase(it)  : std::next(it);
}

_vector CDropStar_Manager::Compute_StarPos(_fvector vCenter, _fvector vLook, const CDropStar_Manager::STAR_SPAWN_DESC& Desc, _uint iIndex)
{
    // CIRCLE ( 원 내부 면적에 균일하게 살포 )
    if (Desc.eType == STAR_SPAWN_TYPE::CIRCLE)
    {
        _float fR = Desc.fRange * sqrtf(m_pGameInstance_Proxy->RandomFloat(0.f, 1.f));
        _float fAngle = m_pGameInstance_Proxy->RandomFloat(0.f, XM_2PI);
        _float fSin = 0.f, fCos = 0.f;
        XMScalarSinCos(&fSin, &fCos, fAngle);

        return vCenter + XMVectorSet(1.f, 0.f, 0.f, 0.f) * (fR * fCos) + XMVectorSet(0.f, 0.f, 1.f, 0.f) * (fR * fSin);
    }

    // SWEEP : LOOK 기준 fStartDeg에서 시작 + fSweepDeg CW/CCW 살포
    _float fT = (Desc.iCount > 1)
        ? static_cast<_float>(iIndex) / (Desc.iCount - 1)
        : 0.f;

    _float fDeg = Desc.fStartDeg + fT * Desc.fSweepDeg;

    _matrix matRot = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fDeg));
    _vector vDir = XMVector3Normalize(XMVector3TransformNormal(vLook, matRot));

    _float fJitter = m_pGameInstance_Proxy->RandomFloat(
        -Desc.fJitter, Desc.fJitter);

    return vCenter + vDir * (Desc.fRange + fJitter);
}

void CDropStar_Manager::Free()
{
	m_Dormant.clear();
	Safe_Release(m_pGameInstance_Proxy);
	__super::Free();
}
