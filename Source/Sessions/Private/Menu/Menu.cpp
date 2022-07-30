// kata.codes
#include "Menu/Menu.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "Helper/Enums.h"
#include "Subsystem/SessionsSubsystem.h"

#pragma region Menu Construction/Destruction

#pragma region Initialize Menu
/**
 * Initialize the Menu system for finding and joining game sessions.
 */
bool UMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	if (HostButton)
		/** add Dynamic Delegate HostButtonPressed */
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonPressed);

	if (JoinButton)
		/** add Dynamic Delegate JoinButtonPressed */
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonPressed);

	return true;
}
#pragma endregion Initialize

#pragma region Setup Menu
/**
 * This will setup the Menu system for finding and joining game sessions.
 * @param Connections - The number of connections allowed.
 * @param TypeOfMatch - The type of match being played.
 * @param LobbyPath - The path to the Lobby map.
 */
void UMenu::Setup(const int32 Connections, const EMatchType TypeOfMatch, const FString LobbyPath)
{
	/** set the number of connections */
	PublicConnections = Connections;

	/** set the Lobby path, append `?listen` */
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);

	/** set the match type */
	MatchType = TypeOfMatch;

	/** add the Menu to the player's viewport */
	AddToViewport();

	/** set Menu visibility */
	SetVisibility(ESlateVisibility::Visible);

	/** set Menu focusable */
	bIsFocusable = true;

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			/**
			 * InputModeData settings for our Menu:
			 *  - specified as UIOnly
			 *  - focus the Menu widget
			 *  - do not lock the cursor to viewport
			 */
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			/** apply InputModeData settings */
			PlayerController->SetInputMode(InputModeData);

			/** show the mouse cursor */
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
		/** set our Subsystem */
		SessionsSubsystem = GameInstance->GetSubsystem<USessionsSubsystem>();

	if (SessionsSubsystem)
	{
		/** add create session completion delegate */
		SessionsSubsystem->SessionsOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);

		/** add destroy session completion delegate */
		SessionsSubsystem->SessionsOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);

		/** add create session completion delegate */
		SessionsSubsystem->SessionsOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);

		/** add find sessions completion delegate */
		SessionsSubsystem->SessionsOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);

		/** add join session completion delegate */
		SessionsSubsystem->SessionsOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
	}
}
#pragma endregion Setup Menu

#pragma region Destroy Menu
/**
 * Destroy the Menu system.
 */
void UMenu::Destroy()
{
	/** remove Menu widget */
	RemoveFromParent();

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			/**
			 * InputModeData settings for removing our Menu:
			 *  - specified as GameOnly
			 */
			const FInputModeGameOnly InputModeData;

			/** apply InputModeData settings */
			PlayerController->SetInputMode(InputModeData);

			/** hide the mouse cursor */
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
#pragma endregion Destroy Menu

#pragma endregion Menu Construction/Destruction

#pragma region Menu Button Interaction

#pragma region Host Button Press
/**
 * Called when the Host button is pressed.
 */
void UMenu::HostButtonPressed()
{
	/** disable the Host button */
	HostButton->SetIsEnabled(false);

	if (!SessionsSubsystem) return;

	/** create a session via our Subsystem */
	SessionsSubsystem->CreateSession(PublicConnections, MatchType);
}
#pragma endregion Host Button Press

#pragma region Join Button Press
/**
 * Called when the Join button is pressed.
 */
void UMenu::JoinButtonPressed()
{
	/** disable the Join button */
	JoinButton->SetIsEnabled(false);

	if (!SessionsSubsystem) return;

	/** find available sessions via our Subsystem */
	SessionsSubsystem->FindSessions(10000);
}
#pragma endregion Join Button Press

#pragma endregion Menu Button Interaction

#pragma region Menu Functionality

#pragma region Create Session
/** Called when the ...
 * @param bWasSuccessful - Was it successful?
 */
void UMenu::OnCreateSession(const bool bWasSuccessful)
{
	if (bWasSuccessful)
		/** SUCCESS */
		if (UWorld* World = GetWorld())
			/** travel to Lobby map */
			World->ServerTravel(PathToLobby);
		else
			/** FAIL */
			/** enable Host button */
			HostButton->SetIsEnabled(true);
}
#pragma endregion Create Session

#pragma region Find Session
/**
 * Called when the ...
 */
void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, const bool bWasSuccessful) const
{
	if (SessionsSubsystem == nullptr) return;

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		
		if (SettingsValue == *UEnum::GetValueAsName(MatchType).ToString())
		{
			SessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
		/** enable Join button */
		JoinButton->SetIsEnabled(true);
}
#pragma endregion Find Session

#pragma region Join Session
/**
 * Called when the ...
 */
void UMenu::OnJoinSession(const EOnJoinSessionCompleteResult::Type Result) const
{
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface(); SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			if (APlayerController* PlayerController{ GetGameInstance()->GetFirstLocalPlayerController() })
				/** join session */
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}

	/** any Result that isn't `Success` */
	if (Result != EOnJoinSessionCompleteResult::Success)
		/** enable Join button */
		JoinButton->SetIsEnabled(true);
}
#pragma endregion Join Session

#pragma region Start Session
/** Called when the ...
 * @param bWasSuccessful - Was it successful?
 */
void UMenu::OnStartSession(const bool bWasSuccessful)
{
	if(bWasSuccessful)
		if(UWorld* World = GetWorld())
			World->ServerTravel(PathToGame);
}
#pragma endregion Start Session

#pragma region Destroy Session
/** Called when the ...
 * @param bWasSuccessful - Was it successful?
 */
void UMenu::OnDestroySession(const bool bWasSuccessful)
{
	
}

/** Called when the current level is removed from the world.
 * @param InLevel - The Level to being removed from the World.
 * @param InWorld - The World being removed from.
 */
void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	/** call our Destroy function */
	Destroy();

	/** call the Super */
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
#pragma endregion Destroy Session

#pragma endregion Menu Functionality