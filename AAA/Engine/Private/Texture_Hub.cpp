#include "Texture_Hub.h"
#include "Shader.h"
#include "Profiler_Defines.h"
#include <cwctype>
#include <filesystem>
#include <mutex>

namespace
{
	using namespace std::filesystem;

	_wstring To_NormalizedTexturePathKey(const _tchar* pTexturePath)
	{
		if (nullptr == pTexturePath || 0 == pTexturePath[0])
			return {};

		path TexturePath(pTexturePath);
		_wstring strKey = TexturePath.lexically_normal().generic_wstring();
		transform(strKey.begin(), strKey.end(), strKey.begin(),
			[](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });
		return strKey;
	}

	_wstring To_NormalizedTextureName(const _tchar* pTextureName)
	{
		if (nullptr == pTextureName || 0 == pTextureName[0])
			return {};

		return CTexture_Hub::Normalize_TextureName(wstring(pTextureName));
	}

	HRESULT Create_TextureSRV(ID3D11Device* pDevice, const _tchar* pTexturePath, ID3D11ShaderResourceView** ppOutSRV)
	{
		if (nullptr == pDevice || nullptr == pTexturePath || nullptr == ppOutSRV)
			return E_FAIL;

		*ppOutSRV = nullptr;

		path TexturePath(pTexturePath);
		_wstring strExtension = TexturePath.extension().wstring();
		transform(strExtension.begin(), strExtension.end(), strExtension.begin(),
			[](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });

		if (L".dds" == strExtension)
			return CreateDDSTextureFromFile(pDevice, pTexturePath, nullptr, ppOutSRV);

		return CreateWICTextureFromFile(pDevice, pTexturePath, nullptr, ppOutSRV);
	}
}

NS_BEGIN(Engine)

CTexture_Hub::CTexture_Hub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

_string CTexture_Hub::Normalize_TextureName(const _string& strRaw)
{
	if (strRaw.empty())
		return {};

	_string strKey = strRaw;

	if (const size_t iSlash = strKey.find_last_of("/\\"); iSlash != _string::npos)
		strKey = strKey.substr(iSlash + 1);

	transform(strKey.begin(), strKey.end(), strKey.begin(),
		[](_char ch) { return static_cast<_char>(tolower(static_cast<unsigned char>(ch))); });

	if (const size_t iDot = strKey.rfind('.'); iDot != _string::npos)
	{
		const _string strExt = strKey.substr(iDot);
		if (".dds" == strExt || ".png" == strExt)
			strKey.erase(iDot);
	}

	return strKey;
}

_wstring CTexture_Hub::Normalize_TextureName(const _wstring& strRaw)
{
	if (strRaw.empty())
		return {};

	_wstring strKey = strRaw;

	if (const size_t iSlash = strKey.find_last_of(L"/\\"); iSlash != _wstring::npos)
		strKey = strKey.substr(iSlash + 1);

	transform(strKey.begin(), strKey.end(), strKey.begin(),
		[](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });

	if (const size_t iDot = strKey.rfind(L'.'); iDot != _wstring::npos)
	{
		const _wstring strExt = strKey.substr(iDot);
		if (L".dds" == strExt || L".png" == strExt)
			strKey.erase(iDot);
	}

	return strKey;
}

