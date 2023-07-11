// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Quests/Quest.h"
#include "UI/NavigatableWidget.h"
#include "QuestLogWidget.generated.h"


class UQuestLog;

/**
 * 
 */
UCLASS()
class KARI_API UQuestLogWidget : public UNavigatableWidget
{
	GENERATED_BODY()
public:
	void NativeConstruct() override;
	void NativeDestruct() override;

	void OnCancel_Implementation() override;
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Quest Log Widget")
	FString GetCharacterName();

	UFUNCTION(BlueprintCallable, Category = "Quest Log Widget")
    FString GenerateHeader(const FQuest& Quest);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest Log Widget")
    void OnInventory();

	UFUNCTION()
    void OnPause() {}

	UPROPERTY(BlueprintReadWrite, Category = "Quest Log Widget")
	FQuest CurrentQuest = {}; 

	UPROPERTY(BlueprintReadWrite)
	UQuestLog* QuestLog = nullptr;
};
