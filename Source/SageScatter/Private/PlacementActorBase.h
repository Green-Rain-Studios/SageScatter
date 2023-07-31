// 2023 Green Rain Studios

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SageScatterUtils.h"
#include "PlacementActorBase.generated.h"

UCLASS(Abstract, NotBlueprintable, NotBlueprintType)
class APlacementActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlacementActorBase();
		
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
