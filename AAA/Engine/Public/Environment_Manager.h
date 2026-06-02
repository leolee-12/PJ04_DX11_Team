#pragma once

// IBL Environment Manager
// - 부팅 때 모든 환경 큐브맵 세트(디퓨즈 irradiance + 스페큘러 prefiltered)를 태그로 등록
// - 맵마다 Set_Current(tag) 로 활성 세트만 스왑
// - 렌더러는 Get_Current() 로 SRV만 받아 바인딩. 경로는 모름.

#include "Base.h"

NS_BEGIN(Engine)

class CEnvironment_Manager final : public CBase
{
private:
    CEnvironment_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~CEnvironment_Manager() = default;

public:
    // 부팅: 태그로 큐브맵 세트 로드+등록. 같은 태그 재등록은 무시.
    HRESULT Register(const _wstring& strTag, const _tchar* pDiffuseDDS, const _tchar* pSpecularDDS, _float fIntensity
        = 1.f);

    // 맵 진입: O(1) 스왑. 없는 태그면 디폴트 유지 + E_FAIL.
    HRESULT Set_Current(const _wstring& strTag);

    // 렌더러가 사용. 항상 유효(폴백 보장).
    const ENVIRONMENT_DESC& Get_Current() const { return m_Current; }

    _bool Has(const _wstring& strTag) const { return m_Registry.find(strTag) != m_Registry.end(); }

private:
    HRESULT Load_Cube(const _tchar* pPath, ID3D11ShaderResourceView** ppSRV, _uint* pMipCount);

private:
    ID3D11Device* m_pDevice = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };

    unordered_map<_wstring, ENVIRONMENT_DESC>   m_Registry;  // 태그 -> 로드된 환경(SRV 소유)
    ENVIRONMENT_DESC                            m_Default;   // 폴백(첫 등록)
    ENVIRONMENT_DESC                            m_Current;   // 활성(registry 포인터 차용)

public:
    static CEnvironment_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END