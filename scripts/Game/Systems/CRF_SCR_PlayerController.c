modded class SCR_PlayerController
{
	//Stores local camera entity to delete whenever you take over a player
	protected IEntity m_eCamera;
	//Stores the vector of your last entity you had control over so it can teleport your camera to it
	protected vector m_vLastEntityTransform[4];

	//Adds action lisener to open menu in game
	override protected void UpdateLocalPlayerController()
	{
		super.UpdateLocalPlayerController();
		
		m_bIsLocalPlayerController = this == GetGame().GetPlayerController();
		if (!m_bIsLocalPlayerController)
			return;

		s_pLocalPlayerController = this;
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		GetGame().GetInputManager().AddActionListener("CRF_OpenLobby", EActionTrigger.PRESSED, OpenMenu);
	}
	
	//Called on the local machine whenever game state is changed
	void GameStateChange(int gameState)
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_PreviewMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SlottingMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SpectatorMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_AARMenu);
		switch(gameState)
		{
			case CRF_GamemodeState.SLOTTING: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_SlottingMenu);	break;}
			case CRF_GamemodeState.GAME: 		{EnterGame(GetPlayerId());	break;}
			case CRF_GamemodeState.AAR: 		{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_AARMenu);	break;}
		}
	}
	
	//Communicates to server that the player is talking
	void SetTalking(bool input, int playerID)
	{
		Rpc(RpcDo_SetTalking, input, playerID);
	}
	
	//Communicates to server that the player is talking
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SetTalking(bool input, int playerID)
	{
		if(input)
			CRF_Gamemode.GetInstance().SetPlayerTalking(playerID);
		else
			CRF_Gamemode.GetInstance().RemovePlayerTalking(playerID);
	}
	
	//Communicates to server to advance state
	void AdvanceGamemodeState()
	{
		Rpc(RpcDo_AdvanceGamemodeState);
	}
	
	//Communicates to server to advance state
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_AdvanceGamemodeState()
	{
		CRF_Gamemode.GetInstance().AdvanceGamemodeState();
	}
	
	//Communicates to server to set slot
	void SetSlot(int index, int playerID)
	{
		Rpc(RpcDo_SetSlot, index, playerID);
	}
	
	//Communicates to server to set slot
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SetSlot(int index, int playerID)
	{
		CRF_Gamemode.GetInstance().SetSlot(index, playerID);
	}
	
	//Communicates to server to set group locked
	void SetGroupLocked(int index, bool input)
	{
		Rpc(RpcDo_SetGroupLocked, index, input);
	}
	
	//Communicates to server to set group locked
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SetGroupLocked(int index, bool input)
	{
		CRF_Gamemode.GetInstance().SetGroupLockedStatus(index, input);
	}
	
	//Called to enter the game, decides if you will go into spectator on the client
	void EnterGame(int playerID)
	{
		//Call to server to enter slot and or get put into a initial entity to spectate
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_PreviewMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SlottingMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SpectatorMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_AARMenu);
		Rpc(RpcDo_EnterGame, playerID);
		if(CRF_Gamemode.GetInstance().m_aSlots.Find(playerID) == -1)
			EnterSpectator();
		else if(m_eCamera)
			delete m_eCamera;
	}
	
	//Communicates to server to enter slot and or get put into a initial entity to spectate
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_EnterGame(int playerID)
	{
		CRF_Gamemode.GetInstance().EnterGame(playerID);
	}
	
	//Whenever player is killed store their location and enter spectator
	override void OnDestroyed(notnull Instigator killer)
	{
		GetGame().GetCallqueue().CallLater(EnterSpectator, 100, false);
		GetGame().GetPlayerController().GetControlledEntity().GetTransform(m_vLastEntityTransform);
	}
	
	//Spawns camera locally and puts the player into it
	void EnterSpectator()
	{
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		if(CRF_Gamemode.GetInstance().m_aSlots.Find(SCR_PlayerController.GetLocalPlayerId()) != -1)
			params.Transform = m_vLastEntityTransform;
		else
			params.Transform = CRF_Gamemode.GetInstance().m_vGenericSpawn;
		
		m_eCamera = GetGame().SpawnEntityPrefab(Resource.Load("{E1FF38EC8894C5F3}Prefabs/Editor/Camera/ManualCameraSpectate.et"), GetGame().GetWorld(), params);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_SpectatorMenu);
		GetGame().GetCameraManager().SetCamera(CameraBase.Cast(m_eCamera));
	}
	
	//Opens the slotting menu for players in game
	void OpenMenu(float value = 0.0, EActionTrigger reason = 0)
	{
		if(value != 1)
			return;
		
		MenuBase topMenu = GetGame().GetMenuManager().GetTopMenu();
		if(topMenu)
			if(topMenu.IsInherited(CRF_PreviewMenuUI) || topMenu.IsInherited(CRF_SlottingMenuUI))
				return;
			else if(topMenu.IsInherited(CRF_SpectatorMenuUI))
				GetGame().GetMenuManager().CloseMenu(topMenu);
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_SlottingMenu);
	}
	
	//Communicates to server to advance the slotting phase
	void AdvanceSlottingPhase()
	{
		Rpc(RpcDo_AdvanceSlottingPhase);
	}
	
	//Communicates to server to advance the slotting phase
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_AdvanceSlottingPhase()
	{
		CRF_Gamemode.GetInstance().AdvanceSlottingState();
	}
}