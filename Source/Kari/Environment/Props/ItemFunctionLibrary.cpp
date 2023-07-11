#include "ItemFunctionLibrary.h"
#include "TableDatabase.h"
#include "General/BPFL_BaseDefaultFunctionLibrary.h"
#include "Engine/GameInstance.h"
#include "General/DefaultGameInstance.h"

#include "AssetData.h"

FItem UItemFunctionLibrary::GetItem(int32 ID)
{
	return FItem::ConvertRow(TableDatabase::Get().GetRow("items", ID), ID);
}

UTexture2D* UItemFunctionLibrary::GetItemIconById(int32 ItemId)
{
	if (ItemId == -1)
	{
		return nullptr;
	}
		
	const FItem Item = UItemFunctionLibrary::GetItem(ItemId);
	return GetItemIcon(Item);
}

UTexture2D* UItemFunctionLibrary::GetItemIcon(const FItem& Item)
{
	// Shorten the base default function library
	using ULib = UBPFL_BaseDefaultFunctionLibrary;

	if (Item.ID == -1)
	{
		return nullptr;
	}

	TArray< FStringFormatArg > Args;
	Args.Add(FStringFormatArg("T_" + Item.Name.Replace(TEXT(" "), TEXT(""))));

	UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *FString::Format(TEXT("/Game/UI/Textures/ItemIcons/{0}.{0}"), Args), nullptr, LOAD_None, nullptr);

	return Texture;
}