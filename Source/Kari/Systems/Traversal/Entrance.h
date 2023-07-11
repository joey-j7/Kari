#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Entrance.generated.h"

/**
 *
 */
UCLASS()
class KARI_API AEntrance : public AActor
{
	GENERATED_BODY()

public:
	AEntrance() = default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Entrance)
	FString Name = TEXT("Rename");
};