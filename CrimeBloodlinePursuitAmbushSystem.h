#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrimeBloodlinePursuitAmbushSystem.generated.h"

class ACrimeBloodlineReputationTension;
class ACrimeBloodlineGangSystem;

UENUM(BlueprintType)
enum class EPursuitEscalationLevel : uint8
{
    Clear           UMETA(DisplayName = "الأجواء هادئة"),
    PatrolSearch    UMETA(DisplayName = "دوريات وتفتيش"),
    ActiveChase     UMETA(DisplayName = "مطاردة نشطة"),
    AllOutLockdown  UMETA(DisplayName = "استنفار شامل وكمين")
};

USTRUCT(BlueprintType)
struct FPursuitAndAmbushState
{
    GENERATED_BODY()

    // مستوى سخونة المطاردة من 0 إلى 100
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit|Law", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PursuitHeat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit|Law")
    EPursuitEscalationLevel EscalationLevel = EPursuitEscalationLevel::Clear;

    // هل يوجد كمين عصابة منصوب حالياً؟
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit|Ambush")
    bool bGangAmbushActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit|Ambush")
    FString AmbushOrChaseHeadline = TEXT("لا توجد مطاردة أو كمين حالياً");

    // هل توجد لقطة مطاردة أو كمين جاهزة للترند؟
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit|Trend")
    bool bPursuitTrendClipReady = false;
};

UCLASS()
class ACrimeBloodlinePursuitAmbushSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrimeBloodlinePursuitAmbushSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Pursuit")
    TObjectPtr<ACrimeBloodlineReputationTension> ReputationTensionActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Pursuit")
    TObjectPtr<ACrimeBloodlineGangSystem> GangSystemActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Pursuit")
    FPursuitAndAmbushState PursuitState;

    // تحديث المطاردة والكمائن بناءً على توتر المنطقة وحالة العصابة
    UFUNCTION(BlueprintCallable, Category = "CrimeBloodline|Pursuit")
    void UpdatePursuitAndAmbush(float DeltaTime, bool bPlayerVisibleToLawOrGang);

    // إشعال مطاردة أو كمين فوري عند حدوث مخالفة أو غدرة
    UFUNCTION(BlueprintCallable, Category = "CrimeBloodline|Pursuit")
    void TriggerHeatSpike(float HeatAdded, bool bForceGangAmbush, const FString& CustomAlert);

    UFUNCTION(BlueprintCallable, Category = "CrimeBloodline|Pursuit")
    void ConsumePursuitTrendFlag();
};
