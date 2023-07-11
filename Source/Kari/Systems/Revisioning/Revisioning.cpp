#include "Revisioning.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Archive.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/FileHelper.h"

#include "Systems/Components/StoryComponent.h"
#include "Systems/Components/Crafting/DefaultCraftingComponent.h"

URevisioning::URevisioning()
{
	
}

URevisioning::~URevisioning()
{

}

FCharacterRevisionData URevisioning::GetCharacterRevision(int32 CharacterID) const
{
	auto D = CharacterRevisions.Find(CharacterID);
	return D ? *D : FCharacterRevisionData();
}

void URevisioning::SetCharacterRevision(int32 CharacterID, const FCharacterRevision& Data)
{
	if (CharacterID < 0 || Data.StoryID < 0)
		return;

	FCharacterRevisionData& D = CharacterRevisions.FindOrAdd(CharacterID);

	// Same ID, return
	if (D.Revision.StoryID == Data.StoryID)
		return;

	D.PreviousID = D.Revision.StoryID;

	D.Revision = Data;

	for (auto DialogueInteractable : DialogueInteractables)
	{
		if (CharacterID != DialogueInteractable->GetCharacterComponent()->GetTableID())
			continue;

		if (Data.Reposition)
		{
			DialogueInteractable->SetActorLocation(Data.Location);
			DialogueInteractable->SetActorRotation(Data.Rotation);

			UE_LOG(LogTemp, Log, TEXT("Interactable %s's location and rotation are set according to its revision"), *DialogueInteractable->GetName());
		}

		DialogueInteractable->SetActorHiddenInGame(Data.Hidden);
		DialogueInteractable->SetActorEnableCollision(!Data.Hidden);
		DialogueInteractable->SetActorTickEnabled(!Data.Hidden);

		if (Data.Hidden)
		{
			UE_LOG(LogTemp, Log, TEXT("Interactable %s has been made invisible according to its revision"), *DialogueInteractable->GetName());
		}

		DialogueInteractable->GetStoryComponent()->SetTableID(Data.StoryID);
	}

	for (auto NPC : NPCs)
	{
		if (CharacterID != NPC->GetCharacterComponent()->GetTableID())
			continue;

		if (Data.Reposition)
		{
			NPC->SetActorLocation(Data.Location);
			NPC->SetActorRotation(Data.Rotation);

			UE_LOG(LogTemp, Log, TEXT("NPC %s's location and rotation are set according to its revision"), *NPC->GetName());
		}

		NPC->SetActorHiddenInGame(Data.Hidden);
		NPC->SetActorEnableCollision(!Data.Hidden);
		NPC->SetActorTickEnabled(!Data.Hidden);

		if (Data.Hidden)
		{
			UE_LOG(LogTemp, Log, TEXT("NPC %s has been made invisible according to its revision"), *NPC->GetName());
		}

		NPC->GetStoryComponent()->SetTableID(Data.StoryID);
	}

	for (auto Merchant : Merchants)
	{
		if (CharacterID != Merchant->GetCharacterComponent()->GetTableID())
			continue;

		Merchant->GetStoryComponent()->SetTableID(Data.StoryID);
	}
}

void URevisioning::UpdateCharacterRevisions()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("NPC count: %i"), NPCs.Num()));
	for (auto NPC : NPCs)
	{
		FCharacterRevisionData* RevisionData = CharacterRevisions.Find(NPC->GetCharacterComponent()->GetTableID());
		if (RevisionData) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Set revision data for NPC"));
			const FCharacterRevision& Data = RevisionData->Revision;
			if (Data.Reposition)
			{
				NPC->SetActorLocation(Data.Location);
				NPC->SetActorRotation(Data.Rotation);

				UE_LOG(LogTemp, Log, TEXT("NPC %s's location and rotation are set according to its revision"), *NPC->GetName());
			}

			NPC->SetActorHiddenInGame(Data.Hidden);
			NPC->SetActorEnableCollision(!Data.Hidden);
			NPC->SetActorTickEnabled(!Data.Hidden);

			if (Data.Hidden)
			{
				UE_LOG(LogTemp, Log, TEXT("NPC %s has been made invisible according to its revision"), *NPC->GetName());
			}

			NPC->GetStoryComponent()->SetTableID(Data.StoryID);
		}
	}
}

FShopRevisionData URevisioning::GetShopRevision(int32 ShopID) const
{
	auto D = ShopRevisions.Find(ShopID);
	return D ? *D : FShopRevisionData();
}

