#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Item.h"
#include "DatabaseIDPin.h"
#include "ItemFunctionLibrary.generated.h"

class UGameInstance;

UCLASS()
class KARI_API UItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item", meta = (DisplayName = "Get Item Properties"))
	static FItem GetItem(int32 ID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	static UTexture2D* GetItemIconById(int32 ItemId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
	static UTexture2D* GetItemIcon(const FItem& Item);
};