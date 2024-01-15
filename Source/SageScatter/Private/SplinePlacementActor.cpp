// 2023 Green Rain Studios


#include "SplinePlacementActor.h"

#include "Components/BillboardComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/LocalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SplineComponent.h"
#include "SageScatterUtils.h"


// Sets default values
ASplinePlacementActor::ASplinePlacementActor()
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
	Icn->ScreenSize = BILLBOARD_SPRITE_SIZE;
	Icn->SetHiddenInGame(true);
	Icn->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASplinePlacementActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASplinePlacementActor::RepopulateISMs()
{
	// Delete previous ISMs
	GetComponents<UHierarchicalInstancedStaticMeshComponent>(ISMs);
	for (int i = 0; i < ISMs.Num(); i++)
	{
		ISMs[i]->UnregisterComponent();
		ISMs[i]->DestroyComponent();
	}
	
	// Clear ISM array and create a new one
	ISMs.Empty();

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

void ASplinePlacementActor::PlaceInstancesAlongSpline()
{
	// First we clear all ISMs of their instances
	for(int i = 0; i < ISMs.Num(); i++)
	{
		ISMs[i]->ClearInstances();
	}

	// Then we populate based on total length of spline
	const float splineLength = Spline->GetSplineLength();

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
		CreatePLCs(i);
		UpdatePLCs(i);
	}
}

bool ASplinePlacementActor::CalculateTransformsAtRegularDistances(float SplineLength, FMeshProfileInstance MeshProfile,
	TArray<FTransform> &OutTransforms)
{
	FBoxSphereBounds meshBounds = MeshProfile.MeshData.Mesh->GetBounds();

	// Scale mesh bounds with global scale
	meshBounds.BoxExtent = meshBounds.BoxExtent*MeshProfile.MeshData.Offset.GetScale3D();

	// If the asset is larger than the current spline length, we will return without placing anything
	if(meshBounds.BoxExtent.X*2 > SplineLength)
		return false;

	const int steps = (SplineLength-MeshProfile.StartOffset) / FMath::Min<float>(MeshProfile.Gap+meshBounds.BoxExtent.X*2, SplineLength-MeshProfile.StartOffset);

	// Iterate with number of steps to get transform values for that many instances
	for(int i = 0; i <= steps; i++)
	{
		float dist = i * (MeshProfile.Gap + meshBounds.BoxExtent.X * 2) + MeshProfile.StartOffset;
		FTransform transform = GetTransformAtDistanceAlongSpline(dist);

		// Cache fwd, up, right vectors
		FVector fwd, right, up;
		GetDirectionVectorsAtDistanceAlongSpline(dist, fwd, right, up);
		
		// Calculate location, rotation, and scale
		FVector location = transform.GetLocation() + USageScatterUtils::CalculateOffsets(MeshProfile.MeshData.Offset.GetLocation(), fwd, right, up);
		FRotator rotation =  transform.GetRotation().Rotator() + MeshProfile.MeshData.Offset.GetRotation().Rotator();
		FVector scale = transform.GetScale3D() * MeshProfile.MeshData.Offset.GetScale3D();

		OutTransforms.Add(FTransform(rotation, location, scale));
	}

	return true;
}

bool ASplinePlacementActor::CalculateTransformsAtSplinePoints(FMeshProfileInstance MeshProfile,
	TArray<FTransform>& OutTransforms)
{
	FBoxSphereBounds meshBounds = MeshProfile.MeshData.Mesh->GetBounds();

	// Scale mesh bounds with global scale
	meshBounds.BoxExtent = meshBounds.BoxExtent*MeshProfile.MeshData.Offset.GetScale3D();

	int numPoints = Spline->GetNumberOfSplinePoints();
	
	// Iterate with number of spline points to generate transforms at those locations
	for(int i = 0; i < numPoints; i++)
	{
		float dist = Spline->GetDistanceAlongSplineAtSplinePoint(i);
		FTransform transform = GetTransformAtDistanceAlongSpline(dist);

		// Cache fwd, up, right vectors
		FVector fwd, right, up;
		GetDirectionVectorsAtDistanceAlongSpline(dist, fwd, right, up);
		
		FVector location = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local) + USageScatterUtils::CalculateOffsets(MeshProfile.MeshData.Offset.GetLocation(), fwd, right, up);
		FRotator rotation = Spline->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::Local) + MeshProfile.MeshData.Offset.GetRotation().Rotator();
		FVector scale = Spline->GetScaleAtSplinePoint(i) * MeshProfile.MeshData.Offset.GetScale3D();

		OutTransforms.Add(FTransform(rotation, location, scale));
	}

	return true;
}

