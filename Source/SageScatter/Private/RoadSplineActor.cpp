// 2023 Green Rain Studios


#include "RoadSplineActor.h"


// Sets default values
ARoadSplineActor::ARoadSplineActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARoadSplineActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoadSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

