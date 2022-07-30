// kata.codes
#include "Subsystem/SessionsSubsystem.h"
#include "Helper/Enums.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

#pragma region Constructor
USessionsSubsystem::USessionsSubsystem() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete))
{
	const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	SessionInterface = Subsystem->GetSessionInterface();
}
#pragma endregion Constructor

#pragma region Session Actions

#pragma region Create Session
/**
 * This will create a session with the specified parameters.
 * @param NumPublicConnections - The number of connections allowed.
 * @param MatchType - The type of match being played, in string format.
 */
void USessionsSubsystem::CreateSession(const int32 NumPublicConnections, const EMatchType MatchType)
{
	if (!SessionInterface.IsValid()) return;

	if (const auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession); ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumberOfConnections = NumPublicConnections;
		LastMatchType = MatchType;
		SessionInterface->DestroySession(NAME_GameSession);
	}

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->Set(FName("MatchType"), FString(*UEnum::GetValueAsName(MatchType).ToString()), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->BuildUniqueId = 1;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		SessionsOnCreateSessionComplete.Broadcast(false);
	}
}
#pragma endregion Create Session

#pragma region Find Sessions
/**
 * This will find sessions to join.
 * @param MaxSearchResults - The maximum number of results allowed.
 */
void USessionsSubsystem::FindSessions(const int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid()) return;

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		SessionsOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}
#pragma endregion Find Sessions

#pragma region Join Session
/**
 * This will join the specified session.
 * @param SessionResult - The Session to join.
 */
void USessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		SessionsOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		SessionsOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}
#pragma endregion Join Session

#pragma region Start Session
/**
 * This will start a new session.
 */
void USessionsSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		SessionsOnStartSessionComplete.Broadcast(false);
		return;
	}
	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		SessionsOnStartSessionComplete.Broadcast(false);
	}
}
#pragma endregion Start Session

#pragma region Destroy Session
/**
 * This will destroy the current session.
 */
void USessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		SessionsOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		SessionsOnDestroySessionComplete.Broadcast(false);
	}
}
#pragma endregion Destroy Session

#pragma endregion Session Actions

#pragma region Session Action Complete Delegates

#pragma region On Create Session Complete
/**
 * Called after a session has been created.
 * @param SessionName - The name of the session that was created.
 * @param bWasSuccessful - Was it successfully created?
 */
void USessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

	SessionsOnCreateSessionComplete.Broadcast(true);
}
#pragma endregion On Create Session Complete

#pragma region On Find Sessions Complete
/**
 * Called after a list of sessions has been found.
 * @param bWasSuccessful - Were sessions successfully found?
 */
void USessionsSubsystem::OnFindSessionsComplete(const bool bWasSuccessful)
{
	if (SessionInterface)
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		SessionsOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	SessionsOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}
#pragma endregion On Find Sessions Complete

#pragma region On Join Session Complete
/**
 * Called after a session was joined.
 * @param SessionName - The name of the session that was joined.
 * @param Result - The result of the join action.
 */
void USessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	SessionsOnJoinSessionComplete.Broadcast(Result);
}
#pragma endregion On Join Session Complete

#pragma region On Start Session Complete
/**
 * Called after a session was started.
 * @param SessionName - The name of the session that has started.
 * @param bWasSuccessful - Was it successfully started?
 */
void USessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);

	SessionsOnStartSessionComplete.Broadcast(bWasSuccessful);
}
#pragma endregion On Start Session Complete

#pragma region On Destroy Session Complete
/**
 * Called after a session was destroyed.
 * @param SessionName - The name of the session was destroyed.
 * @param bWasSuccessful - Was it successfully destroyed?
 */
void USessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumberOfConnections, LastMatchType);
	}

	SessionsOnDestroySessionComplete.Broadcast(bWasSuccessful);
}
#pragma endregion On Destroy Session Complete

#pragma endregion Session Action Complete Delegates