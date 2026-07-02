#pragma once
#include "Base.h"
#include <shared_mutex>

NS_BEGIN(Engine)
class CShader;

class ENGINE_DLL CTexture_Hub final : public CBase
{
private:
	explicit CTexture_Hub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CTexture_Hub() = default;

public:
	static _string Normalize_TextureName(const _string& strRaw);
	static _wstring Normalize_TextureName(const _wstring& strRaw);

	HRESULT LoadOrGet(const _tchar* pTexturePath, TEXTURE_HANDLE* pOut);
	HRESULT	Get(const _tchar* pTextureKey, TEXTURE_HANDLE* pOut) const;
	HRESULT Register_TextureName(TEXTURE_HANDLE Handle, const _tchar* pTextureName);
	HRESULT Bind_ShaderResource(CShader* pShader, const _char* pConstantName, TEXTURE_HANDLE Handle) const;
	HRESULT Bind_DefaultShaderResource(CShader* pShader, const _char* pConstantName, DEFAULT_TEXTURE eKind) const;
	_bool	Is_Valid(TEXTURE_HANDLE Handle) const;
	_bool	Is_CompatibleDevice(ID3D11Device* pDevice) const;
	TEXTURE_HUB_STATS Get_Stats() const;

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	vector<ID3D11ShaderResourceView*> m_SRVs;
	unordered_map<_wstring, TEXTURE_HANDLE> m_HandleByNormalizedPath;
	unordered_map<_wstring, TEXTURE_HANDLE> m_HandleByTextureName;

	mutable shared_mutex m_Mutex;
	mutable _uint m_iCacheHitCount = {};
	_uint m_iCacheMissCount = {};
	_uint m_iLoadFailureCount = {};

private:
	ID3D11ShaderResourceView* Get_SRV(TEXTURE_HANDLE Handle) const;

public:
	static CTexture_Hub* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	virtual void Free() override;
};

NS_END
