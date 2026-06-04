#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

#include "Engine_Defines.h"

NS_BEGIN(Client)

namespace YshModelValidator
{
	constexpr uint32_t kYshMagic = 0x2E595348;
	constexpr uint32_t kMaxMeshes = 8192;
	constexpr uint32_t kMaxMaterials = 8192;
	constexpr uint32_t kMaxTexturesPerSlot = 128;
	constexpr uint32_t kMaxStringBytes = 8192;

	inline void Set_Reason(std::wstring* pOutReason, const wchar_t* pReason)
	{
		if (nullptr != pOutReason)
			*pOutReason = pReason;
	}

	inline bool Has_Bytes(uint64_t iOffset, uint64_t iBytes, uint64_t iFileSize)
	{
		return iBytes <= iFileSize && iOffset <= iFileSize - iBytes;
	}

	inline bool Read_U32(std::ifstream& File, uint64_t iFileSize, uint64_t* pOffset, uint32_t* pOutValue)
	{
		if (nullptr == pOffset || nullptr == pOutValue || !Has_Bytes(*pOffset, sizeof(uint32_t), iFileSize))
			return false;

		File.read(reinterpret_cast<char*>(pOutValue), sizeof(uint32_t));
		if (!File.good())
			return false;

		*pOffset += sizeof(uint32_t);
		return true;
	}

	inline bool Skip_Bytes(std::ifstream& File, uint64_t iFileSize, uint64_t* pOffset, uint64_t iBytes)
	{
		if (nullptr == pOffset || !Has_Bytes(*pOffset, iBytes, iFileSize))
			return false;

		if (iBytes > static_cast<uint64_t>((std::numeric_limits<std::streamoff>::max)()))
			return false;

		File.seekg(static_cast<std::streamoff>(iBytes), std::ios::cur);
		if (!File.good())
			return false;

		*pOffset += iBytes;
		return true;
	}

	inline bool Skip_String(std::ifstream& File, uint64_t iFileSize, uint64_t* pOffset, std::wstring* pOutReason)
	{
		uint32_t iLength = 0;
		if (!Read_U32(File, iFileSize, pOffset, &iLength))
		{
			Set_Reason(pOutReason, L"truncated string length");
			return false;
		}

		if (iLength > kMaxStringBytes)
		{
			Set_Reason(pOutReason, L"string length too large");
			return false;
		}

		if (!Skip_Bytes(File, iFileSize, pOffset, iLength))
		{
			Set_Reason(pOutReason, L"truncated string payload");
			return false;
		}

		return true;
	}

	inline bool Skip_Array(
		std::ifstream& File,
		uint64_t iFileSize,
		uint64_t* pOffset,
		uint32_t iCount,
		uint64_t iElementSize,
		std::wstring* pOutReason,
		const wchar_t* pReason)
	{
		if (0 != iElementSize && iCount > (std::numeric_limits<uint64_t>::max)() / iElementSize)
		{
			Set_Reason(pOutReason, pReason);
			return false;
		}

		const uint64_t iBytes = static_cast<uint64_t>(iCount) * iElementSize;
		if (!Skip_Bytes(File, iFileSize, pOffset, iBytes))
		{
			Set_Reason(pOutReason, pReason);
			return false;
		}

		return true;
	}

	inline bool Validate_NonAnimYsh(const std::filesystem::path& FilePath, std::wstring* pOutReason = nullptr)
	{
		namespace fs = std::filesystem;

		std::error_code ErrorCode;
		const uint64_t iFileSize = static_cast<uint64_t>(fs::file_size(FilePath, ErrorCode));
		if (ErrorCode || iFileSize < sizeof(uint32_t) * 4)
		{
			Set_Reason(pOutReason, L"file missing or too small");
			return false;
		}

		std::ifstream File{ FilePath, std::ios::binary };
		if (!File.is_open())
		{
			Set_Reason(pOutReason, L"file open failed");
			return false;
		}

		uint64_t iOffset = 0;
		uint32_t iMagic = 0;
		uint32_t iVersion = 0;
		uint32_t iType = 0;
		if (!Read_U32(File, iFileSize, &iOffset, &iMagic)
			|| !Read_U32(File, iFileSize, &iOffset, &iVersion)
			|| !Read_U32(File, iFileSize, &iOffset, &iType))
		{
			Set_Reason(pOutReason, L"truncated header");
			return false;
		}

		if (kYshMagic != iMagic)
		{
			Set_Reason(pOutReason, L"invalid magic");
			return false;
		}

		if (0 != iType)
		{
			Set_Reason(pOutReason, L"animated model");
			return false;
		}

		uint32_t iNumMeshes = 0;
		if (!Read_U32(File, iFileSize, &iOffset, &iNumMeshes))
		{
			Set_Reason(pOutReason, L"truncated mesh count");
			return false;
		}

		if (iNumMeshes > kMaxMeshes)
		{
			Set_Reason(pOutReason, L"mesh count too large");
			return false;
		}

		for (uint32_t iMesh = 0; iMesh < iNumMeshes; ++iMesh)
		{
			if (!Skip_String(File, iFileSize, &iOffset, pOutReason))
				return false;

			uint32_t iMaterialIndex = 0;
			uint32_t iNumVerts = 0;
			if (!Read_U32(File, iFileSize, &iOffset, &iMaterialIndex)
				|| !Read_U32(File, iFileSize, &iOffset, &iNumVerts))
			{
				Set_Reason(pOutReason, L"truncated mesh header");
				return false;
			}

			if (!Skip_Array(File, iFileSize, &iOffset, iNumVerts, sizeof(VTXMESH_DATA), pOutReason, L"vertex data exceeds file size"))
				return false;

			uint32_t iNumIndices = 0;
			if (!Read_U32(File, iFileSize, &iOffset, &iNumIndices))
			{
				Set_Reason(pOutReason, L"truncated index count");
				return false;
			}

			if (!Skip_Array(File, iFileSize, &iOffset, iNumIndices, sizeof(uint32_t), pOutReason, L"index data exceeds file size"))
				return false;
		}

		uint32_t iNumMaterials = 0;
		if (!Read_U32(File, iFileSize, &iOffset, &iNumMaterials))
		{
			Set_Reason(pOutReason, L"truncated material count");
			return false;
		}

		if (iNumMaterials > kMaxMaterials)
		{
			Set_Reason(pOutReason, L"material count too large");
			return false;
		}

		for (uint32_t iMaterial = 0; iMaterial < iNumMaterials; ++iMaterial)
		{
			for (uint32_t iSlot = 0; iSlot < MTEX_TYPE_MAX; ++iSlot)
			{
				uint32_t iNumTextures = 0;
				if (!Read_U32(File, iFileSize, &iOffset, &iNumTextures))
				{
					Set_Reason(pOutReason, L"truncated texture count");
					return false;
				}

				if (iNumTextures > kMaxTexturesPerSlot)
				{
					Set_Reason(pOutReason, L"texture count too large");
					return false;
				}

				for (uint32_t iTexture = 0; iTexture < iNumTextures; ++iTexture)
				{
					if (!Skip_String(File, iFileSize, &iOffset, pOutReason))
						return false;
				}
			}
		}

		return true;
	}
}

NS_END
