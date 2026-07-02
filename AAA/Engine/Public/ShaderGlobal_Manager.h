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

    // 시간 보간 예약: strName 값을 vTarget 까지 fDuration 초에 걸쳐 이동
    // fDuration <= 0 이면 즉시 세팅. 같은 이름 재호출 시 현재값에서 다시 시작(덮어씀)
    void Tween(const string& strName, const _float4& vTarget, _float fDuration);

    // 진행 중인 보간 즉시 취소(현재값 유지)
    void Stop_Tween(const string& strName);
    _bool Is_Tweening(const string& strName) const;

    // 매 프레임 호출: 활성 보간을 진행시켜 vValue 갱신
    void Tick(_float fTimeDelta);

    HRESULT Bind(CShader* pShader, const string& strName);                       // 단일
    HRESULT Bind(CShader* pShader, initializer_list<const _char*> Names);        // 다중

    vector<GLOBAL_DESC>& Get_All() { return m_Globals; }    // 에디터 이터레이션

private:
    _uint Type_Size(GVAL eType) const;

private:
    struct TWEEN
    {
        _float4 vStart = {};
        _float4 vTarget = {};
        _float  fElapsed = 0.f;
        _float  fDuration = 0.f;
    };

    unordered_map<string, _uint> m_Index;     // name → m_Globals 인덱스
    vector<GLOBAL_DESC>          m_Globals;
    unordered_map<_uint, TWEEN>  m_Tweens;

public:
    static CShaderGlobal_Manager* Create();
    virtual void Free() override;
};

NS_END