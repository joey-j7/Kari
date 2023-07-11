
 
#include "CraftingComponent.h"
 
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
 
#include "General/DefaultGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Environment/Props/ItemFunctionLibrary.h"
 
UCraftingComponent::UCraftingComponent() : UDefaultCraftingComponent("recipes", "childId", "parentId")
{
	DropdownDisabled = true;
	ID = 0;
}
 
TArray<FCraftSlot> UCraftingComponent::PopulateCraftableItems_Implementation()
{
	TArray<FCraftSlot> Slots;
 
	// get the table
	// const FTableData& tblData = GetTable("recipes");
	TArray<TPair<uint32, const FTableRow*>> Links = FindLinks("recipes", "parentId", ID);
 
	for (auto& Row : Links)
	{
		FCraftSlot Slot;
 
		for (auto& Field : Row.Value->Fields)
		{	
			// get the childId field to get the requirements
			double i;
			
			// if our parentId is equal to ours and childId is the column name
			if (Field.Key.ColumnName != "childId" )
				continue;
			
			// get the childId field to get the requirements
			memcpy(&i, Field.Value.Data.Get(), Field.Value.Size);
 
			// Store the item in that is craft able
			Slot.Craftable = FItem::ConvertRow(GetRow("items", i), i);

			TArray<TPair<uint32, const FTableRow*>> ConditionLinks = FindLinks("recipeCraftConditions", "parentId", i);
 
			for (auto& LinkRow : ConditionLinks)
			{
				FCraftRequirement Requirement;
 
				for (auto& LinkField : LinkRow.Value->Fields)
				{
					// Grab the item
					if (LinkField.Key.ColumnName == "childId")
					{
						double d;
						memcpy(&d, LinkField.Value.Data.Get(), LinkField.Value.Size);
						
						const FTableRow& row = GetRow("items", d);
						Requirement.Item = FItem::ConvertRow(row, d);
					}
					// Grab the amount that we should craft
					else if (LinkField.Key.ColumnName == "amount")
					{
						double d;
						memcpy(&d, LinkField.Value.Data.Get(), LinkField.Value.Size);
 
						Requirement.Amount = d;
					}
				}
 
				Slot.Requirements.Add(Requirement);
 
			}
		}
 
		Slots.Add(Slot);
	}
 
	return Slots;
}