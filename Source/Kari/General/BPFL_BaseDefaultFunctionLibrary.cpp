// Fill out your copyright notice in the Description page of Project Settings.


#include "BPFL_BaseDefaultFunctionLibrary.h"
#include <Engine.h>

#include "AssetRegistryModule.h"
#include "Systems/Audio/Playlist.h"
#include "Systems/Audio/Audio.h"

TArray<UObject*> UBPFL_BaseDefaultFunctionLibrary::DynamicLoadContentFromPath(FString PathFromContentFolder /*= "" */, UClass* AssetClass, bool LoadFromSubfolders)
{
	TArray<UObject*> Array;

	const FString RootFolderFullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + "Content/" + PathFromContentFolder + "/";
	const FString FolderPath = "RootFolderPath = " + RootFolderFullPath;
	UE_LOG(LogClass, Display, TEXT("%s"), *FolderPath);

	//FPaths::NormalizeDirectoryName(RootFolderFullPath);
	//Print("Normalized RootFolderPath = " + RootFolderFullPath);

	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> Files;

	FString Ext;

	if (LoadFromSubfolders)
	{
		if (!Ext.Contains(TEXT("*")))
		{
			if (Ext == "")
			{
				Ext = "*.*";
			}
			else
			{
				Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
			}
		}

		FileManager.FindFilesRecursive(Files, *RootFolderFullPath, *Ext, true, false);
	}
	else
	{
		if (!Ext.Contains(TEXT("*")))
		{
			if (Ext == "")
			{
				Ext = "*.*";
			}
			else
			{
				Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
			}
		}

		FileManager.FindFiles(Files, *(RootFolderFullPath + Ext), true, false);
	}

	for (int32 i = 0; i < Files.Num(); i++)
	{
		FString Path;
		if (LoadFromSubfolders)
		{
			const int32 LastForwardSlash = Files[i].Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			FString File = Files[i].RightChop(LastForwardSlash + 1);
			FString Folder = Files[i].RightChop(Files[i].Find(PathFromContentFolder, ESearchCase::CaseSensitive, ESearchDir::FromEnd) + PathFromContentFolder.Len());
			Folder = Folder.LeftChop(File.Len() + 1);
			File = File.Left(File.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd));
			Path = AssetClass->GetFName().ToString() + "'/Game/" + PathFromContentFolder + Folder + "/" + File + "." + File + "'";
		}
		else
		{
			Path = AssetClass->GetFName().ToString() + "'/Game/" + PathFromContentFolder + "/" + Files[i].LeftChop(7) + "." + Files[i].LeftChop(7) + "'";
		}
		UObject* LoadedObj = StaticLoadObject(AssetClass, nullptr, *Path);

		Array.Add(LoadedObj);
	}

	for (int32 i = 0; i < Array.Num(); i++)
	{
		if (Array.Num() > 0 && Array[i] != nullptr)
		{
			UE_LOG(LogTemp, Display, TEXT("%s"), *Array[i]->GetFName().ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Array is empty"));
		}
	}

	return Array;
}

// Function to amend the sentence 
FString UBPFL_BaseDefaultFunctionLibrary::AmendSentence(const FString& Str, const char* Character, int32 SkipIndex)
{
	// Traverse the string
	FString newStr("");

	if(Str == "")
		return newStr;

	int32 counter = 0;
	for(int32 i = 0; i < Str.Len(); i++) 
	{ 
		// an uppercase character 
		if (Str[i] >= 'A' && Str[i] <= 'Z')
		{
			// if its an uppercase character
			if(counter > SkipIndex)
				newStr += Character;

			counter++;
		} 
  
		// if lowercase character 
		// then just print 
		newStr += Str[i]; 
	}

	return newStr;
} 

void UBPFL_BaseDefaultFunctionLibrary::FadeAudio(UPlaylist* Playlist, float FadeSpeed)
{
	if (Playlist)
	{
		for (const PlaylistItem& Item : Playlist->GetPlaylistItems())
		{
			const float Duration = 1.f / FadeSpeed;

			for (UAudio* Layer : Item)
			{
				Layer->FadeVolume(Layer->GetVolume(), 0.f, Duration, true);
			}
		}
	}
}