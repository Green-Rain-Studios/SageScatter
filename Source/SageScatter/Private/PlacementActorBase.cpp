// 2023 Green Rain Studios


#include "PlacementActorBase.h"

// Sets default values
APlacementActorBase::APlacementActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlacementActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlacementActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

