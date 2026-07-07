#include "Shader.h"
#include "ShaderCache_Utils.h"
#pragma comment(lib, "d3dcompiler.lib")

CShader::CShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent { pDevice, pContext }
{
}

CShader::CShader(const CShader& Prototype)
    : CComponent(Prototype)
    , m_pEffect{ Prototype.m_pEffect }
    , m_iNumPasses{ Prototype.m_iNumPasses }
    , m_InputLayouts{ Prototype.m_InputLayouts }
{
    for (auto& pInputLayout : m_InputLayouts)
        Safe_AddRef(pInputLayout);
    Safe_AddRef(m_pEffect);
}

HRESULT CShader::Initialize_Prototype(const _tchar* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
    _uint   iHLSLFlag = {};

#ifdef _DEBUG
    iHLSLFlag |= D3DCOMPILE_DEBUG;
    iHLSLFlag |= D3DCOMPILE_SKIP_OPTIMIZATION;
    const _tchar* szCacheExt = TEXT(".fxbin_d");    // 컴파일 플래그가 달라서 Debug/Release 캐시 분리
#else
    iHLSLFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
    const _tchar* szCacheExt = TEXT(".fxbin");
#endif

    namespace fs = std::filesystem;
    fs::path srcPath = pShaderFilePath;
    fs::path cachePath = srcPath;
    cachePath += szCacheExt;

    // 1. 캐시가 신선하면 컴파일 없이 블롭에서 바로 이펙트 생성
    if (ShaderCache::Is_Fresh(srcPath, cachePath))
    {
        vector<char> Blob = ShaderCache::Read_Blob(cachePath);
        if (!Blob.empty())
            D3DX11CreateEffectFromMemory(Blob.data(), Blob.size(), 0, m_pDevice, &m_pEffect);
        // 실패하면 m_pEffect가 nullptr로 남아 아래에서 재컴파일한다
    }

    // 2. 캐시 미스 또는 캐시 손상 -> 컴파일 후 캐시 갱신
    if (nullptr == m_pEffect)
    {
        ID3DBlob* pCode = { nullptr };
        ID3DBlob* pError = { nullptr };

        // fx_5_0 프로파일은 엔트리포인트를 nullptr로 넘겨야 한다
        HRESULT hr = D3DCompileFromFile(pShaderFilePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            nullptr, "fx_5_0", iHLSLFlag, 0, &pCode, &pError);

        if (FAILED(hr))
        {
            ShaderCache::Report_Error(pError, "Shader Compile Error");
            Safe_Release(pError);
            return E_FAIL;
        }
        Safe_Release(pError);   // 성공 시엔 경고만 들어있을 수 있다

        hr = D3DX11CreateEffectFromMemory(pCode->GetBufferPointer(), pCode->GetBufferSize(), 0, m_pDevice, &m_pEffect);
        if (SUCCEEDED(hr))
            ShaderCache::Write_Blob(cachePath, pCode->GetBufferPointer(), pCode->GetBufferSize());

        Safe_Release(pCode);

        if (FAILED(hr))
            return E_FAIL;
    }
    
	ID3DX11EffectTechnique*     pTechnique = m_pEffect->GetTechniqueByIndex(0);
    if (!pTechnique->IsValid())
        return E_FAIL;

    D3DX11_TECHNIQUE_DESC     TechniqueDesc = {};

    pTechnique->GetDesc(&TechniqueDesc);

    m_iNumPasses = TechniqueDesc.Passes;

    for (_uint i = 0; i < m_iNumPasses; i++)
    {
		ID3DX11EffectPass& pPass = *pTechnique->GetPassByIndex(i);

		D3DX11_PASS_DESC PassDesc = {};

		if (FAILED(pPass.GetDesc(&PassDesc)))
			return E_FAIL;

        ID3D11InputLayout* pInputLayout = nullptr;

        if (FAILED(m_pDevice->CreateInputLayout(pElements, iNumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout)))
            return E_FAIL;

		m_InputLayouts.push_back(pInputLayout);
    }

    return S_OK;
}

HRESULT CShader::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CShader::Begin(_uint iPassIndex)
{
    if (iPassIndex >= m_iNumPasses)
        return E_FAIL;

    m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex]);

    m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex)->Apply(0, m_pContext);

    return S_OK;
}

HRESULT CShader::Bind_Matrix(const _char* pConstantName, const _float4x4* pMatrix)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
    if (nullptr == pMatrixVariable)
        return E_FAIL;

    return pMatrixVariable->SetMatrix(reinterpret_cast<const _float*>(pMatrix));
}

HRESULT CShader::Bind_Matrices(const _char* pConstantName, const _float4x4* pMatrices, _uint iNumMatrices)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
    if (nullptr == pMatrixVariable)
        return E_FAIL;

    return pMatrixVariable->SetMatrixArray(reinterpret_cast<const _float*>(pMatrices), 0, iNumMatrices);
}

HRESULT CShader::Bind_SRV(const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
    if (nullptr == pSRVariable)
        return E_FAIL;

    return pSRVariable->SetResource(pSRV);
}

HRESULT CShader::Bind_RawValue(const _char* pConstantName, const void* pValue, _uint iLength)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    return pVariable->SetRawValue(pValue, 0, iLength);
}

CShader* CShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
    CShader* pInstance = new CShader(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pShaderFilePath, pElements, iNumElements)))
    {
        MSG_BOX("Failed to Created : CShader");

        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CShader::Clone(void* pArg)
{
    CShader* pInstance = new CShader(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CShader");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CShader::Free()
{
    __super::Free();

    for (auto& pInputLayout : m_InputLayouts)
        Safe_Release(pInputLayout);

    m_InputLayouts.clear();

    Safe_Release(m_pEffect);
}
