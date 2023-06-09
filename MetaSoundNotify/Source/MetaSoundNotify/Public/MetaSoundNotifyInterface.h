#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "MetaSoundNotifyInterface.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UMetaSoundNotifyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface for the Notify Nodes.
 */
class METASOUNDNOTIFY_API IMetaSoundNotifyInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Notifies, meta = (ToolTip = "Event for Simple Notify Nodes."))
	void MetaSoundsNotify(const int32& NotifyID);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Notifies, meta = (ToolTip = "Event for String Notify Nodes."))
	void MetaSoundsNotifyString(const int32& NotifyID, const FString& Message);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Notifies, meta = (ToolTip = "Event for Int Notify Nodes."))
	void MetaSoundsNotifyInt(const int32& NotifyID, const int32& Value);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Notifies, meta = (ToolTip = "Event for Float Notify Nodes."))
	void MetaSoundsNotifyFloat(const int32& NotifyID, const float& Value);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Notifies, meta = (ToolTip = "Event for Bool Notify Nodes."))
	void MetaSoundsNotifyBool(const int32& NotifyID, const bool Value);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Notifies, meta = (ToolTip = "Event for  'Notify Cue Point' nodes."))
	void MetaSoundsNotifyCuePoint(const int32& NotifyID, const int32 CuePointID, const FString& CuePointLabel);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Notifies, meta = (ToolTip = "Event for 'Notify Raw Cue Point' nodes."))
	void MetaSoundsNotifyRawCuePoint(const int32& NotifyID, const FString& Message);
};
