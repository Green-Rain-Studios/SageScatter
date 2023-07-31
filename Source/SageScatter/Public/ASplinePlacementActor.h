﻿// 2023 Green Rain Studios

#pragma once

#include "CoreMinimal.h"
#include "PlacementActorBase.h"
#include "ASplinePlacementActor.generated.h"

USTRUCT(BlueprintType)
struct FMeshProfileSpline
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ShowOnlyInnerProperties))
	FMeshProfile MeshData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile")
	float Gap = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile")
	float StartOffset = 0.f;
};

UCLASS(Blueprintable, meta=(DisplayName="Spline Placement Actor", PrioritizeCategories="Setup"))
class SAGESCATTER_API AASplinePlacementActor : public APlacementActorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AASplinePlacementActor();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Spline placement functions
	void RepopulateISMs();

	// Place instances of meshes along the spline
	void PlaceInstancesAlongSpline();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", meta=(ShowOnlyInnerProperties))
	TArray<FMeshProfileSpline> Meshes;
	
protected:
	UPROPERTY(VisibleAnywhere)
	class USplineComponent* Spline;

	UPROPERTY()
	UBillboardComponent* Icn;

	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> ISMs;
};
