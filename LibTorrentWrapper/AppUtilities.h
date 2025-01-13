#pragma once
#include <msclr/marshal_cppstd.h>
using namespace System;

class AppUtilities
{
public:
	// Convert from managed String^ to native std::string
	static std::string ToStdString(String^ managedString)
	{
		return msclr::interop::marshal_as<std::string>(managedString);
	}

	// Convert from native std::string to managed String^
	static String^ ToManagedString(const std::string& nativeString)
	{
		return gcnew String(nativeString.c_str());
	}

	static bool Contains(array<int>^ dataset, int search)
	{
		for (int s = 0; s < dataset->Length; s++)
		{
			if (dataset[s] == search)
			{
				return true;
			}
		}
		return false;
	}
};
