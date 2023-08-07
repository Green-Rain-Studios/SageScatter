// 2023 Green Rain Studios

#pragma once

#include "CoreMinimal.h"
#include "SplinePlacementActor.h"
#include "RoadSplineActor.generated.h"

class UPointLightComponent;

USTRUCT(BlueprintType)
struct FPointLightProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light", meta=(ClampMin = 0, Units="Candela"))
	float Intensity = 8.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light")
	FLinearColor LightColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light", meta=(UIMin = 8))
	float AttenuationRadius = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light", meta=(ClampMin = 0))
	float SourceRadius = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light")
	bool bUseTemperature;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light", meta=(ClampMin = 1700, ClampMax = 12000, EditCondition=bUseTemperature))
	float Temperature = 6500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light")
	bool AffectsWorld = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light")
	bool CastShadows = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light|Setup")
	FVector Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light|Setup")
	EInstancePlacementType PlacementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light|Setup", meta=(ClampMin=1, EditCondition="PlacementType==EInstancePlacementType::IPT_Gap", EditConditionHides))
	float Gap = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Light|Setup", meta=(EditCondition="PlacementType==EInstancePlacementType::IPT_Gap", EditConditionHides))
	float StartOffset = 0.f;
	
};
UCLASS()
class SAGESCATTER_API ARoadSplineActor : public ASplinePlacementActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARoadSplineActor();
	
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

	// Create required number of point lights based on length of spline and gap
	void CreatePLCs();

	// Update Existing Point Lights
	void UpdatePLCs();

	// Update properties of a single light from profile
	void UpdatePointLightPropertiesFromProfile(const FPointLightProfile& LightProfile, UPointLightComponent* PointLight);
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setup|Lights", meta=(ShowOnlyInnerProperties))
	TArray<FPointLightProfile> PointLights;
	
protected:
	UPROPERTY(VisibleInstanceOnly)
	TArray<UPointLightComponent*> PLCs;
};
