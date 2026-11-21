#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrimeBloodlineGangSystem.generated.h"

class ACrimeBloodlineReputationTension;

UENUM(BlueprintType)
enum class EGangStanding : uint8
{
    LoyalAlly       UMETA(DisplayName = "حليف وفي"),
    Cautious        UMETA(DisplayName = "حذر ومراقبة"),
    MarkedForHit    UMETA(DisplayName = "مطلوب للتصفية"),
    BetrayedOutlaw  UMETA(DisplayName = "خيانة وعداوة مفتوحة")
};

USTRUCT(BlueprintType)
struct FGangFactionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gang")
    FString GangName = TEXT("عصابة الشمال");

    // مستوى الولاء بين اللاعب والعصابة (-100 عداء تام، 0 محايد، +100 ولاء مطلق)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gang|Relations", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
    float LoyaltyScore = 25.0f;

    // عداد الخيانة أو الغدرة (من 0 إلى 100) - إذا وصل للحد الأعلى تنفجر المواجهة
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gang|Relations", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float BetrayalHeat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gang|Relations")
    EGangStanding CurrentStanding = EGangStanding::LoyalAlly;

    // هل تم تفعيل انقلاب أو كمين أو لقطة غدرة جاهزة للعرض/التصوير؟
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gang|Drama")
    bool bBetrayalClipReady = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gang|Drama")
    FString DramaHeadline = TEXT("الأوضاع مستقرة مع العصابة");
};

UCLASS()
class ACrimeBloodlineGangSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrimeBloodlineGangSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Gang")
    TObjectPtr<ACrimeBloodlineReputationTension> ReputationTensionActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrimeBloodline|Gang")
    FGangFactionState GangState;

    // تنفيذ فعل مع العصابة (مساعدة، غدرة، تصفية منافس، خرق اتفاق)
    UFUNCTION(BlueprintCallable, Category = "CrimeBloodline|Gang")
    void ApplyGangAction(
        float LoyaltyChange,
        float BetrayalHeatChange,
        bool bIsBetrayalAct,
        const FString& CustomHeadline
    );

    // مزامنة موقف العصابة مع سمعة اللاعب وتوتر المنطقة الحالية
    UFUNCTION(BlueprintCallable, Category = "CrimeBloodline|Gang")
    void SyncWithReputationAndTension();

    UFUNCTION(BlueprintCallable, Category = "CrimeBloodline|Gang")
    void ConsumeBetrayalClipFlag();
};
