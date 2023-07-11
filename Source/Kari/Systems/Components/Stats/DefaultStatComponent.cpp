#include "DefaultStatComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/FileHelper.h"

UDefaultStatComponent::UDefaultStatComponent()
{
	bWantsInitializeComponent = true;
}

void UDefaultStatComponent::InitializeComponent()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDefaultStatComponent::CustomTick, TimerPeriod, true);
}

void UDefaultStatComponent::CustomTick()
{
	if (Paused)
		return;

	// Update timers (early to prevent stat effect execution when timer has manually been set to finished before the tick)
	for (auto timerIt = Timers.CreateIterator(); timerIt; ++timerIt)
	{
		timerIt.Value().Seconds -= TimerPeriod;
		if (timerIt.Value().Seconds <= 0.0f)
		{
			StatEffects.Remove(timerIt.Key());
			timerIt.RemoveCurrent();
		}
	}

	TickRate = 0.0f;
	for (auto& Effect : StatEffects)
	{
		float amount = Effect.Value.Amount;
		if (amount < 0.0f)
			amount *= FMath::Clamp(Effect.Value.ResistanceMultiplier, 0.0f, 1.0f) *
					  FMath::Clamp(ResistanceMultiplier, 0.0f, 1.0f);

		OnDamaged.Broadcast(true, amount, Effect.Key.Actor);

		TickRate += amount;
	}

	Current = FMath::Min(Current + TickRate, Max);
	if (Current < 0 && TickRate != 0.0f)
		OnDepleted.Broadcast();

	// Decrement timers (late to prevent state effect discarding for single-tick timers)
	for (auto& Timer : Timers)
		Timer.Value.Seconds -= TimerPeriod;
}

void UDefaultStatComponent::Damage(float Amount, AActor* Source)
{
	Amount *= ResistanceMultiplier;

	OnDamaged.Broadcast(false, Amount, Source);
	Current -= Amount;

	if (Current < 0)
		OnDepleted.Broadcast();
}

void UDefaultStatComponent::SetCurrent(float Amount)
{
	Current = FMath::Min(Amount, Max);
	if (Current < 0)
		OnDepleted.Broadcast();
}

FStatEffect UDefaultStatComponent::GetEffect(int32 ID, AActor* Actor)
{
	auto find = StatEffects.Find({ ID, Actor });

	if (!find)
		return {};

	return *find;
}

bool UDefaultStatComponent::EffectExists(int32 ID, AActor* Actor)
{
	return (nullptr != StatEffects.Find({ ID, Actor }));
}

void UDefaultStatComponent::ApplyEffect(int32 ID, AActor* Actor, const FStatEffect& Effect)
{
	bool Exists = EffectExists(ID, Actor);
	StatEffects.FindOrAdd({ ID, Actor }) = Effect;
	if (!Exists)
		OnEffectAdded.Broadcast(ID, Effect);
}

int32 UDefaultStatComponent::ApplyEffectUnique(AActor* Actor, const FStatEffect& Effect)
{
	StatEffects.FindOrAdd({ IDCounter, Actor }) = Effect;
	OnEffectAdded.Broadcast(IDCounter, Effect);
	return IDCounter--;
}

void UDefaultStatComponent::ApplyTimedEffect(int32 ID, AActor* Actor, const FStatEffect& Effect, float Seconds)
{
	bool Exists = EffectExists(ID, Actor);
	StatEffects.FindOrAdd({ ID, Actor }) = Effect;
	Timers.FindOrAdd({ ID, Actor }) = { Seconds };
	if (!Exists)
		OnEffectAdded.Broadcast(ID, Effect);
}

int32 UDefaultStatComponent::ApplyTimedEffectUnique(AActor* Actor, const FStatEffect& Effect, float Seconds)
{
	OnEffectAdded.Broadcast(IDCounter, Effect);
	StatEffects.FindOrAdd({ IDCounter, Actor }) = Effect;
	Timers.FindOrAdd({ IDCounter, Actor }) = { Seconds };
	return IDCounter--;
}

void UDefaultStatComponent::ApplyEffectTimer(int32 ID, AActor* Actor, float Seconds)
{
	// Makes sure that an effect exists before setting a timer to it
	auto find = StatEffects.Find({ ID, Actor });
	if (!find)
		return;

	Timers.FindOrAdd({ ID, Actor }) = { Seconds };
}

void UDefaultStatComponent::SetEffectTimer(int32 ID, AActor* Actor, float Seconds)
{
	auto find = Timers.Find({ ID, Actor });

	if (find)
		find->Seconds  = Seconds;
	else
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, FString::Printf(TEXT("Timer non-existant for Actor type: %s, and ID: %d"), *UKismetSystemLibrary::GetDisplayName(Actor), ID));
}

FStatTimer UDefaultStatComponent::GetEffectTimer(int32 ID, AActor* Actor)
{
	auto find = Timers.Find({ ID, Actor });

	if (!find)
		return {};

	return *find;
}

bool UDefaultStatComponent::EffectTimerExists(int32 ID, AActor* Actor)
{
	return (nullptr != Timers.Find({ ID, Actor }));
}