void URevisioning::SetShopRevision(int32 ShopID, int32 RevisionID)
{
	if (ShopID < 0 || RevisionID < 0)
		return;

	FShopRevisionData& D = ShopRevisions.FindOrAdd(RevisionID);
	D.PreviousID = D.ShopRevisionID;

	D.ShopRevisionID = RevisionID;

	for (auto Merchant : Merchants)
	{
		if (ShopID != Merchant->GetTradeComponent()->GetTableID())
			continue;

		Merchant->RevisionID = RevisionID;
	}
}

void URevisioning::RemoveNPC(ADefaultNPC& NPC)
{
	NPCs.Remove(&NPC);
}

void URevisioning::AddMerchant(ADefaultMerchant& Merchant)
{
	Merchants.AddUnique(&Merchant);

	FShopRevisionData* RevisionData = ShopRevisions.Find(Merchant.GetCharacterComponent()->GetTableID());

	if (RevisionData)
		Merchant.RevisionID = RevisionData->ShopRevisionID;
}

void URevisioning::RemoveMerchant(ADefaultMerchant& Merchant)
{
	Merchants.Remove(&Merchant);
}

void URevisioning::AddDialogueInteractable(ADialogueInteractable& DialogueInteractable)
{
	DialogueInteractables.Add(&DialogueInteractable);

	FCharacterRevisionData* RevisionData = CharacterRevisions.Find(DialogueInteractable.GetCharacterComponent()->GetTableID());

	if (RevisionData)
	{
		const FCharacterRevision& Data = RevisionData->Revision;
		if (Data.Reposition)
		{
			DialogueInteractable.SetActorLocation(Data.Location);
			DialogueInteractable.SetActorRotation(Data.Rotation);

			UE_LOG(LogTemp, Log, TEXT("Interactable %s's location and rotation are set according to its revision"), *DialogueInteractable.GetName());
		}

		DialogueInteractable.SetActorHiddenInGame(Data.Hidden);
		DialogueInteractable.SetActorEnableCollision(!Data.Hidden);
		DialogueInteractable.SetActorTickEnabled(!Data.Hidden);

		if (Data.Hidden)
		{
			UE_LOG(LogTemp, Log, TEXT("Interactable %s has been made invisible according to its revision"), *DialogueInteractable.GetName());
		}

		DialogueInteractable.GetStoryComponent()->SetTableID(Data.StoryID);
	}
}

bool URevisioning::SaveData()
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteData(ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\RevisioningSavedData.sav"));

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void URevisioning::WriteData(FArchive &Ar)
{
	// Save the character revisions
	int32 Count = CharacterRevisions.Num();
	Ar << Count;
	for (auto& Revision : CharacterRevisions)
	{
		Ar << Revision.Key;
		Revision.Value.SaveLoad(Ar);
	}

	// Save the shop revisions
	Count = ShopRevisions.Num();
	Ar << Count;
	for (auto& Revision : ShopRevisions)
	{
		Ar << Revision.Key;
		Revision.Value.SaveLoad(Ar);
	}
}

bool URevisioning::LoadData()
{
	// Remove previous data
	CharacterRevisions.Empty();
	ShopRevisions.Empty();

	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\RevisioningSavedData.sav"))) return false;

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadData(FromBinary);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void URevisioning::ReadData(FArchive &Ar)
{
	// Load the character revisions
	int32 Count;
	Ar << Count;
	for (int i=0; i<Count; i++)
	{
		int32 Key;
		Ar << Key;

		FCharacterRevisionData Data;
		Data.SaveLoad(Ar);

		CharacterRevisions.Add(Key,Data);
	}

	// Load the shop revisions
	Ar << Count;
	for (int i = 0; i < Count; i++)
	{
		int32 Key;
		Ar << Key;

		FShopRevisionData Data;
		Data.SaveLoad(Ar);

		ShopRevisions.Add(Key, Data);
	}
}

void URevisioning::AddNPC(ADefaultNPC& NPC)
{
	NPCs.AddUnique(&NPC);

	FCharacterRevisionData* RevisionData = CharacterRevisions.Find(NPC.GetCharacterComponent()->GetTableID());

	if (RevisionData)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Set revision data for NPC"));
		const FCharacterRevision& Data = RevisionData->Revision;
		if (Data.Reposition)
		{
			NPC.SetActorLocation(Data.Location);
			NPC.SetActorRotation(Data.Rotation);

			UE_LOG(LogTemp, Log, TEXT("NPC %s's location and rotation are set according to its revision"), *NPC.GetName());
		}

		NPC.SetActorHiddenInGame(Data.Hidden);
		NPC.SetActorEnableCollision(!Data.Hidden);
		NPC.SetActorTickEnabled(!Data.Hidden);

		if (Data.Hidden)
		{
			UE_LOG(LogTemp, Log, TEXT("Interactable %s has been made invisible according to its revision"), *NPC.GetName());
		}

		NPC.GetStoryComponent()->SetTableID(Data.StoryID);
	}
}
