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

HRESULT CDropStar_Manager::Spawn(_uint iTargetLevel, const _float3& vPos, const _float3& vLook, const _float3& vDir, _float fDelay, _float fLife, CDropStar** ppOut)
{
    const _wstring strKey = CDropStar::PROTOTYPE_TAG;

    auto it = m_Dormant.find(POOL_KEY{ iTargetLevel, strKey });
    if (it != m_Dormant.end() && !it->second.empty())
    {
        CDropStar* p = it->second.back();
        it->second.pop_back();
        p->Activate(vPos, vLook, vDir, fDelay, fLife);
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
    pStar->Activate(vPos, vLook, vDir, fDelay, fLife);
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

        Spawn(iLayerLevel, vPos3, vFace, vLaunch, fDelay, Desc.fLifeTime);
    }

    return S_OK;
}

HRESULT CDropStar_Manager::Spawn_Preset(_uint iLayerLevel, _fmatrix matCaster, const _wstring& strPreset, const _float3& vOffset)
{
    STAR_SPAWN_PRESET ePreset = Name_To_Preset(strPreset);
    if (ePreset == STAR_SPAWN_PRESET::PRESET_END)
    {
#ifdef _DEBUG
        OutputDebugStringW((L"[CDropStar_Manager] Unknown preset: " + strPreset  + L"\n").c_str());
#endif
        return E_FAIL;
    }

    STAR_SPAWN_DESC Desc = Get_Preset(static_cast<STAR_SPAWN_PRESET>(ePreset));
    Desc.vLocalOffset.x += vOffset.x;
    Desc.vLocalOffset.y += vOffset.y;
    Desc.vLocalOffset.z += vOffset.z;

    return Spawn_Pattern(iLayerLevel, matCaster, Desc);
}

CDropStar_Manager::STAR_SPAWN_DESC CDropStar_Manager::Get_Preset(STAR_SPAWN_PRESET ePreset) const
{
    STAR_SPAWN_DESC tSpawnDesc{};

    switch (ePreset)
    {
#pragma region 보스 - 고릴라
        case STAR_SPAWN_PRESET::GORILLA_ARM_SWEEP_RIGHT:
        {
            tSpawnDesc.eType = CDropStar_Manager::STAR_SPAWN_TYPE::SWEEP;
            tSpawnDesc.iCount = 8;
            tSpawnDesc.fRange = 15.f;
            tSpawnDesc.fStartDeg = 90.f;
            tSpawnDesc.fSweepDeg = -160.f;
            tSpawnDesc.fDelayStart = 0.f;
            tSpawnDesc.fDelayStep = 0.25f;
            tSpawnDesc.vLocalOffset = { 0.f, 0.5f, 0.f };
            tSpawnDesc.fLaunchSpeed = 0.5f;
            tSpawnDesc.fLifeTime = 2.5f;
            break;
        }
        case STAR_SPAWN_PRESET::GORILLA_ARM_SWEEP_LEFT:
        {
            tSpawnDesc.eType = CDropStar_Manager::STAR_SPAWN_TYPE::SWEEP;
            tSpawnDesc.iCount = 8;
            tSpawnDesc.fRange = 15.f;
            tSpawnDesc.fStartDeg = -90.f;
            tSpawnDesc.fSweepDeg = 160.f;
            tSpawnDesc.fDelayStart = 0.f;
            tSpawnDesc.fDelayStep = 0.25f;
            tSpawnDesc.vLocalOffset = { 0.f, 0.5f, 0.f };
            tSpawnDesc.fLaunchSpeed = 0.5f;
            tSpawnDesc.fLifeTime = 2.5f;
            break;
        }
        case STAR_SPAWN_PRESET::AFTER_GORILLA_ARM_SPIN:
        {
            tSpawnDesc.eType = CDropStar_Manager::STAR_SPAWN_TYPE::SWEEP;
            tSpawnDesc.iCount = 4;
            tSpawnDesc.fRange = 5.f;
            tSpawnDesc.fStartDeg = 0.f;
            tSpawnDesc.fSweepDeg = 270.f;
            tSpawnDesc.fDelayStart = 0.f;
            tSpawnDesc.fDelayStep = 0.f;
            tSpawnDesc.vLocalOffset = { 0.f, 0.5f, 0.f };
            tSpawnDesc.fLaunchSpeed = 3.f;
            tSpawnDesc.fJitter = 0.f;
            tSpawnDesc.fLifeTime = 3.5f;
            break;
        }
        case STAR_SPAWN_PRESET::ROCK_IMPACT:
        {
            // 고릴라 돌 낙하시 효과
            tSpawnDesc.eType = CDropStar_Manager::STAR_SPAWN_TYPE::SWEEP;
            tSpawnDesc.iCount = 4;
            tSpawnDesc.fRange = 3.f;
            tSpawnDesc.fStartDeg = 120.f;
            tSpawnDesc.fSweepDeg = -240.f;
            tSpawnDesc.fDelayStart = 0.f;
            tSpawnDesc.fDelayStep = 0.05f;
            tSpawnDesc.vLocalOffset = { 0.f, 0.5f, 0.f };
            tSpawnDesc.fLaunchSpeed = 0.75f;            
            tSpawnDesc.fLifeTime = 2.5f;
            break;
        }
#pragma endregion
    }

    return tSpawnDesc;
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

CDropStar_Manager::STAR_SPAWN_PRESET CDropStar_Manager::Name_To_Preset(const _wstring& strName) const
{
    static const unordered_map<_wstring, STAR_SPAWN_PRESET> s_Table
    {
        { L"GorillaArmSweepRight",      STAR_SPAWN_PRESET::GORILLA_ARM_SWEEP_RIGHT },
        { L"GorillaArmSweepLeft",       STAR_SPAWN_PRESET::GORILLA_ARM_SWEEP_LEFT  },
        { L"AfterGorillaArmSpin",       STAR_SPAWN_PRESET::AFTER_GORILLA_ARM_SPIN  },
        { L"RockImpact",                STAR_SPAWN_PRESET::ROCK_IMPACT             },
    };

    auto it = s_Table.find(strName);
    return (it != s_Table.end()) ? it->second : STAR_SPAWN_PRESET::PRESET_END;
}

void CDropStar_Manager::Free()
{
	m_Dormant.clear();
	Safe_Release(m_pGameInstance_Proxy);
	__super::Free();
}
