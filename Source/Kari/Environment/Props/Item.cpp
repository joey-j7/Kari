#include "Item.h"

#include <string>

#include "Engine/Engine.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/SoftObjectPath.h"
#include "GameFramework/Actor.h"

#include "TableRow.h"
#include "TableObject.h"

#include "Environment/Props/ItemBase.h"

FItem FItem::ConvertRow(const FTableRow& Row, int32 ID)
{
	FItem Item;
	Item.ID = ID;

	if (Row.Fields.Num() == 0)
	{
		return Item;
	}

	for (auto& field : Row.Fields)
	{
		if (field.Key == "description")
		{
			std::string name;
			name.resize(field.Value.Size);

			memcpy((void*)name.c_str(), field.Value.Data.Get(), field.Value.Size);
			Item.Description = UTF8_TO_TCHAR(name.c_str());
		}

		else if (field.Key == "name")
		{
			std::string name;
			name.resize(field.Value.Size);

			memcpy((void*)name.c_str(), field.Value.Data.Get(), field.Value.Size);
			Item.Name = UTF8_TO_TCHAR(name.c_str());
		}

		else if (field.Key == "Despawnable")
		{
			memcpy((void*)&Item.Despawnable, field.Value.Data.Get(), field.Value.Size);
		}

		else if (field.Key == "Droppable")
		{
			memcpy((void*)&Item.Droppable, field.Value.Data.Get(), field.Value.Size);
		}

		else if (field.Key == "Equippable")
		{
			memcpy((void*)&Item.Equippable, field.Value.Data.Get(), field.Value.Size);
		}

		else if (field.Key == "UseOnPickup")
		{
			memcpy((void*)&Item.UseOnPickup, field.Value.Data.Get(), field.Value.Size);
		}

		else if (field.Key == "sellValue")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Item.SellValue = d;
		}

		else if (field.Key == "sellable")
		{
			memcpy((void*)&Item.Sellable, field.Value.Data.Get(), field.Value.Size);
		}

		else if (field.Key == "stackLimit")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			Item.StackLimit = d;
		}

		else if (field.Key == "effectTypeId")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);

			d = FMath::Min(d, (double)EEffectType::E_UNDEFINED);
			Item.Effect.EffectType = EEffectType((uint8)d);
		}

		else if (field.Key == "effectTime")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);
			Item.Effect.Time = d;
		}

		else if (field.Key == "effectPrimaryValue")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);
			Item.Effect.PrimaryValue = d;
		}

		else if (field.Key == "effectSecondaryValue")
		{
			double d;
			memcpy((void*)&d, field.Value.Data.Get(), field.Value.Size);
			Item.Effect.SecondaryValue = d;
		}
	}

	return Item;
}

void FItem::SaveLoad(FArchive &Ar)
{
	Ar << ID;  //int32
	Ar << Description; //string
	Ar << Name;  //string
	Ar << Despawnable; //bool
	Ar << Droppable; //bool
	Ar << Equippable; //bool
	Ar << UseOnPickup; //bool
	Ar << SellValue; //int32
	Ar << Sellable; //bool
	Ar << StackLimit; //int32
	Effect.SaveLoad(Ar);

	Ar << ClassPath;

	// FOR DEBUGGING
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Path);




	// MIGHT LOOK INTO THIS LATER, IF NOT, WILL DELETE IT
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Class->GetAssetPathName());
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Class->GetFName().ToString());
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Object->GetFName().ToString());

	//FSoftObjectPath ObjectPath(Class);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ObjectPath.GetAssetName());

	//UClass tClass = LoadObject<UClass>(nullptr, ObjectName);
	//AActor* test = GetWorld()->SpawnActor<Class>(Class::StaticClass());

	//AItemBase* Actor = Cast<AItemBase>(GetWorld()->SpawnActor(Class));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Actor->Name);

	//GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Red, GetFName().ToString());
}