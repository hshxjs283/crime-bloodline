#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrimeBloodlineWeatherMudBridge.h"
#include "CrimeBloodlineRussiaWeather.h"
#include "CrimeBloodlineMasterCoordinator.generated.h"

class ACrimeBloodlineSurvivalVitals;
class ACrimeBloodlineRubAlKhaliWeather;
class ACrimeBloodlineRussiaWeather;
class ACrimeBloodlineWeatherMudBridge;
class ACrimeBloodlineReputationTension;

USTRUCT(BlueprintType)
struct FMasterWorldFrameSummary
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterLoop")
    EActiveWorldRegion CurrentRegion = EActiveWorldRegion::Russia;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterLoop")
    float FinalMovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterLoop")
    float FinalVehicleSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterLoop")
    float TotalEnvironmentalExposure = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterLoop")
    bool bTrendMomentReady = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MasterLoop")
    FString ActiveMomentTag = TEXT("لا يوجد حدث بارز حالياً");
};

UCLASS()
class ACrimeBloodlineMasterCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ACrimeBloodlineMasterCoordinator();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|Systems")
    TObjectPtr<ACrimeBloodlineSurvivalVitals> SurvivalVitalsActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|Systems")
    TObjectPtr<ACrimeBloodlineRubAlKhaliWeather> DesertWeatherActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|Systems")
    TObjectPtr<ACrimeBloodlineRussiaWeather> RussiaWeatherActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|Systems")
    TObjectPtr<ACrimeBloodlineWeatherMudBridge> WeatherMudBridgeActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|Systems")
    TObjectPtr<ACrimeBloodlineReputationTension> ReputationTensionActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|World")
    EActiveWorldRegion ActiveRegion = EActiveWorldRegion::Russia;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|World")
    bool bIsNightTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|World")
    bool bTriggerSandstorm = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TargetSandstormSeverity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|World")
    ERussiaWeatherType TargetRussiaWeather = ERussiaWeatherType::ClearCold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|World", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TargetRussiaWeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Master|Summary")
    FMasterWorldFrameSummary LatestSummary;

    // تحديث جميع الأنظمة المرتبطة في دورة واحدة منظمة
    UFUNCTION(BlueprintCallable, Category = "CrimeBloodline|Master")
    void TickMasterWorldSystems(
        float DeltaTime,
        bool bSprinting,
        bool bIsDirtOrSoilTerrain,
        bool bWearingFaceMaskOrWinterCoat,
        bool bWearingMudBoots,
        bool bInsideVehicleOrShelter,
        bool bHasAllWheelDrive,
        bool bHasOffroadTires,
        float CurrentVehicleSpeed
    );
};
