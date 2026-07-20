#pragma once
#include "Engine_Defines.h"
#include <filesystem>
#include <fstream>

NS_BEGIN(Engine)

namespace ShaderCache
{
    namespace fs = std::filesystem;

    // 원본 hlsl 또는 같은 폴더의 hlsli(공용 인클루드)가 캐시보다 새로우면 캐시 무효
    inline _bool Is_Fresh(const fs::path& srcPath, const fs::path& cachePath)
    {
        std::error_code ec;
        if (!fs::exists(cachePath, ec))
            return false;

        const auto cacheTime = fs::last_write_time(cachePath, ec);
        if (ec)
            return false;

        const auto srcTime = fs::last_write_time(srcPath, ec);
        if (ec || srcTime > cacheTime)
            return false;

        for (const auto& entry : fs::directory_iterator(srcPath.parent_path(), ec))
        {
            const fs::path Extension = entry.path().extension();
            if (Extension == L".hlsli" || Extension == L".hlsl")
            {
                const auto incTime = fs::last_write_time(entry.path(), ec);
                if (ec || incTime > cacheTime)
                    return false;
            }
        }
        return true;
    }

    inline vector<char> Read_Blob(const fs::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
            return {};

        const std::streamsize iSize = file.tellg();
        file.seekg(0, std::ios::beg);

        vector<char> Buffer(static_cast<size_t>(iSize));
        if (!file.read(Buffer.data(), iSize))
            return {};

        return Buffer;
    }

    inline void Write_Blob(const fs::path& path, const void* pData, size_t iSize)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (file)
            file.write(static_cast<const char*>(pData), iSize);
    }

    inline void Report_Error(ID3DBlob* pError, const _char* szTitle)
    {
        if (nullptr == pError)
            return;

        const _char* szMsg = static_cast<const _char*>(pError->GetBufferPointer());
        OutputDebugStringA(szMsg);
        MessageBoxA(nullptr, szMsg, szTitle, MB_OK | MB_ICONERROR);
    }
}

NS_END