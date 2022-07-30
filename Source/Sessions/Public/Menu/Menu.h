// kata.codes
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Helper/Enums.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UButton;
class USessionsSubsystem;

UCLASS()
class SESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
	int32 PublicConnections{ 4 };
	FString PathToLobby{ FString(TEXT("/Game/Maps/Lobby")) };
	FString PathToGame{ FString(TEXT("/Game/Maps/Game")) };
	EMatchType MatchType { EMatchType::EMT_FFA };

	UPROPERTY()
	USessionsSubsystem* SessionsSubsystem;

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	void Destroy();

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful) const;
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result) const;

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);	

	UFUNCTION()
	void HostButtonPressed();

	UFUNCTION()
	void JoinButtonPressed();

public:
	UFUNCTION(BlueprintCallable)
	void Setup(int32 Connections = 4, EMatchType TypeOfMatch = EMatchType::EMT_FFA, FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));
};