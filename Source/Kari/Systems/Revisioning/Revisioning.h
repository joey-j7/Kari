#pragma once

#include "CoreMinimal.h"

#include "Characters/NPCs/DefaultNPC.h"
#include "Characters/NPCs/DefaultMerchant.h"
#include "Gameplay/DialogueInteractable.h"

#include "Revisioning.generated.h"

USTRUCT(BlueprintType)
struct FCharacterRevision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	int32 StoryID = -INT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	bool Reposition = false;

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	FVector Location;

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	FRotator Rotation;

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	bool Hidden = false;

	void SaveLoad(FArchive &Ar)
	{
		Ar << StoryID; // int32
		Ar << Reposition; // bool
		Ar << Location; // vector
		Ar << Rotation; // rotator
		Ar << Hidden; // hidden
	}
};

USTRUCT(BlueprintType)
struct FCharacterRevisionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	int32 PreviousID = -INT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	FCharacterRevision Revision;

	void SaveLoad(FArchive &Ar)
	{
		Ar << PreviousID; //int32
		Revision.SaveLoad(Ar);
	}
};

USTRUCT(BlueprintType)
struct FShopRevisionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	int32 PreviousID = -INT_MAX;

	UPROPERTY(BlueprintReadWrite, Category = "Revision Data")
	int32 ShopRevisionID = -INT_MAX;

	void SaveLoad(FArchive &Ar)
	{
		Ar << PreviousID; //int32
		Ar << ShopRevisionID; //int32
	}
};

UCLASS()
class KARI_API URevisioning : public UObject
{
	friend class UDefaultGameInstance;
	friend class ADefaultNPC;
	friend class ADefaultMerchant;
	friend class ADialogueInteractable;

	GENERATED_BODY()

public:
	URevisioning();
	~URevisioning();

	UFUNCTION(BlueprintCallable)
	FCharacterRevisionData GetCharacterRevision(int32 CharacterID) const;

	UFUNCTION(BlueprintCallable)
	void SetCharacterRevision(int32 CharacterID, const FCharacterRevision& Revision);
	void UpdateCharacterRevisions();
	
	UFUNCTION(BlueprintCallable)
	FShopRevisionData GetShopRevision(int32 ShopID) const;

	UFUNCTION(BlueprintCallable)
	void SetShopRevision(int32 ShopID, int32 RevisionID);

	/* Saving & Loading */
	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool SaveData();
	void WriteData(FArchive &Ar);

	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool LoadData();
	void ReadData(FArchive &Ar);

protected:
	void AddNPC(ADefaultNPC& NPC);
	void RemoveNPC(ADefaultNPC& NPC);
	void AddMerchant(ADefaultMerchant& Merchant);
	void RemoveMerchant(ADefaultMerchant& Merchant);
	void AddDialogueInteractable(ADialogueInteractable& DialogueInteractable);

	TArray<ADefaultNPC*> NPCs;
	TArray<ADefaultMerchant*> Merchants;
	TArray<ADialogueInteractable*> DialogueInteractables;

	// Sorted by character ID
	TMap<int32, FCharacterRevisionData> CharacterRevisions;

	// Sorted by shop ID, revisioned by revision ID
	TMap<int32, FShopRevisionData> ShopRevisions;
};
