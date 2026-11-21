#include "CrimeBloodlineGangSystem.h"
#include "CrimeBloodlineReputationTension.h"

ACrimeBloodlineGangSystem::ACrimeBloodlineGangSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    GangState.GangName = TEXT("عصابة الشمال");
    GangState.LoyaltyScore = 25.0f;
    GangState.BetrayalHeat = 0.0f;
    GangState.CurrentStanding = EGangStanding::LoyalAlly;
    GangState.bBetrayalClipReady = false;
    GangState.DramaHeadline = TEXT("الأوضاع مستقرة مع العصابة");
}

void ACrimeBloodlineGangSystem::ApplyGangAction(
    float LoyaltyChange,
    float BetrayalHeatChange,
    bool bIsBetrayalAct,
    const FString& CustomHeadline
)
{
    const float PreviousHeat = GangState.BetrayalHeat;
    const EGangStanding PreviousStanding = GangState.CurrentStanding;

    GangState.LoyaltyScore = FMath::Clamp(GangState.LoyaltyScore + LoyaltyChange, -100.0f, 100.0f);
    GangState.BetrayalHeat = FMath::Clamp(GangState.BetrayalHeat + BetrayalHeatChange, 0.0f, 100.0f);

    if (bIsBetrayalAct)
    {
        GangState.LoyaltyScore = FMath::Clamp(GangState.LoyaltyScore - 25.0f, -100.0f, 100.0f);
        GangState.BetrayalHeat = FMath::Clamp(GangState.BetrayalHeat + 30.0f, 0.0f, 100.0f);
    }

    // تحديد موقف العصابة بناءً على حرارة الغدر والولاء
    if (GangState.BetrayalHeat >= 75.0f || GangState.LoyaltyScore <= -50.0f)
    {
        GangState.CurrentStanding = EGangStanding::BetrayedOutlaw;
    }
    else if (GangState.BetrayalHeat >= 45.0f || GangState.LoyaltyScore <= -15.0f)
    {
        GangState.CurrentStanding = EGangStanding::MarkedForHit;
    }
    else if (GangState.BetrayalHeat >= 20.0f || GangState.LoyaltyScore < 15.0f)
    {
        GangState.CurrentStanding = EGangStanding::Cautious;
    }
    else
    {
        GangState.CurrentStanding = EGangStanding::LoyalAlly;
    }

    if (!CustomHeadline.IsEmpty())
    {
        GangState.DramaHeadline = CustomHeadline;
    }
    else if (GangState.CurrentStanding == EGangStanding::BetrayedOutlaw)
    {
        GangState.DramaHeadline = FString::Printf(TEXT("انقلاب كامل من %s — أوامر اشتباك فوري!"), *GangState.GangName);
    }
    else if (GangState.CurrentStanding == EGangStanding::MarkedForHit)
    {
        GangState.DramaHeadline = FString::Printf(TEXT("%s وضعتك على قائمة التصفية!"), *GangState.GangName);
    }
    else if (GangState.CurrentStanding == EGangStanding::Cautious)
    {
        GangState.DramaHeadline = FString::Printf(TEXT("عيون %s تراقب كل تحركاتك بحذر"), *GangState.GangName);
    }
    else
    {
        GangState.DramaHeadline = FString::Printf(TEXT("%s تقف في صفك وتدعم تحركاتك"), *GangState.GangName);
    }

    const bool bCrossedMajorThreshold =
        (PreviousStanding != GangState.CurrentStanding && GangState.CurrentStanding >= EGangStanding::MarkedForHit) ||
        (GangState.BetrayalHeat >= 75.0f && PreviousHeat < 75.0f);

    GangState.bBetrayalClipReady = bIsBetrayalAct || bCrossedMajorThreshold;

    // ربط أثر الحدث بالسمعة والتوتر في الشارع
    if (ReputationTensionActor)
    {
        const float RepDelta = bIsBetrayalAct ? -20.0f : FMath::Clamp(LoyaltyChange * 0.4f, -15.0f, 15.0f);
        const float TensionDelta = bIsBetrayalAct ? 35.0f : FMath::Clamp(BetrayalHeatChange * 0.6f, 0.0f, 30.0f);
        ReputationTensionActor->RegisterStreetEvent(RepDelta, TensionDelta, bIsBetrayalAct, GangState.DramaHeadline);
    }
}

void ACrimeBloodlineGangSystem::SyncWithReputationAndTension()
{
    if (!ReputationTensionActor)
    {
        return;
    }

    const FReputationAndTensionState& RepState = ReputationTensionActor->RepTensionState;

    // إذا كان توتر المنطقة عالياً جداً، تزداد حرارة الحذر والشك داخل العصابة
    if (RepState.AreaTension >= 75.0f)
    {
        GangState.BetrayalHeat = FMath::Clamp(GangState.BetrayalHeat + 5.0f, 0.0f, 100.0f);
    }

    // السمعة العالية في الشارع تدعم الولاء جزئياً ما لم يكن هناك غدر مباشر
    if (RepState.StreetReputation >= 50.0f && GangState.BetrayalHeat < 45.0f)
    {
        GangState.LoyaltyScore = FMath::Clamp(GangState.LoyaltyScore + 3.0f, -100.0f, 100.0f);
    }

    if (GangState.BetrayalHeat >= 75.0f || GangState.LoyaltyScore <= -50.0f)
    {
        GangState.CurrentStanding = EGangStanding::BetrayedOutlaw;
    }
    else if (GangState.BetrayalHeat >= 45.0f || GangState.LoyaltyScore <= -15.0f)
    {
        GangState.CurrentStanding = EGangStanding::MarkedForHit;
    }
    else if (GangState.BetrayalHeat >= 20.0f || GangState.LoyaltyScore < 15.0f)
    {
        GangState.CurrentStanding = EGangStanding::Cautious;
    }
    else
    {
        GangState.CurrentStanding = EGangStanding::LoyalAlly;
    }
}

void ACrimeBloodlineGangSystem::ConsumeBetrayalClipFlag()
{
    GangState.bBetrayalClipReady = false;
}
