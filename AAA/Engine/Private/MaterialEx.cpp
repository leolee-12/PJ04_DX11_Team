#include "MaterialEx.h"
#include "GameInstance.h"
#include "Shader.h"

#include <filesystem>

namespace
{
	using namespace std::filesystem;

	HRESULT Resolve_TexturePath(const _char* pModelFilePath, const string& strTextureName, path* pOutPath)
	{
		if (nullptr == pModelFilePath || nullptr == pOutPath)
			return E_FAIL;

		string strBaseName = strTextureName;
		if (const size_t iDot = strBaseName.rfind('.'); iDot != string::npos)
			strBaseName = strBaseName.substr(0, iDot);

		const path ModelPath(StrToWstr(string(pModelFilePath)));
		const path DirectoryPath = ModelPath.parent_path();

		auto Try_ResolveFromDirectory = [&](const path& RootDirectory) -> _bool
			{
				if (RootDirectory.empty())
					return false;

				error_code ec;
				const path DDSPath = RootDirectory / (StrToWstr(strBaseName) + L".dds");
				if (exists(DDSPath, ec) && !ec)
				{
					*pOutPath = DDSPath;
					return true;
				}

				ec.clear();
				const path PNGPath = RootDirectory / (StrToWstr(strBaseName) + L".png");
				if (exists(PNGPath, ec) && !ec)
				{
					*pOutPath = PNGPath;
					return true;
				}

				return false;
			};

		if (Try_ResolveFromDirectory(DirectoryPath))
			return S_OK;

		const path StageRoot = DirectoryPath.parent_path();
		const path MapRoot = StageRoot.parent_path();
		if (Try_ResolveFromDirectory(MapRoot / L"TexPool"))
			return S_OK;

		return E_FAIL;
	}

	HRESULT Load_TextureHandleWithFallback(
		CGameInstance_Proxy* pProxy,
		const path& TexturePath,
		TEXTURE_HANDLE* pOutHandle)
	{
		if (nullptr == pProxy || nullptr == pOutHandle)
			return E_FAIL;

		*pOutHandle = INVALID_TEXTURE_HANDLE;

		HRESULT hr = pProxy->LoadOrGet_TextureFromHub(TexturePath.c_str(), pOutHandle);
		if (SUCCEEDED(hr))
			return S_OK;

		wstring strExtension = TexturePath.extension().wstring();
		transform(strExtension.begin(), strExtension.end(), strExtension.begin(),
			[](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });

		if (L".dds" != strExtension)
			return hr;

		path PngPath = TexturePath;
		PngPath.replace_extension(L".png");

		error_code ec;
		if (!exists(PngPath, ec) || ec)
			return hr;

		*pOutHandle = INVALID_TEXTURE_HANDLE;
		return pProxy->LoadOrGet_TextureFromHub(PngPath.c_str(), pOutHandle);
	}
}

NS_BEGIN(Engine)

HRESULT CMaterialEx::Initialize(const MATERIAL_DATA& data, const _char* pModelFilePath)
{
	if (nullptr == pModelFilePath)
		return E_FAIL;

	m_pProxy = CGameInstance::GetProxy();
	if (nullptr == m_pProxy)
		return E_FAIL;

	for (_uint i = 0; i < MTEX_TYPE_MAX; ++i)
	{
		for (const string& strTextureName : data.TextureNames[i])
		{
			path TexturePath;
			if (FAILED(Resolve_TexturePath(pModelFilePath, strTextureName, &TexturePath)))
				return E_FAIL;

			TEXTURE_HANDLE Handle = INVALID_TEXTURE_HANDLE;
			if (FAILED(Load_TextureHandleWithFallback(m_pProxy, TexturePath, &Handle)))
				return E_FAIL;

			m_MaterialHandles[i].push_back(Handle);
		}
	}

	return S_OK;
}

HRESULT CMaterialEx::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, MTEX_TYPE eTexType, _uint iIndex)
{
	if (nullptr == pShader || nullptr == pConstantName || nullptr == m_pProxy)
		return E_FAIL;

	const _uint iType = ETOUI(eTexType);
	if (iType >= MTEX_TYPE_MAX || iIndex >= m_MaterialHandles[iType].size())
		return E_FAIL;

	return m_pProxy->Bind_TextureFromHub(
		pShader,
		pConstantName,
		m_MaterialHandles[iType][iIndex]);
}

CMaterialEx* CMaterialEx::Create(const MATERIAL_DATA& data, const _char* pModelFilePath)
{
	CMaterialEx* pInstance = new CMaterialEx();

	if (FAILED(pInstance->Initialize(data, pModelFilePath)))
	{
		MSG_BOX("Failed to Created : CMaterialEx");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMaterialEx::Free()
{
	for (auto& Handles : m_MaterialHandles)
		Handles.clear();

	Safe_Release(m_pProxy);

	__super::Free();
}

NS_END
