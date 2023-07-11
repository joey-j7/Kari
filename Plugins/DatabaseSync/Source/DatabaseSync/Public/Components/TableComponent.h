#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Engine/DataTable.h"
#include "TableField.h"

#include "TableData.h"
#include "TableType.h"

#include "TableComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DATABASESYNC_API UTableComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class FTableSelector;

public:
	// Sets default values for this component's properties
	UTableComponent();
	UTableComponent(const FString& name, const FString& dropdownColumn, const FString& linkedColumn = "", uint32 linkedId = UINT_MAX);

	void Init(const FString& name, const FString& dropdownColumn, const FString& linkedColumn = "", uint32 linkedId = UINT_MAX);

	UFUNCTION(BlueprintCallable)
	int32 TableID() const { return static_cast<int32>(ID); }
	
	uint32 GetTableID() const { return ID; }
	void SetTableID(uint32 id);
	virtual void OnTableIDChanged();

	const FString& GetTableName() const { return Name; }
	const FString& GetDropdownColumn() const { return DropdownColumn; }
	const FString& GetLinkedColumn() const { return LinkedColumn; }

	uint32 GetLinkedID() const { return LinkedID; }
	void SetLinkedID(uint32 Id);

	const FTableField* GetField(const FString& tableName, const FString& columnName, uint32 id) const;
	const FTableRow& GetRow(const FString& tableName, uint32 id) const;
	const FTableData& GetTable(const FString& tableName);
	TPair<uint32, const FTableRow*> FindLink(const FString& tableName, const FString& columnName, uint32 id) const;
	TArray<TPair<uint32, const FTableRow*>> FindLinks(const FString& tableName, const FString& columnName, uint32 id) const;

	UPROPERTY()
	bool DropdownDisabled = false;

protected:
	// @brief - Keep in mind uint32 is not supported so you can never find this is the Blueprint editor!
	UPROPERTY(EditAnywhere, Category = "Table Properties")
	uint32 ID = UINT_MAX;

	UPROPERTY(VisibleAnywhere, Category = "Table Properties")
	FString LinkedColumn = "";

	UPROPERTY(VisibleAnywhere, Category = "Table Properties")
	uint32 LinkedID = UINT_MAX;

	UPROPERTY(VisibleAnywhere, Category = "Table Properties")
	FString Name = "";

	UPROPERTY()
	FString DropdownColumn = "";
};
