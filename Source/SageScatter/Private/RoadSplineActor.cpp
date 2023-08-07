// 2023 Green Rain Studios


#include "RoadSplineActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SplineComponent.h"
#include "SageScatterUtils.h"


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

void ARoadSplineActor::CreatePLCs()
{
	// First we calculate total number of Point lights needed
	int requiredPLCs = 0;

	for(int i = 0; i < PointLights.Num(); i++)
	{
		// If mesh is single, increment required by 1. Else we calculate using steps
		if(PointLights[i].PlacementType == EInstancePlacementType::IPT_POINT)
		{
			requiredPLCs+= Spline->GetNumberOfSplinePoints();
		}
		else if(PointLights[i].PlacementType == EInstancePlacementType::IPT_GAP)
		{
			// Get extents of total mesh and calculate number of steps required to place mesh along spline
			// Subtract end and start distance from it
			const float finalSplineLength = Spline->GetSplineLength() - PointLights[i].StartOffset;
			PointLights[i].Gap = PointLights[i].Gap <= 0 ? 1.f : PointLights[i].Gap;
			const int steps = finalSplineLength / FMath::Min<float>(PointLights[i].Gap, finalSplineLength);

			// Number of steps = number of SMCs needed
			requiredPLCs += steps;
		}
	}

	GetComponents<UPointLightComponent>(PLCs);
	// If there are less Spline mesh components than needed, we need to create more. If there are more, we need to destroy
	if(PLCs.Num() < requiredPLCs)
	{
		for(int i = PLCs.Num(); i < requiredPLCs; i++)
		{
			UPointLightComponent* plc = NewObject<UPointLightComponent>(this);
			plc->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
			plc->RegisterComponent();
			plc->SetIntensityUnits(ELightUnits::Candelas);
			PLCs.Add(plc);
		}
	}
	else if(PLCs.Num() > requiredPLCs)
	{
		// Destroy the excess components
		for(int i = requiredPLCs; i < PLCs.Num(); i++)
		{
			PLCs[i]->UnregisterComponent();
			PLCs[i]->DestroyComponent();
		}

		// Remove the destroyed components from the array
		PLCs.RemoveAt(requiredPLCs, PLCs.Num() - requiredPLCs);
	}
}

void ARoadSplineActor::UpdatePLCs()
{
	int currentPointLightIdx = 0;
	// Iterate over all the light profiles in the actor
	for(FPointLightProfile LightProfile : PointLights)
	{
		// Set locations for point lights
		if(LightProfile.PlacementType == EInstancePlacementType::IPT_POINT)
		{
			for(int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
			{
				const float dist = Spline->GetDistanceAlongSplineAtSplinePoint(i);
				// Cache fwd, up, right vectors
				FVector fwd, right, up;
				GetDirectionVectorsAtDistanceAlongSpline(dist, fwd, right, up);

				// Location at resultant offset + spline location
				const FVector location = Spline->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::Local) + USageScatterUtils::CalculateOffsets(LightProfile.Offset, fwd, right, up);

				// Set data on point light component
				PLCs[currentPointLightIdx]->SetRelativeLocation(location);
				UpdatePointLightPropertiesFromProfile(LightProfile, PLCs[currentPointLightIdx]);

				// Increment index to use (since we are fitting a dynamic 2d array into a 1d array)
				currentPointLightIdx++;
			}
		}
		else if(LightProfile.PlacementType == EInstancePlacementType::IPT_GAP)
		{
			// Divide the total spline into equidistant steps based on the gap between light profiles
			const float finalSplineLength = Spline->GetSplineLength() - LightProfile.StartOffset;
			const int steps = finalSplineLength/FMath::Min<float>(LightProfile.Gap, finalSplineLength);

			for(int i = 0; i < steps; i++)
			{
				const float dist = i * LightProfile.Gap + LightProfile.StartOffset;
				// Cache fwd, up, right vectors
				FVector fwd, right, up;
				GetDirectionVectorsAtDistanceAlongSpline(dist, fwd, right, up);

				// Location at resultant offset + spline location
				const FVector location = Spline->GetLocationAtDistanceAlongSpline(dist, ESplineCoordinateSpace::Local) + USageScatterUtils::CalculateOffsets(LightProfile.Offset, fwd, right, up);

				// Set data on point light component
				PLCs[currentPointLightIdx]->SetRelativeLocation(location);
				UpdatePointLightPropertiesFromProfile(LightProfile, PLCs[currentPointLightIdx]);

				// Increment index to use (since we are fitting a dynamic 2d array into a 1d array)
				currentPointLightIdx++;
			}
		}
	}
}

void ARoadSplineActor::UpdatePointLightPropertiesFromProfile(const FPointLightProfile& LightProfile,
	UPointLightComponent* PointLight)
{
	if(PointLight == nullptr)
		return;
	
	// Set all properties of the light from the light profile
	PointLight->SetLightBrightness(LightProfile.Intensity);
	PointLight->SetLightColor(LightProfile.LightColor);
	PointLight->SetAttenuationRadius(LightProfile.AttenuationRadius);
	PointLight->SetSourceRadius(LightProfile.SourceRadius);
	
	PointLight->bUseTemperature = LightProfile.bUseTemperature;    
	if(LightProfile.bUseTemperature)
		PointLight->SetTemperature(LightProfile.Temperature);

	PointLight->bAffectsWorld = LightProfile.AffectsWorld;
	PointLight->SetCastShadows(LightProfile.CastShadows);
}

// Called every frame
void ARoadSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARoadSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.GetPropertyName() == "PointLights") 
	{
		CreatePLCs();
	}

	if(PropertyChangedEvent.MemberProperty->GetName() == "PointLights" && (PropertyChangedEvent.GetPropertyName() == "PlacementType" ||
		PropertyChangedEvent.GetPropertyName() == "Gap" || PropertyChangedEvent.GetPropertyName() == "StartOffset"))
	{
		CreatePLCs();
	}

	UpdatePLCs();
}

void ARoadSplineActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	CreatePLCs();
	UpdatePLCs();
}

void ARoadSplineActor::PostEditUndo()
{
	Super::PostEditUndo();

	CreatePLCs();
	UpdatePLCs();
}

