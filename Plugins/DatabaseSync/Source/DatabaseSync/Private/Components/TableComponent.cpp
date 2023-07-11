#include "TableComponent.h"
#include "TableDatabase.h"

UTableComponent::UTableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UTableComponent::UTableComponent(const FString& name, const FString& dropdownColumn, const FString& linkedColumn, uint32 linkedId) : UTableComponent()
{
	Init(name, dropdownColumn, linkedColumn, linkedId);
}

void UTableComponent::Init(const FString& name, const FString& dropdownColumn, const FString& linkedColumn, uint32 linkedId)
{
	Name = name;
	DropdownColumn = dropdownColumn;

	LinkedColumn = linkedColumn;
	LinkedID = linkedId;
}

void UTableComponent::SetTableID(uint32 id)
{
	if (ID == id)
		return;

	ID = id;
	OnTableIDChanged();
}

void UTableComponent::OnTableIDChanged()
{
	TableDatabase::Get().Refresh();
}

void UTableComponent::SetLinkedID(uint32 Id)
{
	if (Id == LinkedID)
		return;

	LinkedID = Id;
}

const FTableField* UTableComponent::GetField(const FString& tableName, const FString& columnName, uint32 id) const
{
	return TableDatabase::Get().GetField(tableName, columnName, id);
}

const FTableRow& UTableComponent::GetRow(const FString& tableName, uint32 id) const
{
	return TableDatabase::Get().GetRow(tableName, id);
}

const FTableData& UTableComponent::GetTable(const FString& tableName)
{
	return TableDatabase::Get().GetTable(tableName);
}

TPair<uint32, const FTableRow*> UTableComponent::FindLink(const FString& tableName, const FString& columnName, uint32 id) const
{
	return TableDatabase::Get().FindLink(tableName, columnName, id);
}

TArray<TPair<uint32, const FTableRow*>> UTableComponent::FindLinks(const FString& tableName, const FString& columnName, uint32 id) const
{
	return TableDatabase::Get().FindLinks(tableName, columnName, id);
}