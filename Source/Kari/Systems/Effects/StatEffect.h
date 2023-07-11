#pragma once

#include "StatEffect.generated.h"


USTRUCT(BlueprintType)
struct FStatEffect
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Effect")
	float Amount = 0.0f;

	//1 is resist nothing, 0 is resist fully
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Effect")
	float ResistanceMultiplier = 1.0f;
};