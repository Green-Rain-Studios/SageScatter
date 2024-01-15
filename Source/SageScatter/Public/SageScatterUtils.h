// 2023 Green Rain Studios

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SageScatterUtils.generated.h"

USTRUCT(BlueprintType)
struct FMeshProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile")
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile")
	FTransform Offset = FTransform::Identity;
	
};

/**
 * 
 */
UCLASS()
class SAGESCATTER_API USageScatterUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Given a position vector and up, right, and forward vectors, calculate correct result vector offset by those values
	UFUNCTION(BlueprintPure, Category="SageScatter|Helper")
	static FVector CalculateOffsets(FVector Offset, FVector Forward, FVector Right, FVector Up);
	UFUNCTION(BlueprintPure, Category="SageScatter|Helper")
	static FRotator MakeRotatorFromAxes(FVector Forward, FVector Right, FVector Up);
};
