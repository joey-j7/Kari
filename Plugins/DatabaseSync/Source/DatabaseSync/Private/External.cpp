#ifdef WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <string>
#include <iostream>

#include "TableBinary.h"
#include "TableDatabase.h"

#include "JsonSerializer.h"

// For use outside of the application
extern "C" __declspec(dllexport) void PackExternal(
	HWND hwnd,        // handle to owner window
	HINSTANCE hinst,  // instance handle for the DLL
	LPTSTR lpCmdLine, // string the DLL will parse
	int nCmdShow      // show state
)
{
	std::cout << "test" << std::endl;

	//// Handle result
	//FString string = FString(lpCmdLine);

	////Create a pointer to hold the json serialized data
	//TSharedPtr<FJsonObject> JsonObject;

	////Create a reader pointer to read the json data
	//TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(string);

	////Deserialize the json data given Reader and the actual object to deserialize
	//if (FJsonSerializer::Deserialize(Reader, JsonObject))
	//{
	//	// For each table
	//	for (auto table = JsonObject->Values.begin(); table != JsonObject->Values.end(); ++table)
	//	{
	//		std::string key = TCHAR_TO_UTF8(*table->Key);

	//		if (key == "error_msg")
	//		{
	//			continue;
	//		}

	//		// Get table name and store it as individual data
	//		TableBinary bin = TableDatabase::Get().GetBinary(table->Key);
	//		bin.Import(table->Value.Get()->AsObject());
	//		bin.Export();
	//	}
	//}
}

#endif