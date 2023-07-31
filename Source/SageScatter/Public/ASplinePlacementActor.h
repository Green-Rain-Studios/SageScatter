// 2023 Green Rain Studios

#pragma once

#include "CoreMinimal.h"
#include "PlacementActorBase.h"
#include "ASplinePlacementActor.generated.h"

UENUM(BlueprintType, meta=(DisplayName="Instance Placement Type"))
enum class EInstancePlacementType : uint8
{
	IPT_GAP			UMETA(DisplayName = "Place with gap"),
	IPT_POINT		UMETA(DisplayName = "Place at spline point")
};
USTRUCT(BlueprintType)
struct FMeshProfileInstance
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ShowOnlyInnerProperties))
	FMeshProfile MeshData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile")
	EInstancePlacementType PlacementType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(EditCondition="PlacementType==EInstancePlacementType::IPT_Gap", EditConditionHides))
	float Gap = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(EditCondition="PlacementType==EInstancePlacementType::IPT_Gap", EditConditionHides))
	float StartOffset = 0.f;
};

USTRUCT(BlueprintType)
struct FMeshProfileSpline
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ShowOnlyInnerProperties))
	FMeshProfile MeshData;

	// Offset for spline start (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ClampMin=0, ClampMax=1))
	float StartOffset = 0.f;

	// Total distance this spline mesh should cover from start (set to 0 for full length of spline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ClampMin=0, ClampMax=1))
	float DistanceCovered = 0.f;
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

	// Place instances along spline at regular distances
	bool CalculateTransformsAtRegularDistances(float SplineLength, FMeshProfileInstance MeshProfile, TArray<FTransform> &OutTransforms);
	// Place instances along spline at spline points
	bool CalculateTransformsAtSplinePoints(FMeshProfileInstance MeshProfile, TArray<FTransform> &OutTransforms);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", meta=(ShowOnlyInnerProperties))
	TArray<FMeshProfileInstance> InstancedMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", meta=(ShowOnlyInnerProperties))
	TArray<FMeshProfileSpline> SplineMeshes;

protected:
	UPROPERTY(VisibleAnywhere)
	class USplineComponent* Spline;

	UPROPERTY()
	UBillboardComponent* Icn;

	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> ISMs;
};
