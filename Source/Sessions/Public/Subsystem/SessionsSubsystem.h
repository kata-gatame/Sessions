// kata.codes
#pragma once

#include "CoreMinimal.h"
#include "Helper/Enums.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionsOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSessionsOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool WasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FSessionsOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionsOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSessionsOnStartSessionComplete, bool, bWasSuccessful);

UCLASS()
class SESSIONS_API USessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumberOfConnections{ 4 };
	EMatchType LastMatchType{ EMatchType::EMT_FFA };

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

public:
	USessionsSubsystem();
	
	FSessionsOnCreateSessionComplete SessionsOnCreateSessionComplete;
	FSessionsOnFindSessionsComplete SessionsOnFindSessionsComplete;
	FSessionsOnJoinSessionComplete SessionsOnJoinSessionComplete;
	FSessionsOnStartSessionComplete SessionsOnStartSessionComplete;
	FSessionsOnDestroySessionComplete SessionsOnDestroySessionComplete;

	void CreateSession(int32 NumPublicConnections, EMatchType MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void StartSession();
	void DestroySession();
};