HRESULT CTexture_Hub::LoadOrGet(const _tchar* pTexturePath, TEXTURE_HANDLE* pOut)
{
	if (nullptr == pOut)
		return E_FAIL;

	*pOut = INVALID_TEXTURE_HANDLE;

	if (nullptr == pTexturePath || 0 == pTexturePath[0] || nullptr == m_pDevice)
		return E_FAIL;

	const _wstring strNormalizedPath = To_NormalizedTexturePathKey(pTexturePath);
	if (strNormalizedPath.empty())
		return E_FAIL;

	{
		unique_lock<shared_mutex> Lock(m_Mutex);

		const auto iter = m_HandleByNormalizedPath.find(strNormalizedPath);
		if (iter != m_HandleByNormalizedPath.end())
		{
			if constexpr (0 != PROFILE_ENABLE)
				++m_iCacheHitCount;

			*pOut = iter->second;
			return S_OK;
		}

		if constexpr (0 != PROFILE_ENABLE)
			++m_iCacheMissCount;
	}

	// 파일 로드/디코딩 -> 락 바깥에서 수행
	ID3D11ShaderResourceView* pSRV = nullptr;
	const HRESULT hr = Create_TextureSRV(m_pDevice, pTexturePath, &pSRV);

	unique_lock<shared_mutex> Lock(m_Mutex);

	const auto iter = m_HandleByNormalizedPath.find(strNormalizedPath);
	if (iter != m_HandleByNormalizedPath.end())
	{
		*pOut = iter->second;
		Safe_Release(pSRV);
		return S_OK;
	}

	if (FAILED(hr))
	{
		if constexpr (0 != PROFILE_ENABLE)
			++m_iLoadFailureCount;
		
		return hr;
	}

	try
	{
		const TEXTURE_HANDLE Handle = static_cast<TEXTURE_HANDLE>(m_SRVs.size());
		m_SRVs.push_back(pSRV);
		m_HandleByNormalizedPath.emplace(strNormalizedPath, Handle);
		*pOut = Handle;
	}
	catch (...)
	{
		if constexpr (0 != PROFILE_ENABLE)
			++m_iLoadFailureCount;
		
		Safe_Release(pSRV);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CTexture_Hub::Get(const _tchar* pTextureName, TEXTURE_HANDLE* pOut) const
{
	if (nullptr == pOut)
		return E_FAIL;

	*pOut = INVALID_TEXTURE_HANDLE;

	const _wstring strKey = To_NormalizedTextureName(pTextureName);
	if (strKey.empty())
		return E_FAIL;

	using GetLock = std::conditional_t<0 != PROFILE_ENABLE, unique_lock<shared_mutex>, shared_lock<shared_mutex>>;

	GetLock Lock(m_Mutex);

	const auto iter = m_HandleByTextureName.find(strKey);
	if (iter == m_HandleByTextureName.end())
		return E_FAIL;

	if constexpr (0 != PROFILE_ENABLE)
		++m_iCacheHitCount;
	
	*pOut = iter->second;
	return S_OK;
}

HRESULT CTexture_Hub::Register_TextureName(TEXTURE_HANDLE Handle, const _tchar* pTextureName)
{
	const _wstring strKey = To_NormalizedTextureName(pTextureName);
	if (strKey.empty())
		return E_FAIL;

	unique_lock<shared_mutex> Lock(m_Mutex);

	if (Handle >= m_SRVs.size() || nullptr == m_SRVs[Handle])
		return E_FAIL;

	const auto iter = m_HandleByTextureName.find(strKey);
	if (iter != m_HandleByTextureName.end())
		return iter->second == Handle ? S_OK : E_FAIL;

	m_HandleByTextureName.emplace(strKey, Handle);
	return S_OK;
}

HRESULT CTexture_Hub::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, TEXTURE_HANDLE Handle) const
{
	if (nullptr == pShader || nullptr == pConstantName)
		return E_FAIL;

	ID3D11ShaderResourceView* pSRV = Get_SRV(Handle);
	if (nullptr == pSRV)
		return E_FAIL;

	return pShader->Bind_SRV(pConstantName, pSRV);
}

HRESULT CTexture_Hub::Bind_DefaultShaderResource(CShader* pShader, const _char* pConstantName, DEFAULT_TEXTURE eKind) const
{
	if (nullptr == pShader || nullptr == pConstantName)
		return E_FAIL;

	const _tchar* pTextureName = nullptr;
	switch (eKind)
	{
	case DEFAULT_TEXTURE::WHITE:		pTextureName = L"__Default_White";			break;
	case DEFAULT_TEXTURE::BLACK:		pTextureName = L"__Default_Black";			break;
	case DEFAULT_TEXTURE::MAGENTA:		pTextureName = L"__Default_Magenta";		break;
	case DEFAULT_TEXTURE::FLAT_NORMAL:	pTextureName = L"__Default_Flat_Normal";	break;
	case DEFAULT_TEXTURE::MRA:			pTextureName = L"__Default_MRA";			break;
	default:	return E_FAIL;
	}

	TEXTURE_HANDLE Handle = INVALID_TEXTURE_HANDLE;
	if (FAILED(Get(pTextureName, &Handle)))
		return E_FAIL;

	ID3D11ShaderResourceView* pSRV = Get_SRV(Handle);
	if (nullptr == pSRV)
		return E_FAIL;

	return pShader->Bind_SRV(pConstantName, pSRV);
}

_bool CTexture_Hub::Is_Valid(TEXTURE_HANDLE Handle) const
{
	shared_lock<shared_mutex> Lock(m_Mutex);
	return Handle < m_SRVs.size() && nullptr != m_SRVs[Handle];
}

_bool CTexture_Hub::Is_CompatibleDevice(ID3D11Device* pDevice) const
{
	return nullptr != pDevice && pDevice == m_pDevice;
}

TEXTURE_HUB_STATS CTexture_Hub::Get_Stats() const
{
	shared_lock<shared_mutex> Lock(m_Mutex);

	TEXTURE_HUB_STATS Stats{};
	Stats.iCachedSRVCount = static_cast<_uint>(m_SRVs.size());
	Stats.iCacheReuseCount = m_iCacheHitCount;
	Stats.iFirstLoadRequestCount = m_iCacheMissCount;
	Stats.iLoadFailCount = m_iLoadFailureCount;
	return Stats;
}

ID3D11ShaderResourceView* CTexture_Hub::Get_SRV(TEXTURE_HANDLE Handle) const
{
	shared_lock<shared_mutex> Lock(m_Mutex);
	if (Handle >= m_SRVs.size())
		return nullptr;

	return m_SRVs[Handle];
}

CTexture_Hub* CTexture_Hub::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTexture_Hub* pInstance = new CTexture_Hub(pDevice, pContext);

	if (nullptr == pInstance)
	{
		MSG_BOX("Failed to Created : CTexture_Hub");
	}

	return pInstance;
}


void CTexture_Hub::Free()
{
	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);
	m_SRVs.clear();

	m_HandleByNormalizedPath.clear();
	m_HandleByTextureName.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	__super::Free();
}

NS_END
