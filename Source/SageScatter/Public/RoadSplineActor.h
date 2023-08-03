// 2023 Green Rain Studios

#pragma once

#include "CoreMinimal.h"
#include "SplinePlacementActor.h"
#include "RoadSplineActor.generated.h"

UCLASS()
class SAGESCATTER_API ARoadSplineActor : public ASplinePlacementActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoadSplineActor();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
};