void ASplinePlacementActor::RecalculateSplineMeshes()
{
	// First we calculate total number of spline meshes needed with current spline length
	int requiredSMCs = 0;

	for(int i = 0; i < SplineMeshes.Num(); i++)
	{
		// If the mesh is not set, skip this profile
		if(SplineMeshes[i].MeshData.Mesh == nullptr)
			continue;

		// If mesh is single, increment required by 1. Else we calculate using steps
		if(SplineMeshes[i].PlacementType == ESplinePlacementType::SPT_SINGLE)
		{
			requiredSMCs++;
		}
		else if(SplineMeshes[i].PlacementType == ESplinePlacementType::SPT_LOOPED)
		{
			// Get extents of total mesh and calculate number of steps required to place mesh along spline
			// Subtract end and start distance from it
			const float finalSplineLength = Spline->GetSplineLength() * SplineMeshes[i].EndOffset - Spline->GetSplineLength() * SplineMeshes[i].StartOffset;

			const FVector extent = SplineMeshes[i].MeshData.Mesh->GetBounds().BoxExtent * SplineMeshes[i].MeshData.Offset.GetScale3D();

			const float singleStep = FMath::Min<float>(extent.X * 2 * SplineMeshes[i].RelaxMultiplier, finalSplineLength);
			const int steps = finalSplineLength / singleStep;

			// Number of steps = number of SMCs needed
			requiredSMCs += steps;
		}
	}

	GetComponents<USplineMeshComponent>(SMCs);
	// If there are less Spline mesh components than needed, we need to create more. If there are more, we need to destroy
	if(SMCs.Num() < requiredSMCs)
	{
		for(int i = SMCs.Num(); i < requiredSMCs; i++)
		{
			USplineMeshComponent* smc = NewObject<USplineMeshComponent>(this);
			smc->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
			smc->RegisterComponent();
			SMCs.Add(smc);
		}
	}
	else if(SMCs.Num() > requiredSMCs)
	{
		// Destroy the excess components
		for(int i = requiredSMCs; i < SMCs.Num(); i++)
		{
			SMCs[i]->UnregisterComponent();
			SMCs[i]->DestroyComponent();
		}

		// Remove the destroyed components from the array
		SMCs.RemoveAt(requiredSMCs, SMCs.Num() - requiredSMCs);
	}
}

