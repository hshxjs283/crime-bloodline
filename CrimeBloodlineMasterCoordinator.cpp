#include "CrimeBloodlineMasterCoordinator.h"
#include "CrimeBloodlineSurvivalVitals.h"
#include "CrimeBloodlineRubAlKhaliWeather.h"
#include "CrimeBloodlineRussiaWeather.h"
#include "CrimeBloodlineWeatherMudBridge.h"
#include "CrimeBloodlineMudSystem.h"
#include "CrimeBloodlineReputationTension.h"

ACrimeBloodlineMasterCoordinator::ACrimeBloodlineMasterCoordinator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACrimeBloodlineMasterCoordinator::TickMasterWorldSystems(
    float DeltaTime,
    bool bSprinting,
    bool bIsDirtOrSoilTerrain,
    bool bWearingFaceMaskOrWinterCoat,
    bool bWearingMudBoots,
    bool bInsideVehicleOrShelter,
    bool bHasAllWheelDrive,
    bool bHasOffroadTires,
    float CurrentVehicleSpeed
)
{
    const float SafeDelta = FMath::Max(0.0f, DeltaTime);

    LatestSummary.CurrentRegion = ActiveRegion;
    LatestSummary.FinalMovementSpeedMultiplier = 1.0f;
    LatestSummary.FinalVehicleSpeedMultiplier = 1.0f;
    LatestSummary.TotalEnvironmentalExposure = 0.0f;

    float ActiveTempCelsius = 20.0f;
    float WeatherMoveMult = 1.0f;
    float WeatherVehicleMult = 1.0f;
    bool bHighHeatRegion = false;

    // 1. تحديث طقس المنطقة النشطة (الربع الخالي أو روسيا)
    if (ActiveRegion == EActiveWorldRegion::RubAlKhali)
    {
        bHighHeatRegion = !bIsNightTime;
        if (DesertWeatherActor)
        {
            DesertWeatherActor->UpdateDesertWeather(SafeDelta, bIsNightTime, bTriggerSandstorm, TargetSandstormSeverity);
            ActiveTempCelsius = DesertWeatherActor->DesertWeatherState.DesertTemperatureCelsius;
            WeatherMoveMult = DesertWeatherActor->DesertWeatherState.MovementSpeedMultiplier;
            WeatherVehicleMult = DesertWeatherActor->DesertWeatherState.VehicleSpeedMultiplier;
            LatestSummary.TotalEnvironmentalExposure += DesertWeatherActor->EvaluateDustAndHeatImpact(
                bWearingFaceMaskOrWinterCoat,
                bInsideVehicleOrShelter
            );
        }
    }
    else
    {
        if (RussiaWeatherActor)
        {
            RussiaWeatherActor->UpdateRussiaWeather(SafeDelta, TargetRussiaWeather, TargetRussiaWeatherIntensity, bIsNightTime);
            ActiveTempCelsius = RussiaWeatherActor->RussiaWeatherState.RussiaTemperatureCelsius;
            WeatherMoveMult = RussiaWeatherActor->RussiaWeatherState.MovementSpeedMultiplier;
            WeatherVehicleMult = RussiaWeatherActor->RussiaWeatherState.VehicleGripMultiplier;
            LatestSummary.TotalEnvironmentalExposure += RussiaWeatherActor->EvaluateColdExposure(
                bWearingFaceMaskOrWinterCoat,
                bInsideVehicleOrShelter
            );
        }
    }

    // 2. مزامنة الطقس مع نظام الطين وتحديث سرعة المشي والسيارة
    float MudMoveMult = 1.0f;
    float MudVehicleMult = 1.0f;
    if (WeatherMudBridgeActor)
    {
        WeatherMudBridgeActor->ActiveRegion = ActiveRegion;
        WeatherMudBridgeActor->SyncWeatherToMud(
            SafeDelta,
            bIsDirtOrSoilTerrain,
            bWearingMudBoots,
            bHasAllWheelDrive,
            bHasOffroadTires,
            CurrentVehicleSpeed
        );

        if (WeatherMudBridgeActor->MudSystemActor)
        {
            const FMudPhysicsState& MudState = WeatherMudBridgeActor->MudSystemActor->MudState;
            MudMoveMult = MudState.PlayerMovementMultiplier;
            MudVehicleMult = MudState.VehicleSpeedMultiplier;
        }
    }

    // 3. تحديث الحالة الجسدية والبقاء وحساب سرعة اللاعب النهائية
    float VitalsMoveMult = 1.0f;
    if (SurvivalVitalsActor)
    {
        SurvivalVitalsActor->TickSurvivalVitals(SafeDelta, ActiveTempCelsius, bSprinting, bHighHeatRegion);
        VitalsMoveMult = SurvivalVitalsActor->VitalsState.MovementSpeedMultiplier;
    }

    LatestSummary.FinalMovementSpeedMultiplier = FMath::Clamp(
        VitalsMoveMult * WeatherMoveMult * MudMoveMult,
        0.25f,
        1.35f
    );

    LatestSummary.FinalVehicleSpeedMultiplier = FMath::Clamp(
        WeatherVehicleMult * MudVehicleMult,
        0.20f,
        1.30f
    );

    // 4. تهدئة توتر المنطقة تدريجياً وقراءة حالة الترند
    if (ReputationTensionActor)
    {
        ReputationTensionActor->DecayAreaTension(SafeDelta, 1.5f);
        LatestSummary.bTrendMomentReady = ReputationTensionActor->RepTensionState.bTrendWorthyMomentTriggered;
        LatestSummary.ActiveMomentTag = ReputationTensionActor->RepTensionState.LastMomentTag;
    }
    else
    {
        LatestSummary.bTrendMomentReady = false;
        LatestSummary.ActiveMomentTag = TEXT("لا يوجد حدث بارز حالياً");
    }
}
