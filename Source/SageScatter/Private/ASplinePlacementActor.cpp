// 2023 Green Rain Studios


#include "ASplinePlacementActor.h"

#include "Components/BillboardComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"


// Sets default values
AASplinePlacementActor::AASplinePlacementActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root spline component
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->EditorUnselectedSplineSegmentColor = FLinearColor(0, 1.0, 0.376262);
	Spline->EditorTangentColor = FLinearColor(0.921, 0.299687, 0);
	RootComponent = Spline;

	// Billboard for icon
	static ConstructorHelpers::FObjectFinder<UTexture2D> ICN(TEXT("/Engine/EngineResources/Cursors/SplitterHorz.SplitterHorz"));	
	Icn = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icn"));
	Icn->Sprite = ICN.Object;
	Icn->bIsScreenSizeScaled = true;
	Icn->ScreenSize = 0.05f;
	Icn->SetHiddenInGame(true);
	Icn->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AASplinePlacementActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AASplinePlacementActor::RepopulateISMs()
{
	// Delete previous ISMs
	for (int i = 0; i < ISMs.Num(); i++)
	{
		ISMs[i]->UnregisterComponent();
		ISMs[i]->DestroyComponent();
	}
	
	// Clear ISM array and create a new one
	ISMs = TArray<UHierarchicalInstancedStaticMeshComponent*>();

	// Iterate and add each mesh profile as a new ISM
	for(int i = 0; i < InstancedMeshes.Num(); i++)
	{
		// Check if mesh is null first
		if(InstancedMeshes[i].MeshData.Mesh == nullptr)
			continue;
		
		UHierarchicalInstancedStaticMeshComponent* ism = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		ism->SetStaticMesh(InstancedMeshes[i].MeshData.Mesh);
		ism->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		ism->RegisterComponent();
		ISMs.Add(ism);
	}
}

void AASplinePlacementActor::PlaceInstancesAlongSpline()
{
	// First we clear all ISMs of their instances
	for(int i = 0; i < ISMs.Num(); i++)
	{
		ISMs[i]->ClearInstances();
	}

	// Then we populate based on total length of spline
	float splineLength = Spline->GetSplineLength();

	for(int i = 0; i < ISMs.Num(); i++)
	{
		// Error checking. If mesh does not exist then skip this one
		if(InstancedMeshes[i].MeshData.Mesh == nullptr)
			continue;

		// Different placement types will generate different lists of transforms
		TArray<FTransform> transforms;
		switch (InstancedMeshes[i].PlacementType)
		{
		case EInstancePlacementType::IPT_GAP:
			if(CalculateTransformsAtRegularDistances(splineLength, InstancedMeshes[i], transforms))
				break;

		case EInstancePlacementType::IPT_POINT:
			if(CalculateTransformsAtSplinePoints(InstancedMeshes[i], transforms))
			 	break;
		}
		
		// Add instances to ISM
		ISMs[i]->AddInstances(transforms,false);
	}
}

void AASplinePlacementActor::PlaceSplineMeshesLooped()
{
	// First, clear all SplineMeshes
	for (int i = 0; i < SMCs.Num(); i++)
	{
		SMCs[i]->UnregisterComponent();
		SMCs[i]->DestroyComponent();
	}

	// Clear array
	SMCs = TArray<USplineMeshComponent*>();

	// Iterate and add relevant spline mesh for each mesh profile
	for (int i = 0; i < SplineMeshes.Num(); i++)
	{
		// Check if mesh is null first, and if spline placement is looped
		if (SplineMeshes[i].MeshData.Mesh == nullptr)
			continue;
		if (SplineMeshes[i].PlacementType != ESplinePlacementType::SPT_LOOPED)
			continue;

		float splineTotalDistance = Spline->GetSplineLength();
		float splineStartDistance = splineTotalDistance * SplineMeshes[i].StartOffset;
		float splineEndDistance = splineTotalDistance * SplineMeshes[i].EndOffset;

		// Calculate bounds
		FBoxSphereBounds meshBounds = SplineMeshes[i].MeshData.Mesh->GetBounds();
		// Scale mesh bounds with global scale
		meshBounds.BoxExtent = meshBounds.BoxExtent;

		// If the asset is larger than the current spline length, we will return without placing anything
		if (meshBounds.BoxExtent.X*2 > (splineEndDistance - splineStartDistance))
			continue;

		int steps = (splineEndDistance - splineStartDistance) / (meshBounds.BoxExtent.X*2);

		for (int j = 0; j < steps; j++)
		{
			float startDistanceAlongSpline = (j * meshBounds.BoxExtent.X*2) + splineStartDistance;
			float endDistanceAlongSpline = ((j+1) * meshBounds.BoxExtent.X*2) + splineStartDistance;

			// Calculate start and end locations and tangents from the given spline
			FVector startPosition = Spline->GetLocationAtDistanceAlongSpline(startDistanceAlongSpline, ESplineCoordinateSpace::World)+SplineMeshes[i].MeshData.Offset.GetLocation();
			FVector endPosition = Spline->GetLocationAtDistanceAlongSpline(endDistanceAlongSpline, ESplineCoordinateSpace::World)+SplineMeshes[i].MeshData.Offset.GetLocation();
			FVector startTangent = Spline->GetTangentAtDistanceAlongSpline(startDistanceAlongSpline, ESplineCoordinateSpace::Local).GetClampedToMaxSize(meshBounds.BoxExtent.X * 2);
			FVector endTangent = Spline->GetTangentAtDistanceAlongSpline(endDistanceAlongSpline, ESplineCoordinateSpace::Local).GetClampedToMaxSize(meshBounds.BoxExtent.X * 2);

			// Initialize the spline mesh component
			USplineMeshComponent* smc = NewObject<USplineMeshComponent>(this);	
			smc->SetStaticMesh(SplineMeshes[i].MeshData.Mesh);
			smc->SetStartAndEnd(startPosition, startTangent, endPosition, endTangent);
			smc->SetWorldScale3D(SplineMeshes[i].MeshData.Offset.GetScale3D());
			smc->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
			smc->RegisterComponent();
			SMCs.Add(smc);
		}
	}
}