void ASplinePlacementActor::PlaceSplineMeshComponentsAlongSpline()
{
	int currentIdx = 0;
	// Place SMCs based on mesh data
	for (FMeshProfileSpline splineMeshProfile : SplineMeshes)
	{
		// If the mesh is not set, skip this profile
		if(splineMeshProfile.MeshData.Mesh == nullptr)
			continue;
		
		// Get extents of total mesh and calculate number of steps required to place mesh along spline
		// Subtract end and start distance from it
		const float rawSplineLength = Spline->GetSplineLength();
		const float finalSplineLength = rawSplineLength * splineMeshProfile.EndOffset - rawSplineLength * splineMeshProfile.StartOffset;

		const FVector extent = splineMeshProfile.MeshData.Mesh->GetBounds().BoxExtent * splineMeshProfile.MeshData.Offset.GetScale3D();

		// Use the steps to find transforms and place SMCs
		if(splineMeshProfile.PlacementType == ESplinePlacementType::SPT_LOOPED)
		{
			// Single step should be the extent of the mesh, or the length of the spline if that is smaller
			const float singleStep = FMath::Min<float>(extent.X * 2 * splineMeshProfile.RelaxMultiplier, finalSplineLength);
			const int steps = finalSplineLength / singleStep;
			
			for(int i = 0; i < steps; i++)
			{
				const float startDist = i * singleStep + rawSplineLength * splineMeshProfile.StartOffset;
				const float endDist = (i + 1) * singleStep + rawSplineLength * splineMeshProfile.StartOffset;
				
				// For spline meshes, there is a start and end transform
				FTransform startTransform = Spline->GetTransformAtDistanceAlongSpline(startDist, ESplineCoordinateSpace::Local, true);
				FTransform endTransform = Spline->GetTransformAtDistanceAlongSpline(endDist, ESplineCoordinateSpace::Local, true);
				
				// Cache fwd, up, right vectors
				FVector startFwd, startRight, startUp;
				GetDirectionVectorsAtDistanceAlongSpline(startDist, startFwd, startRight, startUp);
				FVector endFwd, endRight, endUp;
				GetDirectionVectorsAtDistanceAlongSpline(endDist, endFwd, endRight, endUp);

				// Calculate locations
				FVector startLocation = GetActorLocation() + startTransform.GetLocation() + USageScatterUtils::CalculateOffsets(splineMeshProfile.MeshData.Offset.GetLocation(), startFwd, startRight, startUp);
				FVector startTangent = Spline->GetTangentAtDistanceAlongSpline(startDist, ESplineCoordinateSpace::Local).GetClampedToMaxSize(singleStep);
				FVector endLocation = GetActorLocation() + endTransform.GetLocation() + USageScatterUtils::CalculateOffsets(splineMeshProfile.MeshData.Offset.GetLocation(), endFwd, endRight, endUp);
				FVector endTangent = Spline->GetTangentAtDistanceAlongSpline(endDist, ESplineCoordinateSpace::Local).GetClampedToMaxSize(singleStep);
				
				// Assign mesh and use SMC
				SMCs[currentIdx]->SetStaticMesh(splineMeshProfile.MeshData.Mesh);
				SMCs[currentIdx]->SetStartAndEnd(startLocation, startTangent, endLocation, endTangent);

				// Increment index to use (since we are fitting a dynamic 2d array into a 1d array)
				currentIdx++;
			}
		}
		else if(splineMeshProfile.PlacementType == ESplinePlacementType::SPT_SINGLE)
		{
			const float startDist = splineMeshProfile.StartDistance;
			const float endDist = startDist + splineMeshProfile.MeshLength;

			// Cache fwd, up, right vectors
			FVector startFwd, startRight, startUp;
			GetDirectionVectorsAtDistanceAlongSpline(startDist, startFwd, startRight, startUp);
			FVector endFwd, endRight, endUp;
			GetDirectionVectorsAtDistanceAlongSpline(endDist, endFwd, endRight, endUp);

			// Calculate locations
			FVector startLocation = GetActorLocation() + Spline->GetLocationAtDistanceAlongSpline(startDist, ESplineCoordinateSpace::Local) + USageScatterUtils::CalculateOffsets(splineMeshProfile.MeshData.Offset.GetLocation(), startFwd, startRight, startUp);
			FVector startTangent = Spline->GetTangentAtDistanceAlongSpline(startDist, ESplineCoordinateSpace::Local).GetClampedToMaxSize(splineMeshProfile.MeshLength);
			FVector endLocation = GetActorLocation() + Spline->GetLocationAtDistanceAlongSpline(endDist, ESplineCoordinateSpace::Local) + USageScatterUtils::CalculateOffsets(splineMeshProfile.MeshData.Offset.GetLocation(), endFwd, endRight, endUp);
			FVector endTangent = Spline->GetTangentAtDistanceAlongSpline(endDist, ESplineCoordinateSpace::Local).GetClampedToMaxSize(splineMeshProfile.MeshLength);

			// Assign mesh and use SMC
			SMCs[currentIdx]->SetStaticMesh(splineMeshProfile.MeshData.Mesh);
			SMCs[currentIdx]->SetStartAndEnd(startLocation, startTangent, endLocation, endTangent);
			
			// Increment index to use (since we are fitting a dynamic 2d array into a 1d array)
			currentIdx++;
		}
	}
}

void ASplinePlacementActor::CreatePLCs(const int idx)
{
	// If the light doesnt need to be added, we skip it
	if(!InstancedMeshes[idx].bActivateLight || bForceUnloadLights)
	{
		for(ULocalLightComponent* ll : InstancedMeshes[idx].PLCs)
		{
			ll->UnregisterComponent();
			ll->DestroyComponent();
		}
		
		// Clear array
		InstancedMeshes[idx].PLCs.Empty();

		if(!InstancedMeshes[idx].bActivateLight)
			return;
	}

	bForceUnloadLights = false;
	
	if(InstancedMeshes[idx].PLCs.Num() != ISMs[idx]->GetInstanceCount())
	{
		if(InstancedMeshes[idx].PLCs.Num() > ISMs[idx]->GetInstanceCount())
		{
			for(int i = InstancedMeshes[idx].PLCs.Num()-1; i >= ISMs[idx]->GetInstanceCount(); i--)
			{
				ULocalLightComponent* ll = InstancedMeshes[idx].PLCs.Pop();
				ll->UnregisterComponent();
				ll->DestroyComponent();
			}
		}
		else if(InstancedMeshes[idx].PLCs.Num() < ISMs[idx]->GetInstanceCount())
		{
			for(int i = InstancedMeshes[idx].PLCs.Num(); i < ISMs[idx]->GetInstanceCount(); i++)
			{
				if(InstancedMeshes[idx].LightData.Type == ELightType::LT_POINT)
				{
					UPointLightComponent* plc = NewObject<UPointLightComponent>(this);
					plc->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
					plc->RegisterComponent();
					plc->SetIntensityUnits(ELightUnits::Candelas);
					InstancedMeshes[idx].PLCs.Add(plc);
				}
				else
				{
					USpotLightComponent* slc = NewObject<USpotLightComponent>(this);
					slc->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
					slc->RegisterComponent();
					slc->SetIntensityUnits(ELightUnits::Candelas);
					InstancedMeshes[idx].PLCs.Add(slc);
				}
			}
		}
	}
}

