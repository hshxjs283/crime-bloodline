#include "CrimeBloodlinePursuitAmbushSystem.h"
#include "CrimeBloodlineReputationTension.h"
#include "CrimeBloodlineGangSystem.h"

ACrimeBloodlinePursuitAmbushSystem::ACrimeBloodlinePursuitAmbushSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    PursuitState.PursuitHeat = 0.0f;
    PursuitState.EscalationLevel = EPursuitEscalationLevel::Clear;
    PursuitState.bGangAmbushActive = false;
    PursuitState.AmbushOrChaseHeadline = TEXT("لا توجد مطاردة أو كمين حالياً");
    PursuitState.bPursuitTrendClipReady = false;
}

void ACrimeBloodlinePursuitAmbushSystem::UpdatePursuitAndAmbush(float DeltaTime, bool bPlayerVisibleToLawOrGang)
{
    const float SafeDelta = FMath::Max(0.0f, DeltaTime);
    const EPursuitEscalationLevel PreviousLevel = PursuitState.EscalationLevel;
    const bool bPreviousAmbush = PursuitState.bGangAmbushActive;

    // 1. ربط توتر المنطقة بحرارة مطاردة الشرطة
    if (ReputationTensionActor)
    {
        const float AreaTension = ReputationTensionActor->RepTensionState.AreaTension;
        if (AreaTension >= 40.0f && bPlayerVisibleToLawOrGang)
        {
            const float TensionRiseRate = ((AreaTension - 35.0f) / 65.0f) * 14.0f;
            PursuitState.PursuitHeat = FMath::Clamp(
                PursuitState.PursuitHeat + (TensionRiseRate * SafeDelta),
                0.0f,
                100.0f
            );
        }
        else if (!bPlayerVisibleToLawOrGang)
        {
            // تهدئة حرارة المطاردة تدريجياً عندما يختفي اللاعب عن الأنظار
            PursuitState.PursuitHeat = FMath::Clamp(
                PursuitState.PursuitHeat - (4.5f * SafeDelta),
                0.0f,
                100.0f
            );
        }
    }

    // 2. فحص حالة العصابة وتفعيل الكمين إذا انقلبت أو وضعت اللاعب على قائمة التصفية
    bool bShouldAmbush = false;
    FString GangHeadline = TEXT("");

    if (GangSystemActor)
    {
        const EGangStanding Standing = GangSystemActor->GangState.CurrentStanding;
        if (Standing == EGangStanding::BetrayedOutlaw)
        {
            bShouldAmbush = true;
            PursuitState.PursuitHeat = FMath::Max(PursuitState.PursuitHeat, 70.0f);
            GangHeadline = FString::Printf(TEXT("كمين مسلح من %s في الطريق!"), *GangSystemActor->GangState.GangName);
        }
        else if (Standing == EGangStanding::MarkedForHit && bPlayerVisibleToLawOrGang)
        {
            bShouldAmbush = true;
            PursuitState.PursuitHeat = FMath::Max(PursuitState.PursuitHeat, 48.0f);
            GangHeadline = FString::Printf(TEXT("قناصة وملاحقون من %s رصدوا موقعك!"), *GangSystemActor->GangState.GangName);
        }
    }

    PursuitState.bGangAmbushActive = bShouldAmbush;

    // 3. تحديد مستوى تصعيد المطاردة
    if (PursuitState.PursuitHeat >= 75.0f || PursuitState.bGangAmbushActive)
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::AllOutLockdown;
    }
    else if (PursuitState.PursuitHeat >= 40.0f)
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::ActiveChase;
    }
    else if (PursuitState.PursuitHeat >= 15.0f)
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::PatrolSearch;
    }
    else
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::Clear;
    }

    // 4. صياغة العنوان الدرامي للموقف الحالي
    if (PursuitState.bGangAmbushActive)
    {
        PursuitState.AmbushOrChaseHeadline = GangHeadline;
    }
    else if (PursuitState.EscalationLevel == EPursuitEscalationLevel::AllOutLockdown)
    {
        PursuitState.AmbushOrChaseHeadline = TEXT("استنفار أمني شامل ومطاردة على كافة المحاور!");
    }
    else if (PursuitState.EscalationLevel == EPursuitEscalationLevel::ActiveChase)
    {
        PursuitState.AmbushOrChaseHeadline = TEXT("مطاردة نشطة — دوريات تطاردك خلفك!");
    }
    else if (PursuitState.EscalationLevel == EPursuitEscalationLevel::PatrolSearch)
    {
        PursuitState.AmbushOrChaseHeadline = TEXT("دوريات تفتيش تمشّط المنطقة بحثاً عنك");
    }
    else
    {
        PursuitState.AmbushOrChaseHeadline = TEXT("لا توجد مطاردة أو كمين حالياً");
    }

    // 5. تفعيل إشارة لقطة الترند عند التصعيد الكبير أو بداية الكمين
    const bool bEscalatedToHighAlert =
        (PreviousLevel != PursuitState.EscalationLevel && PursuitState.EscalationLevel >= EPursuitEscalationLevel::ActiveChase);
    const bool bAmbushJustStarted = (!bPreviousAmbush && PursuitState.bGangAmbushActive);

    if (bEscalatedToHighAlert || bAmbushJustStarted)
    {
        PursuitState.bPursuitTrendClipReady = true;
    }
}

void ACrimeBloodlinePursuitAmbushSystem::TriggerHeatSpike(float HeatAdded, bool bForceGangAmbush, const FString& CustomAlert)
{
    const bool bPreviousAmbush = PursuitState.bGangAmbushActive;
    const EPursuitEscalationLevel PreviousLevel = PursuitState.EscalationLevel;

    PursuitState.PursuitHeat = FMath::Clamp(PursuitState.PursuitHeat + HeatAdded, 0.0f, 100.0f);

    if (bForceGangAmbush)
    {
        PursuitState.bGangAmbushActive = true;
        PursuitState.PursuitHeat = FMath::Max(PursuitState.PursuitHeat, 75.0f);
    }

    if (PursuitState.PursuitHeat >= 75.0f || PursuitState.bGangAmbushActive)
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::AllOutLockdown;
    }
    else if (PursuitState.PursuitHeat >= 40.0f)
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::ActiveChase;
    }
    else if (PursuitState.PursuitHeat >= 15.0f)
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::PatrolSearch;
    }
    else
    {
        PursuitState.EscalationLevel = EPursuitEscalationLevel::Clear;
    }

    if (!CustomAlert.IsEmpty())
    {
        PursuitState.AmbushOrChaseHeadline = CustomAlert;
    }
    else if (PursuitState.bGangAmbushActive)
    {
        PursuitState.AmbushOrChaseHeadline = TEXT("كمين مفاجئ انطلق في قلب الطريق!");
    }
    else if (PursuitState.EscalationLevel >= EPursuitEscalationLevel::ActiveChase)
    {
        PursuitState.AmbushOrChaseHeadline = TEXT("اندلاع مطاردة سريعة ومفاجئة!");
    }

    if (PursuitState.bGangAmbushActive && !bPreviousAmbush)
    {
        PursuitState.bPursuitTrendClipReady = true;
    }
    else if (PursuitState.EscalationLevel > PreviousLevel && PursuitState.EscalationLevel >= EPursuitEscalationLevel::ActiveChase)
    {
        PursuitState.bPursuitTrendClipReady = true;
    }
}

void ACrimeBloodlinePursuitAmbushSystem::ConsumePursuitTrendFlag()
{
    PursuitState.bPursuitTrendClipReady = false;
}