bool AASplinePlacementActor::CalculateTransformsAtRegularDistances(float SplineLength, FMeshProfileInstance MeshProfile,
	TArray<FTransform> &OutTransforms)
{
	FBoxSphereBounds meshBounds = MeshProfile.MeshData.Mesh->GetBounds();

	// Scale mesh bounds with global scale
	meshBounds.BoxExtent = meshBounds.BoxExtent*MeshProfile.MeshData.Offset.GetScale3D();

	// If the asset is larger than the current spline length, we will return without placing anything
	if(meshBounds.BoxExtent.X*2 > SplineLength)
		return false;

	int steps = (SplineLength-MeshProfile.StartOffset) / (MeshProfile.Gap+meshBounds.BoxExtent.X*2);

	// Iterate with number of steps to get transform values for that many instances
	for(int i = 0; i <= steps; i++)
	{
		float distanceAlongSpline = i * (MeshProfile.Gap+meshBounds.BoxExtent.X*2) + MeshProfile.StartOffset;

		// Calculate location, rotation, and scale
		FVector location = Spline->GetLocationAtDistanceAlongSpline(distanceAlongSpline, ESplineCoordinateSpace::Local) + MeshProfile.MeshData.Offset.GetLocation();
		FRotator rotation =  Spline->GetRotationAtDistanceAlongSpline(distanceAlongSpline, ESplineCoordinateSpace::Local) + MeshProfile.MeshData.Offset.GetRotation().Rotator();
		FVector scale = Spline->GetScaleAtDistanceAlongSpline(distanceAlongSpline) * MeshProfile.MeshData.Offset.GetScale3D();

		OutTransforms.Add(FTransform(rotation, location, scale));
	}

	return true;
}

bool AASplinePlacementActor::CalculateTransformsAtSplinePoints(FMeshProfileInstance MeshProfile,
	TArray<FTransform>& OutTransforms)
{
	FBoxSphereBounds meshBounds = MeshProfile.MeshData.Mesh->GetBounds();

	// Scale mesh bounds with global scale
	meshBounds.BoxExtent = meshBounds.BoxExtent*MeshProfile.MeshData.Offset.GetScale3D();

	int numPoints = Spline->GetNumberOfSplinePoints();
	
	// Iterate with number of spline points to generate transforms at those locations
	for(int i = 0; i < numPoints; i++)
	{
		FVector location = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local) + MeshProfile.MeshData.Offset.GetLocation();
		FRotator rotation = Spline->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::Local) + MeshProfile.MeshData.Offset.GetRotation().Rotator();
		FVector scale = Spline->GetScaleAtSplinePoint(i) * MeshProfile.MeshData.Offset.GetScale3D();

		OutTransforms.Add(FTransform(rotation, location, scale));
	}

	return true;
}

// Called every frame
void AASplinePlacementActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AASplinePlacementActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.MemberProperty->GetName() == "InstancedMeshes")
	{
		// Discard and repopulate ISMs
		RepopulateISMs();
	}

	if (PropertyChangedEvent.MemberProperty->GetName() == "SplineMeshes")
	{
		// Discard and recalculate everything for spline meshes
		PlaceSplineMeshesLooped();
	}
	
	// Recalculate locations
	PlaceInstancesAlongSpline();
}

void AASplinePlacementActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	
	// Recalculate locations
	PlaceInstancesAlongSpline();

	// Place looped splines
	PlaceSplineMeshesLooped();
}