void ASplinePlacementActor::UpdatePLCs(const int idx)
{
	// If the light doesnt need to be added, we skip it
	if(!InstancedMeshes[idx].bActivateLight)
		return;
	
	for(int i = 0; i < ISMs[idx]->GetInstanceCount(); i++)
	{
		FTransform transform;
		ISMs[idx]->GetInstanceTransform(i, transform);
		transform.SetLocation(transform.GetLocation() + USageScatterUtils::CalculateOffsets(InstancedMeshes[idx].LightData.LocationOffset, transform.GetUnitAxis(EAxis::X), transform.GetUnitAxis(EAxis::Y), transform.GetUnitAxis(EAxis::Z)));
		transform.SetRotation((USageScatterUtils::MakeRotatorFromAxes(transform.GetUnitAxis(EAxis::X), transform.GetUnitAxis(EAxis::Y), transform.GetUnitAxis(EAxis::Z)) + InstancedMeshes[idx].LightData.RotationOffset).Quaternion());
		
		// Set data on point light component
		InstancedMeshes[idx].PLCs[i]->SetRelativeTransform(transform);
		UpdateLightPropertiesFromProfile(InstancedMeshes[idx].LightData, InstancedMeshes[idx].PLCs[i]);
	}
}

FTransform ASplinePlacementActor::GetTransformAtDistanceAlongSpline(float Distance)
{
	return Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local, true);
}

void ASplinePlacementActor::GetDirectionVectorsAtDistanceAlongSpline(float Distance, FVector& Fwd, FVector& Right, FVector& Up)
{
	Fwd = Spline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
	Right = Spline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
	Up = Spline->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
}

void ASplinePlacementActor::UpdateLightPropertiesFromProfile(const FPointLightProfile& LightProfile,
	ULocalLightComponent* Light)
{
	if(Light == nullptr)
		return;
	
	// Set all properties of the light from the light profile
	Light->Intensity = LightProfile.Intensity;
	Light->SetLightColor(LightProfile.LightColor);
	Light->AttenuationRadius = LightProfile.AttenuationRadius;

	if(LightProfile.Type == ELightType::LT_POINT)
	{
		Cast<UPointLightComponent>(Light)->SourceRadius = LightProfile.SourceRadius;
	}
	else if(LightProfile.Type == ELightType::LT_SPOT)
	{
		USpotLightComponent* slc = Cast<USpotLightComponent>(Light);
		slc->SourceRadius = LightProfile.SourceRadius;
		slc->InnerConeAngle = LightProfile.InnerConeAngle;
		slc->OuterConeAngle = LightProfile.OuterConeAngle;
	}
	
	Light->bUseTemperature = LightProfile.bUseTemperature;    
	if(LightProfile.bUseTemperature)
		Light->Temperature = LightProfile.Temperature;

	Light->bAffectsWorld = LightProfile.AffectsWorld;
	Light->CastShadows = LightProfile.CastShadows;
}

// Called every frame
void ASplinePlacementActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASplinePlacementActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	GEngine->AddOnScreenDebugMessage(1, 2, FColor::Emerald, PropertyChangedEvent.GetPropertyName().ToString());

	if(PropertyChangedEvent.MemberProperty->GetName() == "InstancedMeshes")
	{
		// Discard and repopulate ISMs
		RepopulateISMs();

		if(PropertyChangedEvent.Property->GetName() == "Type")
		{
			bForceUnloadLights = true;
		}
	}

	// Making this as granular as possible for max performance
	if (PropertyChangedEvent.MemberProperty->GetName() == "SplineMeshes")
	{
		RecalculateSplineMeshes();
		PlaceSplineMeshComponentsAlongSpline();
	}
	
	// Recalculate locations
	PlaceInstancesAlongSpline();
}

void ASplinePlacementActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	
	// Recalculate locations
	PlaceInstancesAlongSpline();

	// Place splines
	RecalculateSplineMeshes();
	PlaceSplineMeshComponentsAlongSpline();
}

void ASplinePlacementActor::PostEditUndo()
{
	Super::PostEditUndo();

	// Recalculate locations
	PlaceInstancesAlongSpline();

	// Place splines
	RecalculateSplineMeshes();
	PlaceSplineMeshComponentsAlongSpline();
}

void ASplinePlacementActor::PostEditImport()
{
	Super::PostEditImport();

	RepopulateISMs();
	// Recalculate locations
	PlaceInstancesAlongSpline();

	// Place splines
	RecalculateSplineMeshes();
	PlaceSplineMeshComponentsAlongSpline();
}

