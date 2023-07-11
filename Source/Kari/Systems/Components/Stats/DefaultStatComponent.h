#pragma once
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Systems/Effects/StatEffect.h"
#include "DefaultStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStatEventDelegate_OnEffectAdded, int32, ID, const FStatEffect&, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStatEventDelegate_OnDamaged, bool, IsEffectDamage, float, Amount, AActor*, Source);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStatEventDelegate_OnDepleted);

USTRUCT(BlueprintType)
struct FStatPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	int32 ID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	AActor* Actor = nullptr;

	bool operator== (const FStatPair& Other) const
	{
		return ID == Other.ID && Actor == Other.Actor;
	}

	friend uint32 GetTypeHash(const FStatPair& Other)
	{
		return GetTypeHash(Other.ID);
	}
};

USTRUCT(BlueprintType)
struct FStatTimer
{
	GENERATED_BODY()

public:
	bool operator== (const FStatTimer& Other) const
	{
		return Seconds == Other.Seconds;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float Seconds = -1.0f;
};

UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class KARI_API UDefaultStatComponent : public UActorComponent
{
	friend class UDebugWidget;

	GENERATED_BODY()


public:
	UDefaultStatComponent();

	void InitializeComponent() override;

	void CustomTick();

	UFUNCTION(BlueprintCallable, Category = Stats)
	void Damage(float Amount, AActor* Source);

	UFUNCTION(BlueprintCallable, Category = Stats)
	void Restore(float Amount)
	{
		Current = FMath::Min(Current + Amount, Max);
	}

	UFUNCTION(BlueprintCallable, Category = Stats)
	bool IsDrained() const
	{
		return Current <= 0.f;
	}

	UFUNCTION(BlueprintCallable, Category = Stats)
	float Normalized() const
	{
		return Max == 0.f ? 0.f : Current / Max;
	}

	UFUNCTION(BlueprintCallable, Category = Stats)
	void Reset()
	{
		Current = Max;
	}

	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetCurrent(float Amount);

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetCurrent()
	{
		return Current;
	}

	UFUNCTION(BlueprintCallable, Category = Stats)
	void Pause() { Paused = true; };

	UFUNCTION(BlueprintCallable, Category = Stats)
	void Resume() { Paused = false; };

	UFUNCTION(BlueprintCallable, Category = Stats)
	bool IsPaused() { return Paused; };

	UFUNCTION(BlueprintCallable, Category = Stats)
	FStatEffect GetEffect(int32 ID, AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Stats)
	bool EffectExists(int32 ID, AActor* Actor);

	// Apply stat effect
	UFUNCTION(BlueprintCallable, Category = Stats)
	void ApplyEffect(int32 ID, AActor* Actor, const FStatEffect& Effect); 

	// Apply stat effect, and returns a new, previously unused, effect ID
	UFUNCTION(BlueprintCallable, Category = Stats)
	int32 ApplyEffectUnique(AActor* Actor, const FStatEffect& Effect);

	// Applies stat effect that is removed after set seconds
	UFUNCTION(BlueprintCallable, Category = Stats)
	void ApplyTimedEffect(int32 ID, AActor* Actor, const FStatEffect& Effect, float Seconds);

	// Apply stat effect that is removed after set seconds, and returns a new, previously unused, effect ID
	UFUNCTION(BlueprintCallable, Category = Stats)
	int32 ApplyTimedEffectUnique(AActor* Actor, const FStatEffect& Effect, float Seconds);

	// Applies a timer to an effect that previously did not have a timer, or changes its existing timer's seconds count
	UFUNCTION(BlueprintCallable, Category = Stats)
	void ApplyEffectTimer(int32 ID, AActor* Actor, float Seconds);

	// Set the tick count of an existing timer
	UFUNCTION(BlueprintCallable, Category = Stats)
	void SetEffectTimer(int32 ID, AActor* Actor, float Seconds);

	UFUNCTION(BlueprintCallable, Category = Stats)
	FStatTimer GetEffectTimer(int32 ID, AActor* Actor);

	// Returns if a timer exists for set actor and ID
	UFUNCTION(BlueprintCallable, Category = Stats)
	bool EffectTimerExists(int32 ID, AActor* Actor);

	// Increases the seconds of an existing timer
	UFUNCTION(BlueprintCallable, Category = Stats)
	void EffectTimerIncrement(int32 ID, AActor* Actor, float Seconds);

	// Decreases the seconds of an existing timer
	UFUNCTION(BlueprintCallable, Category = Stats)
	void EffectTimerDecrement(int32 ID, AActor* Actor, float Seconds);

	// Removes a timer from a stat effect without deleting the stat effect
	UFUNCTION(BlueprintCallable, Category = Stats)
	void RemoveEffectTimer(int32 ID, AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Stats)
	void RemoveEffect(int32 ID, AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Stats)
	void RemoveActorEffects(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Stats)
	void RemoveActorTimers(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = Stats)
	void ClearEffects();

	UFUNCTION(BlueprintCallable, Category = Stats)
	void ClearTimers();

	FTimerHandle& GetTimerHandle()
	{
		return TimerHandle;
	}

	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FStatEventDelegate_OnEffectAdded OnEffectAdded;

	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FStatEventDelegate_OnDamaged OnDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FStatEventDelegate_OnDepleted OnDepleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0), Category = "Stat")
	float Max = 255.f;

	// 1 is resist nothing, 0 is resist everything
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0), Category = "Stat")
	float ResistanceMultiplier = 1.0f;

	UFUNCTION(BlueprintCallable, Category = Stats)
	float GetTimerPeriod() const { return TimerPeriod; };

	float TickRate = 0.0f;

	// Save/Load
	bool SaveData(bool Traversal);
	void WriteData(FArchive &Ar);
	bool LoadData(AActor* Actor, bool Traversal);
	void ReadData(FArchive &Ar, AActor* Actor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	TMap<FStatPair, FStatEffect> StatEffects;

	bool Paused = false;

	float Current = Max;
	FTimerHandle TimerHandle;

	int32 IDCounter = -1;

	const float TimerPeriod = 1.0f / 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	TMap<FStatPair, FStatTimer> Timers;
};