void UDefaultStatComponent::EffectTimerIncrement(int32 ID, AActor* Actor, float Seconds)
{
	auto find = Timers.Find({ ID, Actor });

	if (find)
		find->Seconds += Seconds;
	else
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, FString::Printf(TEXT("Timer non-existant for Actor type: %s, and ID: %d"), *UKismetSystemLibrary::GetDisplayName(Actor), ID));
}

void UDefaultStatComponent::EffectTimerDecrement(int32 ID, AActor* Actor, float Seconds)
{
	auto find = Timers.Find({ ID, Actor });

	if (find)
		find->Seconds -= Seconds;
	else
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, FString::Printf(TEXT("Timer non-existant for Actor type: %s, and ID: %d"), *UKismetSystemLibrary::GetDisplayName(Actor), ID));
}

void UDefaultStatComponent::RemoveEffectTimer(int32 ID, AActor* Actor)
{
	Timers.Remove({ ID, Actor });
}

void UDefaultStatComponent::RemoveEffect(int32 ID, AActor* Actor)
{
	StatEffects.Remove({ ID, Actor });
	Timers.Remove({ ID, Actor });
}

void UDefaultStatComponent::RemoveActorEffects(AActor* Actor)
{
	// Stat effects
	TArray<FStatPair> Keys;
	StatEffects.GetKeys(Keys);

	for (auto& Key : Keys)
	{
		if (Key.Actor == Actor)
		{
			StatEffects.Remove(Key);
		}
	}

	// Timers
	Timers.GetKeys(Keys);

	for (auto& Key : Keys)
	{
		if (Key.Actor == Actor)
		{
			Timers.Remove(Key);
		}
	}
}

void UDefaultStatComponent::RemoveActorTimers(AActor* Actor)
{
	// Timers
	TArray<FStatPair> Keys;
	Timers.GetKeys(Keys);

	for (auto& Key : Keys)
	{
		if (Key.Actor == Actor)
		{
			Timers.Remove(Key);
		}
	}
}

void UDefaultStatComponent::ClearEffects()
{
	StatEffects.Empty();
	Timers.Empty();
}

void UDefaultStatComponent::ClearTimers()
{
	Timers.Empty();
}

bool UDefaultStatComponent::SaveData(bool Traversal)
{
	//Save the data to binary
	FBufferArchive ToBinary;
	WriteData(ToBinary);

	//No data were saved
	if (ToBinary.Num() <= 0) return false;

	//Save binaries to disk
	bool result;
	if (Traversal)
	{
		result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Persistence\\BlessingSavedData.sav"));
	}
	else
	{
		result = FFileHelper::SaveArrayToFile(ToBinary, TEXT("Saving\\BlessingSavedData.sav"));
	}

	//Empty the buffer's contents
	ToBinary.FlushCache();
	ToBinary.Empty();

	return result;
}
void UDefaultStatComponent::WriteData(FArchive &Ar)
{
	Ar << IDCounter; //int32
	Ar << Current; //float
	Ar << TickRate; //float

	// Save stat effects
	int32 Count = StatEffects.Num();
	Ar << Count;
	for (auto& Stat : StatEffects)
	{
		Ar << Stat.Key.ID; //int32

		Ar << Stat.Value.Amount; //float
		Ar << Stat.Value.ResistanceMultiplier; //float
	}

	// Save stat timers
	Count = Timers.Num();
	Ar << Count;
	for (auto& Timer : Timers)
	{
		Ar << Timer.Key.ID; //int32

		Ar << Timer.Value.Seconds; //float
	}
}

bool UDefaultStatComponent::LoadData(AActor* Actor, bool Traversal)
{
	TArray<uint8> BinaryArray;

	//load disk data to binary array
	if (Traversal)
	{
		if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Persistence\\BlessingSavedData.sav"))) return false;
	}
	else
	{
		if (!FFileHelper::LoadFileToArray(BinaryArray, TEXT("Saving\\BlessingSavedData.sav"))) return false;
	}

	if (BinaryArray.Num() <= 0) return false;

	//Memory reader is the archive that we're going to use in order to read the loaded data
	FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
	FromBinary.Seek(0);
	ReadData(FromBinary, Actor);

	//Empty the buffer's contents
	FromBinary.FlushCache();
	BinaryArray.Empty();
	//Close the stream
	FromBinary.Close();

	return true;
}
void UDefaultStatComponent::ReadData(FArchive &Ar, AActor* Actor)
{
	// Empty the maps
	StatEffects.Empty();
	Timers.Empty();
	
	Ar << IDCounter; //int32
	Ar << Current; //float
	Paused = false; // Force pause to false
	Ar << TickRate; //float

	// Load stat effects
	int32 Count;
	Ar << Count;

	for (int32 i=0; i<Count; i++)
	{
		FStatPair Pair;
		Ar << Pair.ID; //int32
		Pair.Actor = Actor;
		
		FStatEffect Stat;
		Ar << Stat.Amount; //float
		Ar << Stat.ResistanceMultiplier; //float

		StatEffects.Add(Pair, Stat);
	}

	// Load stat timers
	Ar << Count;

	for (int32 i = 0; i < Count; i++)
	{
		FStatPair Pair;
		Ar << Pair.ID; //int32
		Pair.Actor = Actor;

		FStatTimer Timer;
		Ar << Timer.Seconds; //float

		Timers.Add(Pair, Timer);
	}
}