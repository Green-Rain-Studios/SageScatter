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
	
};
