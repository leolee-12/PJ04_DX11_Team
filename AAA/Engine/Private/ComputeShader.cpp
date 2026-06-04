#include "ComputeShader.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")   // 이미 링크돼 있으면 중복 무해

CComputeShader::CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }, m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CComputeShader::Initialize(const _tchar* pShaderFilePath, const _char* pEntryPoint)
{
    _uint iFlags = 0;
#ifdef _DEBUG
    iFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    iFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

    ID3DBlob* pCode = nullptr;
    ID3DBlob* pError = nullptr;

    if (FAILED(D3DCompileFromFile(pShaderFilePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        pEntryPoint, "cs_5_0", iFlags, 0, &pCode, &pError)))
    {
        if (pError) { OutputDebugStringA(reinterpret_cast<char*>(pError->GetBufferPointer())); Safe_Release(pError); }
        Safe_Release(pCode);
        return E_FAIL;
    }
    Safe_Release(pError);

    HRESULT hr = m_pDevice->CreateComputeShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr,
        &m_pComputeShader);
    Safe_Release(pCode);
    return hr;
}

void CComputeShader::Bind() { m_pContext->CSSetShader(m_pComputeShader, nullptr, 0); }
void CComputeShader::Dispatch(_uint x, _uint y, _uint z) { m_pContext->Dispatch(x, y, z); }
void CComputeShader::Bind_CBV(_uint s, ID3D11Buffer* b) {
    m_pContext->CSSetConstantBuffers(s, 1,
        &b);
}
void CComputeShader::Bind_SRV(_uint s, ID3D11ShaderResourceView* v) {
    m_pContext->CSSetShaderResources(s, 1,
        &v);
}
void CComputeShader::Bind_UAV(_uint s, ID3D11UnorderedAccessView* u) {
    m_pContext->CSSetUnorderedAccessViews(s,
        1, &u, nullptr);
}

void CComputeShader::Unbind_SRVs(_uint s, _uint c)
{
    ID3D11ShaderResourceView* nulls[8] = {};
    m_pContext->CSSetShaderResources(s, c, nulls);
}
void CComputeShader::Unbind_UAVs(_uint s, _uint c)
{
    ID3D11UnorderedAccessView* nulls[8] = {};
    m_pContext->CSSetUnorderedAccessViews(s, c, nulls, nullptr);
}
void CComputeShader::Unbind_Shader() { m_pContext->CSSetShader(nullptr, nullptr, 0); }

CComputeShader* CComputeShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,
    const _tchar* pShaderFilePath, const _char* pEntryPoint)
{
    CComputeShader* pInstance = new CComputeShader(pDevice, pContext);
    if (FAILED(pInstance->Initialize(pShaderFilePath, pEntryPoint)))
    {
        MSG_BOX("Failed to Created : CComputeShader");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CComputeShader::Free()
{
    __super::Free();
    Safe_Release(m_pComputeShader);
    Safe_Release(m_pContext);
    Safe_Release(m_pDevice);
}