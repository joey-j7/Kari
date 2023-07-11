

#pragma once

#include <string>
#include <array>
#include <utility>

#include "CoreMinimal.h"
#include "UI/NavigatableWidget.h"
#include "Events/DialogueOptionEvent.h"

#include "Border.h"
#include "CanvasPanel.h"
#include "DialogueWidget.generated.h"

class UStoryComponent;
class UPlaylist;

/**
 * 
 */
UCLASS()
class KARI_API UDialogueWidget : public UNavigatableWidget
{
	GENERATED_BODY()
	friend class UStoryComponent;

public:
	UDialogueWidget(const FObjectInitializer& ObjectInitializer);
	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetDialogue(const FText& Text, const FString& Name, int32 CurrentID, int32 NextID);

	void AddOption(const std::string& Text, std::pair<uint32, uint32> Option);

	void OnNavChange_Implementation(bool Reset) override;

	void OnCancel_Implementation() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Dialogue)
	void ClearOptions();

	UFUNCTION(BlueprintImplementableEvent, Category = Dialogue)
	void Hide();

	UFUNCTION(BlueprintImplementableEvent, Category = Dialogue)
	void Show();

	UFUNCTION(BlueprintCallable, Category = Dialogue)
	bool Advance();

	UFUNCTION(BlueprintCallable, Category = Dialogue)
	void Skip();

	int32 GetNextDialogID() const { return NextDialogID; }
	void StopSFX();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetStoryComponent(UStoryComponent* Component);

	void ScrollText(float ScrollSpeed);

	bool SkipHTMLTag(const FString& Tag, const FString& EnclosingTag);

	UFUNCTION(BlueprintImplementableEvent, Category = Dialogue)
	void OnTextChanged(const FText& Text);

	UFUNCTION(BlueprintImplementableEvent, Category = Dialogue)
	void OnOptionAdded(const FString& Text);

	UFUNCTION(BlueprintImplementableEvent, Category = Dialogue)
	void OnOptionsAddedComplete();

	UFUNCTION(BlueprintImplementableEvent, Category = Dialogue)
	void OnAnimate(const FText& Text);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Dialogue)
	void OnAnimationFinish();

	void InitEvents(uint32 DialogueOptionID);

	UFUNCTION()
	void OnInventory() {}

	UFUNCTION()
	void OnPause() {}

	UFUNCTION()
	void OnQuestLog() {}
	
	UPROPERTY(Category = Dialogue, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FText FullText;

	UPROPERTY(Category = Dialogue, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FString CharacterName;

	UPROPERTY(Category = Dialogue, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float ScrollSpeed = 50.f;

	FString ScrollEndingString = "";

	UPROPERTY(Category = Dialogue, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float SkipSpeed = 500.f;

	UPROPERTY(Category = Dialogue, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float TextCounter = 0.f;

	int CurrentTextChar = 0;
	
	UPROPERTY(Category = Dialogue, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool AnimateText = true;

	UPROPERTY(Category = Dialogue, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool SkipText = false;

	UPROPERTY(Category = Dialogue, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool OptionShown = false;

	TArray<std::pair<FString, std::pair<uint32, uint32>>> Options;
	std::pair<uint32, uint32> LeaveOption = { UINT_MAX, UINT_MAX };

	// SFX
	UPlaylist* ScrollSFX = nullptr;
	UPlaylist* SelectSFX = nullptr;
	UPlaylist* ContinueSFX = nullptr;
	UPlaylist* ConfirmSFX = nullptr;

	uint32 DialogID = UINT_MAX;
	uint32 NextDialogID = UINT_MAX;

	UDefaultGameInstance* GameInstance = nullptr;

	UPROPERTY(Category = Dialogue, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStoryComponent* StoryComponent = nullptr;

	UPROPERTY()
	TArray<FDialogueOptionEvent> Events;

	TArray<FString> LeaveOptions = { "Leave", "Back" };
};
