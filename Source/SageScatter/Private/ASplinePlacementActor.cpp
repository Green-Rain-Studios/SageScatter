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
	for(int i = 0; i < Meshes.Num(); i++)
	{
		// Check if mesh is null first
		if(Meshes[i].MeshData.Mesh == nullptr)
			continue;
		
		UHierarchicalInstancedStaticMeshComponent* ism = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		ism->SetStaticMesh(Meshes[i].MeshData.Mesh);
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
		// Error checking. If mesh does not exist, or mesh is bigger than spline length, then skip this one
		if(Meshes[i].MeshData.Mesh == nullptr)
			continue;
		FMeshProfileSpline meshProfile = Meshes[i];
		FBoxSphereBounds meshBounds = meshProfile.MeshData.Mesh->GetBounds();
		// Scale mesh bounds with global scale
		meshBounds.BoxExtent = meshBounds.BoxExtent*meshProfile.MeshData.Offset.GetScale3D();
		if(meshBounds.BoxExtent.X*2 > splineLength)
			continue;

		int steps = splineLength / (meshProfile.Gap+meshBounds.BoxExtent.X*2);

		// Iterate over steps and create array of transforms to add to the ISM
		UHierarchicalInstancedStaticMeshComponent* ism = ISMs[i];
		TArray<FTransform> transforms;
		for(int j = 0; j <= steps; j++)
		{
			float distanceAlongSpline = j * (meshProfile.Gap+meshBounds.BoxExtent.X*2) + meshProfile.StartOffset;

			// Calculate location, rotation, and scale
			FVector location = Spline->GetLocationAtDistanceAlongSpline(distanceAlongSpline, ESplineCoordinateSpace::Local) + meshProfile.MeshData.Offset.GetLocation();
			FRotator rotation =  Spline->GetRotationAtDistanceAlongSpline(distanceAlongSpline, ESplineCoordinateSpace::Local) + meshProfile.MeshData.Offset.GetRotation().Rotator();
			FVector scale = Spline->GetScaleAtDistanceAlongSpline(distanceAlongSpline) * meshProfile.MeshData.Offset.GetScale3D();

			transforms.Add(FTransform(rotation, location, scale));
		}
		// Add instances to ISM
		ism->AddInstances(transforms,false);
	}
}

// Called every frame
void AASplinePlacementActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AASplinePlacementActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.MemberProperty->GetName() == "Meshes")
	{
		// Discard and repopulate ISMs
		RepopulateISMs();	
	}
	
	// Recalculate locations
	PlaceInstancesAlongSpline();
}

void AASplinePlacementActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	
	// Recalculate locations
	PlaceInstancesAlongSpline();
}

