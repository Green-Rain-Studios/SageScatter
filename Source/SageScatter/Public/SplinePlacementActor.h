// 2023 Green Rain Studios

#pragma once

#include "CoreMinimal.h"
#include "PlacementActorBase.h"
#include "Components/SplineMeshComponent.h"
#include "SplinePlacementActor.generated.h"

UENUM(BlueprintType, meta=(DisplayName="Instance Placement Type"))
enum class EInstancePlacementType : uint8
{
	IPT_GAP			UMETA(DisplayName = "Place with gap"),
	IPT_POINT		UMETA(DisplayName = "Place at spline point")
};

UENUM(BlueprintType, meta = (DisplayName = "Spline Placement Type"))
enum class ESplinePlacementType : uint8
{
	SPT_LOOPED		UMETA(DisplayName = "Repeat spline mesh"),
	SPT_SINGLE		UMETA(DisplayName = "Single spline mesh")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Profile")
	ESplinePlacementType PlacementType;

	// "Relaxes" the geometry, for performance. Too high values can lead to mesh not following the spline closely. Reset to original factor with 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ClampMin=0, EditCondition="PlacementType==ESplinePlacementType::SPT_LOOPED", EditConditionHides))
	float RelaxMultiplier = 1.f;

	// Offset for spline start (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ClampMin=0, ClampMax=1, EditCondition="PlacementType==ESplinePlacementType::SPT_LOOPED", EditConditionHides))
	float StartOffset = 0.f;

	// Total distance this spline mesh should cover from start (set to 1 for full length of spline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mesh Profile", meta=(ClampMin=0, ClampMax=1, EditCondition="PlacementType==ESplinePlacementType::SPT_LOOPED", EditConditionHides))
	float EndOffset = 1.f;

	// Distance in units for single spline placement start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Profile", meta = (ClampMin=0, EditCondition="PlacementType==ESplinePlacementType::SPT_SINGLE", EditConditionHides))
	float StartDistance = 0.f;

	// Length of the spline from Start Distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Profile", meta = (ClampMin=0, EditCondition="PlacementType==ESplinePlacementType::SPT_SINGLE", EditConditionHides))
	float MeshLength = 100.f;
};

UCLASS(Blueprintable, meta=(DisplayName="Spline Placement Actor", PrioritizeCategories="Setup"))
class SAGESCATTER_API ASplinePlacementActor : public APlacementActorBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASplinePlacementActor();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditUndo() override;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Instance placement functions
	void RepopulateISMs();

	// Place instances of meshes along the spline
	void PlaceInstancesAlongSpline();

	// Place instances along spline at regular distances
	bool CalculateTransformsAtRegularDistances(float SplineLength, FMeshProfileInstance MeshProfile, TArray<FTransform> &OutTransforms);
	// Place instances along spline at spline points
	bool CalculateTransformsAtSplinePoints(FMeshProfileInstance MeshProfile, TArray<FTransform> &OutTransforms);

	// Spline Mesh placement functions
	void RecalculateSplineMeshes();

	// Place Spline Mesh components
	void PlaceSplineMeshComponentsAlongSpline();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", meta=(ShowOnlyInnerProperties))
	TArray<FMeshProfileInstance> InstancedMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup", meta=(ShowOnlyInnerProperties))
	TArray<FMeshProfileSpline> SplineMeshes;

protected:
	UPROPERTY(VisibleDefaultsOnly)
	class USplineComponent* Spline;

	UPROPERTY()
	UBillboardComponent* Icn;

	UPROPERTY()
	TArray<UHierarchicalInstancedStaticMeshComponent*> ISMs;

	UPROPERTY()
	TArray<USplineMeshComponent*> SMCs;

	FTransform GetTransformAtDistanceAlongSpline(float Distance);
	void GetDirectionVectorsAtDistanceAlongSpline(float Distance, FVector& Fwd, FVector& Right, FVector& Up);
};
