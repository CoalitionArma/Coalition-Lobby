modded class SCR_PlayerController
{
	//Stores local camera entity to delete whenever you take over a player
	protected IEntity m_eCamera;
	//Stores the vector of your last entity you had control over so it can teleport your camera to it
	protected vector m_vLastEntityTransform[4];
	
	protected bool m_bIsListening = false;
	int m_iFPS;
	int m_iAudioSetting;

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

		GetGame().GetInputManager().AddActionListener("CLB_OpenLobby", EActionTrigger.PRESSED, OpenMenu);
		GetGame().GetInputManager().AddActionListener("CLB_EnterListening", EActionTrigger.PRESSED, Action_SetListening);
		PlayerJoined();
	}
	
	void PlayerJoined()
	{
		if (GetPlayerId() == 0)
		{
			GetGame().GetCallqueue().CallLater(PlayerJoined, 100, false);
			return;
		}
		
		if (CLB_Gamemode.GetInstance().m_aSlots.Find(GetPlayerId()) == -1)
		{
			Rpc(RpcDo_SetIntialEntity, GetPlayerId());
			GetGame().GetCallqueue().CallLater(CLB_Gamemode.GetInstance().OpenMenu, 500, false);
		}
		else
		{
			Rpc(RpcDo_SetIntialEntity, GetPlayerId());
			GetGame().GetCallqueue().CallLater(EnterGame, 500, false, GetPlayerId());
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SetIntialEntity(int playerID)
	{
		CLB_Gamemode.GetInstance().SpawnInitialEntity(playerID);
	}
	
	void Action_SetListening()
	{
		SCR_VONController vonController = SCR_VONController.Cast(GetGame().GetPlayerController().FindComponent(SCR_VONController));
		vonController.PublicResetVON();
		m_bIsListening = !m_bIsListening;
		SetListening(m_bIsListening);
		if(m_bIsListening)
			vonController.SetVONDisabled(true);
		else
			vonController.SetVONDisabled(false);
	}
	
	void SetListening(bool input)
	{
		Rpc(RpcDo_SetListening, SCR_PlayerController.GetLocalPlayerId(), input);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SetListening(int playerID, bool input)
	{
		SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID)).SetListening(input);
	}
	
	void Respawn(int playerID, string prefab, vector position, int groupID)
	{
		Rpc(RpcDo_Respawn, playerID, prefab, position, groupID);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_Respawn(int playerID, string prefab, vector position, int groupID)
	{
		CLB_Gamemode.GetInstance().RespawnPlayer(playerID, prefab, position, groupID);
	}
	
	void UpdateCameraPos(vector cameraPos[4])
	{
		//Rpc(RpcDo_UpdateCameraPos, entityID, cameraPos);
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		vector transform[4];
		player.GetWorldTransform(transform);
		transform[3] = cameraPos[3];
		
		//~ Align to terrain if not a character
		if (!ChimeraCharacter.Cast(player))
			SCR_TerrainHelper.OrientToTerrain(transform);

		BaseGameEntity baseGameEntity = BaseGameEntity.Cast(player);
		if (baseGameEntity)
			baseGameEntity.Teleport(transform);
		else
			player.SetWorldTransform(transform);

		Physics phys = player.GetPhysics();
		if (phys)
		{
			phys.SetVelocity(vector.Zero);
			phys.SetAngularVelocity(vector.Zero);
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_UpdateCameraPos(RplId entityID, vector cameraPos[4])
	{
		//CLB_Gamemode.GetInstance().SetCameraPos(entityID, cameraPos);
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
			CLB_Gamemode.GetInstance().SetPlayerTalking(playerID);
		else
			CLB_Gamemode.GetInstance().RemovePlayerTalking(playerID);
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
		CLB_Gamemode.GetInstance().AdvanceGamemodeState();
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
		CLB_Gamemode.GetInstance().SetSlot(index, playerID);
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
		CLB_Gamemode.GetInstance().SetGroupLockedStatus(index, input);
	}
	
	//Called to enter the game, decides if you will go into spectator on the client
	void EnterGame(int playerID)
	{
		//Call to server to enter slot and or get put into a initial entity to spectate
		if(m_eCamera)
			delete m_eCamera;
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_PreviewMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_SlottingMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_SpectatorMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_AARMenu);
		Rpc(RpcDo_EnterGame, playerID);
		if(CLB_Gamemode.GetInstance().m_aSlots.Find(playerID) == -1)
			EnterSpectator();
		else if(CLB_Gamemode.GetInstance().m_aEntityDeathStatus.Get(CLB_Gamemode.GetInstance().m_aSlots.Find(playerID)))
			EnterSpectator();
		if(m_iFPS == 0)
		{
			BaseContainer video = GetGame().GetEngineUserSettings().GetModule("VideoUserSettings");
			video.Set("MaxFps", 0);	
			GetGame().UserSettingsChanged();
		}
		else
		{
			BaseContainer video = GetGame().GetEngineUserSettings().GetModule("VideoUserSettings");
			video.Set("MaxFps", m_iFPS);	
			GetGame().UserSettingsChanged();
		}
		if(m_iAudioSetting == 0)
			AudioSystem.SetMasterVolume(AudioSystem.SFX, 100);
		else
			AudioSystem.SetMasterVolume(AudioSystem.SFX, m_iAudioSetting);
	}
	
	//Communicates to server to enter slot and or get put into a initial entity to spectate
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_EnterGame(int playerID)
	{
		CLB_Gamemode.GetInstance().EnterGame(playerID);
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
		if(CLB_Gamemode.GetInstance().m_aSlots.Find(SCR_PlayerController.GetLocalPlayerId()) != -1)
			params.Transform = m_vLastEntityTransform;
		else
			params.Transform = CLB_Gamemode.GetInstance().m_vGenericSpawn;
		
		m_bIsListening = false;
		
		if(SCR_EditorManagerEntity.GetInstance().IsOpened())
			return;
		
		m_eCamera = GetGame().SpawnEntityPrefab(Resource.Load("{E1FF38EC8894C5F3}Prefabs/Editor/Camera/ManualCameraSpectate.et"), GetGame().GetWorld(), params);
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_SpectatorMenu);
		GetGame().GetCameraManager().SetCamera(CameraBase.Cast(m_eCamera));
	}
	
	//Opens the slotting menu for players in game
	void OpenMenu(float value = 0.0, EActionTrigger reason = 0)
	{
		if(value != 1)
			return;
		
		MenuBase topMenu = GetGame().GetMenuManager().GetTopMenu();
		if(topMenu)
			if(topMenu.IsInherited(CLB_PreviewMenuUI) || topMenu.IsInherited(CLB_SlottingMenuUI))
				return;
			else if(topMenu.IsInherited(CLB_SpectatorMenuUI))
				GetGame().GetMenuManager().CloseMenu(topMenu);
		
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_SlottingMenu);
		if(CLB_Gamemode.GetInstance().m_GamemodeState != CLB_GamemodeState.GAME)
		{
			BaseContainer video = GetGame().GetEngineUserSettings().GetModule("VideoUserSettings");
			if(m_iFPS)
				video.Get("MaxFps", m_iFPS);
			video.Set("MaxFps", 30);
			GetGame().UserSettingsChanged();
			if(m_iAudioSetting)
				m_iAudioSetting = AudioSystem.GetMasterVolume(AudioSystem.SFX);
			AudioSystem.SetMasterVolume(AudioSystem.SFX, 0);
		}
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
		CLB_Gamemode.GetInstance().AdvanceSlottingState();
	}
}