// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "TableBPFunctions.generated.h"

//above name must match the name on your hard disk for this .h file

//note about UBlueprintFunctionLibrary
// This class is a base class for any function libraries exposed to blueprints.
// Methods in subclasses are expected to be static

UCLASS()
class KARI_API UTableBPFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = Database)
	static FString GetTableString(const FString& TableName, const FString& FieldName, int32 ID);

	UFUNCTION(BlueprintCallable, Category = Database)
	static float GetTableFloat(const FString& TableName, const FString& FieldName, int32 ID);

	UFUNCTION(BlueprintCallable, Category = Database)
	static int32 GetTableInt(const FString& TableName, const FString& FieldName, int32 ID);
};