#ifndef Engine_Function_h__
#define Engine_Function_h__

namespace Engine
{
	// 템플릿은 기능의 정해져있으나 자료형은 정해져있지 않은 것
	// 기능을 인스턴스화 하기 위하여 만들어두는 틀

	template<typename T>
	void	Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void	Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete [] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned int Safe_Release(T& Instance)
	{
		unsigned int		iRefCnt = { 0 };

		if (nullptr != Instance)
		{
			iRefCnt = Instance->Release();

			if (0 == iRefCnt)
				Instance = nullptr;
		}

		return iRefCnt;
	}

	template<typename T>
	unsigned int Safe_AddRef(T& Instance)
	{
		unsigned int		iRefCnt = { 0 };

		if (nullptr != Instance)	
			iRefCnt = Instance->AddRef();		

		return iRefCnt;
	}

	inline string WstrToStr(const wstring& wstr)
	{
		if (wstr.empty()) return {};

		int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		string str(size - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
		return str;
	}

	inline wstring StrToWstr(const string& str)
	{
		if (str.empty()) return {};

		int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		wstring wstr(size - 1, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
		return wstr;
	}
}

#endif // Engine_Function_h__
