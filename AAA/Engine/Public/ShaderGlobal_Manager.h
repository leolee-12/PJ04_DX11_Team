#pragma once
#include "Engine_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)

class CShader;

class CShaderGlobal_Manager final : public CBase
{
private:
    CShaderGlobal_Manager();
    virtual ~CShaderGlobal_Manager() = default;

public:
    HRESULT Initialize();

public:
    void Register(const GLOBAL_DESC& Desc);                 // 전역 등록/갱신

    void           Set(const string& strName, const _float4& vValue);  // 값 편집(게임플레이)
    const _float4* Get(const string& strName) const;

    HRESULT Bind(CShader* pShader, const string& strName);                       // 단일
    HRESULT Bind(CShader* pShader, initializer_list<const _char*> Names);        // 다중

    vector<GLOBAL_DESC>& Get_All() { return m_Globals; }    // 에디터 이터레이션

private:
    _uint Type_Size(GVAL eType) const;

private:
    unordered_map<string, _uint> m_Index;     // name → m_Globals 인덱스
    vector<GLOBAL_DESC>          m_Globals;

public:
    static CShaderGlobal_Manager* Create();
    virtual void Free() override;
};

NS_END