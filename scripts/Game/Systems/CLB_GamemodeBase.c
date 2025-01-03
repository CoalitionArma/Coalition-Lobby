class CLB_GamemodeClass: SCR_BaseGameModeClass
{
};

enum CLB_GamemodeState
{
	INITIAL,
	SLOTTING,
	GAME,
	AAR
}

enum CLB_SlottingState
{
	LEADERSANDMEDICS,
	SPECIALTIES,
	EVERYONE
}

[BaseContainerProps()]
class CLB_MissionDescriptor
{
	[Attribute("")]
	string m_sTitle;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBoxMultiline)]
	string m_sTextData;
	
	[Attribute("")]
	ref array<string> m_aFactionKeys;
	
	[Attribute("")]
	bool m_bShowForAnyFaction;
}

class CLB_Gamemode : SCR_BaseGameMode
{
	[RplProp(onRplName: "OnGamemodeStateChanged")]
	int m_GamemodeState = CLB_GamemodeState.INITIAL;
	
	[RplProp()]
	int m_SlottingState = CLB_SlottingState.LEADERSANDMEDICS;
	
	//Stores when a player is talking
	[RplProp()]
	ref array<int> m_aPlayersTalking = {};
	
	//Slot ID given to an entity
	[RplProp()]
	ref array<int> m_aSlots = {};
	
	//Slot name of entity
	[RplProp()]
	ref array<string> m_aSlotNames = {};
	
	//Player that is slotted in the entities name
	[RplProp()]
	ref array<string> m_aSlotPlayerNames = {};
	
	//Slot icon off of UI Info
	[RplProp()]
	ref array<ResourceName> m_aSlotIcons = {};
	
	[RplProp()]
	ref array<ResourceName> m_aSlotPrefabs = {};
	
	//Entities slot type, leader, specialty, everyone
	[RplProp()]
	ref array<int> m_aEntitySlotTypes = {};
	
	//Is the entity dead or alive, needed as entities pop in and out of streamable.
	[RplProp()]
	ref array<bool> m_aEntityDeathStatus = {};
	
	//RplId of entities that are playable
	[RplProp()]
	ref array<RplId> m_aEntitySlots = {};
	
	//Stores the group ID for each slot, so I can reference what group a slot is in. CAUSE THERE IS NO WAY TO DO THAT ON THE CLIENT.
	[RplProp()]
	ref array<RplId> m_aPlayerGroupIDs = {};
	
	//Communicates change across all clients so they can refresh their slots in the UI
	[RplProp()]
	int m_iSlotChanges = 0;
	
	//Is a group locked
	[RplProp()]
	ref array<bool> m_aGroupLockedStatus = {};
	
	//Stores SCR_AIGroup RplId, CAUSE YOU CAN'T FUCKING GRAB NON PLAYABLE GROUPS BOHEMIA
	[RplProp()]
	ref array<RplId> m_aGroupRplIDs = {};
	
	//Stores the playable group created whenever an AI group is created in the editor
	[RplProp()]
	ref array<RplId> m_aActivePlayerGroupsIDs = {};
	
	//Just stores a generic spawnpoint for players to spawn the spectator cam on. Cause of entities being streamable and such.
	[RplProp()]
	vector m_vGenericSpawn[4];
	
	[RplProp()]
	RplId m_rSpectatorGroup;
	
	//This just is what is auto set in the slotting UI for ratio calculation
	[Attribute("1", "auto", "", category: "CRF Gamemode Slotting")]
	int m_iFactionOneRatio;
	
	//This just is what is auto set in the slotting UI for ratio calculation
	[Attribute("", uiwidget: UIWidgets.ComboBox, enums: {ParamEnum("", ""), ParamEnum("BLU", "BLU"), ParamEnum("OPF", "OPF"), ParamEnum("IND", "IND"), ParamEnum("CIV", "CIV")}, category: "CRF Gamemode Slotting")]
	string m_sFactionOneKey;
	
	//This just is what is auto set in the slotting UI for ratio calculation
	[Attribute("1", "auto", "", category: "CRF Gamemode Slotting")]
	int m_iFactionTwoRatio;
	
	//This just is what is auto set in the slotting UI for ratio calculation
	[Attribute("", uiwidget: UIWidgets.ComboBox, enums: {ParamEnum("", ""), ParamEnum("BLU", "BLU"), ParamEnum("OPF", "OPF"), ParamEnum("IND", "IND"), ParamEnum("CIV", "CIV")}, category: "CRF Gamemode Slotting")]
	string m_sFactionTwoKey;
	
	//Descriptions on the left in briefing
	[Attribute("", category: "CLB Gamemode Descriptors")]
	ref	array<ref CLB_MissionDescriptor> m_aMissionDescriptors;
	
	IEntity m_eGamemodeEntity;
	
	//
	protected ref array<CLB_GamemodeComponent> m_aAdditionalCLBGamemodeComponents = {};
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_eGamemodeEntity = owner;
		
		array<Managed> additionalComponents = new array<Managed>();
		int count = owner.FindComponents(CLB_GamemodeComponent, additionalComponents);
		m_aAdditionalCLBGamemodeComponents.Clear();
		for (int i = 0; i < count; i++)
		{
			CLB_GamemodeComponent comp = CLB_GamemodeComponent.Cast(additionalComponents[i]);
			m_aAdditionalCLBGamemodeComponents.Insert(comp);
		}
	}
	
	static CLB_Gamemode GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return CLB_Gamemode.Cast(gameMode);
		else
			return null;
	}

	protected override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		//Throw em into spectator
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		GetGame().GetCallqueue().CallLater(SetPlayerSpectator, 100, false, playerId, playerEntity);
	}
	
	void SetPlayerSpectator(int playerId, IEntity playerEntity)
	{
		if(playerEntity.GetPrefabData().GetPrefabName() != "{59886ECB7BBAF5BC}Prefabs/Characters/CLB_InitialEntity.et")
			SpawnInitialEntity(playerId);
	}
	
	void SpawnInitialEntity(int playerID)
	{
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = "0 10000 0";
		IEntity initialEntity = GetGame().SpawnEntityPrefab(Resource.Load("{59886ECB7BBAF5BC}Prefabs/Characters/CLB_InitialEntity.et"),GetGame().GetWorld(),spawnParams);
		GetGame().GetCallqueue().CallLater(SetPlayerEntity, 200, false, initialEntity, playerID);
		SCR_AIGroup currentGroup = SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(playerID);
		if(currentGroup)
			currentGroup.RemovePlayer(playerID);
		SCR_CharacterDamageManagerComponent.Cast(initialEntity.FindComponent(SCR_CharacterDamageManagerComponent)).EnableDamageHandling(false);
		SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID).FindComponent(SCR_PlayerFactionAffiliationComponent)).RequestFaction(GetGame().GetFactionManager().GetFactionByKey("SPEC"));
		return;
	}
	
	void SetCameraPos(int playerID, vector cameraPos[4])
	{
		RpcDo_SetCameraPos(playerID, cameraPos);
		Rpc(RpcDo_SetCameraPos, playerID, cameraPos);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void RpcDo_SetCameraPos(int playerID, vector cameraPos[4])
	{	
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if(!player)
			return;
		vector transform[4];
		player.GetWorldTransform(transform);
		transform[3] = cameraPos[3];

		BaseGameEntity playerEntity = BaseGameEntity.Cast(player); 
        
        if(!playerEntity)
            return;
        
        float scale = playerEntity.GetScale();
		playerEntity.Teleport(transform);
        playerEntity.SetWorldTransform(transform);
        playerEntity.SetScale(scale);
        playerEntity.Update();
        playerEntity.OnTransformReset();

		Physics phys = player.GetPhysics();
		if (phys)
		{
			phys.SetVelocity(vector.Zero);
			phys.SetAngularVelocity(vector.Zero);
		}
	}
	
	//Called to enter the actual game, just puts the player into a slot or spectator.
	void EnterGame(int playerID)
	{
		if(m_aSlots.Find(playerID) == -1 && GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID).GetPrefabData().GetPrefabName() != "{59886ECB7BBAF5BC}Prefabs/Characters/CLB_InitialEntity.et")
			SpawnInitialEntity(playerID);
		
		if(m_aSlots.Find(playerID) == -1)
			return;
		
		if(m_aEntityDeathStatus.Get(m_aSlots.Find(playerID)))
		{
			SpawnInitialEntity(playerID);
			return;
		}
		
		IEntity oldEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		RplId oldGroup = RplId.Invalid();
		if(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID).GetPrefabData().GetPrefabName() != "{59886ECB7BBAF5BC}Prefabs/Characters/CLB_InitialEntity.et")
			oldGroup = m_aActivePlayerGroupsIDs.Get(m_aGroupRplIDs.Find(m_aPlayerGroupIDs.Get(m_aEntitySlots.Find(RplComponent.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID).FindComponent(RplComponent)).Id()))));
		SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID)).SetInitialMainEntity(RplComponent.Cast(Replication.FindItem(m_aEntitySlots.Get(m_aSlots.Find(playerID)))).GetEntity());
		SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID).FindComponent(SCR_PlayerFactionAffiliationComponent)).RequestFaction(SCR_AIGroup.Cast(RplComponent.Cast(Replication.FindItem(m_aActivePlayerGroupsIDs.Get(m_aGroupRplIDs.Find(m_aPlayerGroupIDs.Get(m_aSlots.Find(playerID)))))).GetEntity()).GetFaction());
		if(oldGroup != RplId.Invalid())
		{
			if(oldGroup != m_aActivePlayerGroupsIDs.Get(m_aGroupRplIDs.Find(m_aPlayerGroupIDs.Get(m_aSlots.Find(playerID)))))
				SCR_AIGroup.Cast(RplComponent.Cast(Replication.FindItem(m_aActivePlayerGroupsIDs.Get(m_aGroupRplIDs.Find(m_aPlayerGroupIDs.Get(m_aSlots.Find(playerID)))))).GetEntity()).AddPlayer(playerID);
		}
		else
			SCR_AIGroup.Cast(RplComponent.Cast(Replication.FindItem(m_aActivePlayerGroupsIDs.Get(m_aGroupRplIDs.Find(m_aPlayerGroupIDs.Get(m_aSlots.Find(playerID)))))).GetEntity()).AddPlayer(playerID);
		
		if(oldEntity.GetPrefabData().GetPrefabName() == "{59886ECB7BBAF5BC}Prefabs/Characters/CLB_InitialEntity.et")
			GetGame().GetCallqueue().CallLater(DeleteOldEntity, 100, false, oldEntity);
	}
	
	//Gotta wait for stuff and thangssss
	void DeleteOldEntity(IEntity entity)
	{
		delete entity;
	}
	
	//Initializes group into the replicated arrays
	void AddGroup(SCR_AIGroup group)
	{
		m_aGroupRplIDs.Insert(RplComponent.Cast(group.FindComponent(RplComponent)).Id());
		m_aGroupLockedStatus.Insert(false);
		SCR_AIGroup newGroup = SCR_GroupsManagerComponent.GetInstance().CreateNewPlayableGroup(group.GetFaction());
		newGroup.SetCanDeleteIfNoPlayer(false);
		m_aActivePlayerGroupsIDs.Insert(RplComponent.Cast(newGroup.FindComponent(RplComponent)).Id());
		Replication.BumpMe();
	}
	
	//Sets the group to be locked
	void SetGroupLockedStatus(int index, bool input)
	{
		m_aGroupLockedStatus.Set(index, input);
	}
	
	//Sets slot to player or removes him from it
	void SetSlot(int index, int playerID)
	{
		if(playerID > 0)
		{
			SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID).FindComponent(SCR_PlayerFactionAffiliationComponent)).RequestFaction(FactionAffiliationComponent.Cast(RplComponent.Cast(Replication.FindItem(m_aEntitySlots.Get(index))).GetEntity().FindComponent(FactionAffiliationComponent)).GetAffiliatedFaction());
			m_aSlotPlayerNames.Set(index, GetGame().GetPlayerManager().GetPlayerName(playerID));
		}
		else
		{
			if(m_aSlots.Get(index) > 0)
			{
				SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(m_aSlots.Get(index)).FindComponent(SCR_PlayerFactionAffiliationComponent)).RequestFaction(GetGame().GetFactionManager().GetFactionByKey("SPEC"));
				m_aSlotPlayerNames.Set(index, "");
			}
		}
		m_aSlots.Set(index, playerID);
		m_iSlotChanges++;
		Replication.BumpMe();
	}
	
	//Sets if an entity is dead or not in the array
	void SetDeathState(IEntity entity, bool input)
	{
		m_aEntityDeathStatus.Set(m_aEntitySlots.Find(RplComponent.Cast(entity.FindComponent(RplComponent)).Id()), input);
		m_iSlotChanges++;
		Replication.BumpMe();
	}
	
	//Initializes playable entities and adds their values into the replicated arrays
	int AddPlayableEntity(IEntity entity)
	{
		int index = m_aSlots.Insert(0);
		m_aEntitySlots.Insert(RplComponent.Cast(entity.FindComponent(RplComponent)).Id());
		m_aPlayerGroupIDs.Insert(RplComponent.Cast(SCR_AIGroup.Cast(ChimeraAIControlComponent.Cast(entity.FindComponent(ChimeraAIControlComponent)).GetControlAIAgent().GetParentGroup()).FindComponent(RplComponent)).Id());
		m_aSlotNames.Insert(CLB_PlayableCharacter.Cast(entity.FindComponent(CLB_PlayableCharacter)).GetName());
		m_aSlotPrefabs.Insert(entity.GetPrefabData().GetPrefabName());
		m_aSlotIcons.Insert(SCR_EditableCharacterComponent.Cast(entity.FindComponent(SCR_EditableCharacterComponent)).GetInfo().GetIconPath());
		m_aEntityDeathStatus.Insert(false);
		m_aSlotPlayerNames.Insert("");
		if(CLB_PlayableCharacter.Cast(entity.FindComponent(CLB_PlayableCharacter)).IsLeader())
			m_aEntitySlotTypes.Insert(0);
		else if(CLB_PlayableCharacter.Cast(entity.FindComponent(CLB_PlayableCharacter)).IsSpecialty())
			m_aEntitySlotTypes.Insert(1);
		else
			m_aEntitySlotTypes.Insert(2);
		
		if(m_aSlots.Count() == 1)
			entity.GetTransform(m_vGenericSpawn);

		return index;
		
		Replication.BumpMe();
	}
	
	void RemovePlayableEntity(RplId entityID)
	{
		int index = m_aEntitySlots.Find(entityID);
		m_aSlots.RemoveOrdered(index);
		m_aPlayerGroupIDs.RemoveOrdered(index);
		m_aSlotNames.RemoveOrdered(index);
		m_aSlotIcons.RemoveOrdered(index);
		m_aSlotPrefabs.RemoveOrdered(index);
		m_aEntityDeathStatus.RemoveOrdered(index);
		m_aSlotPlayerNames.RemoveOrdered(index);
		m_aEntitySlotTypes.RemoveOrdered(index);
		m_aEntitySlots.RemoveOrdered(index);
		if(Replication.FindItem(entityID))
			SCR_EntityHelper.DeleteEntityAndChildren(RplComponent.Cast(Replication.FindItem(entityID)).GetEntity());
		m_iSlotChanges++;
		Replication.BumpMe();
	}
	
	// Should only ever be ran on the server
	void RespawnPlayer(int playerID, string prefab, vector position, int groupID)
	{
		if(!Replication.IsServer())
		{
			Print("ONLY RUN RespawnPlayer ON SERVER");
			return;
		}
		EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
		vector finalSpawnLocation = vector.Zero;
		SCR_WorldTools.FindEmptyTerrainPosition(finalSpawnLocation, position, 3);
        spawnParams.Transform[3] = finalSpawnLocation;
		IEntity newEntity = GetGame().SpawnEntityPrefab(Resource.Load(prefab),GetGame().GetWorld(),spawnParams);
		GetGame().GetCallqueue().CallLater(RespawnPlayerDelay, 100, false, playerID, groupID, newEntity);
	}
	
	void RespawnPlayerDelay(int playerID, int groupID, IEntity newEntity)
	{
		SCR_AIGroup playerGroup = SCR_GroupsManagerComponent.GetInstance().FindGroup(groupID);
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(RplComponent.Cast(Replication.FindItem(m_aGroupRplIDs.Get(m_aActivePlayerGroupsIDs.Find(RplComponent.Cast(playerGroup.FindComponent(RplComponent)).Id())))).GetEntity());
		aiGroup.AddAIEntityToGroup(newEntity);
		int index = AddPlayableEntity(newEntity);
		SetSlot(m_aSlots.Find(playerID), -2);
		SetSlot(index, playerID);
		Rpc(RpcDo_EnterGame, playerID);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_EnterGame(int playerID)
	{
		if(SCR_PlayerController.GetLocalPlayerId() != playerID)
			return;
		
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterGame(playerID);
	}
	
	//Puts the player into an entity when they connect
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
//		if(m_aSlots.Find(playerId) == -1)
//			SpawnInitialEntity(playerId);
		
		if(m_aSlots.Find(playerId) != -1)
		{
			m_iSlotChanges++;
			Replication.BumpMe();
		}
	}
	
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		m_OnPlayerDisconnected.Invoke(playerId, cause, timeout);
		
		// RespawnSystemComponent is not a SCR_BaseGameModeComponent, so for now we have to
		// propagate these events manually. 
		if (IsMaster())
			m_pRespawnSystemComponent.OnPlayerDisconnected_S(playerId, cause, timeout);

		foreach (SCR_BaseGameModeComponent comp : m_aAdditionalGamemodeComponents)
		{
			comp.OnPlayerDisconnected(playerId, cause, timeout);
		}
		
		m_OnPostCompPlayerDisconnected.Invoke(playerId, cause, timeout);
		//Updates connection status
		if(m_aSlots.Find(playerId) != -1)
		{
			m_iSlotChanges++;
			Replication.BumpMe();
		}
	}
	
	//Sets if the player is talking for UI purposes
	void SetPlayerTalking(int playerID)
	{
		if(m_aPlayersTalking.Find(playerID) != -1)
			return;
		
		m_aPlayersTalking.Insert(playerID);
		Replication.BumpMe();
	}
	
	//Sets if the player is talking for UI purposes
	void RemovePlayerTalking(int playerID)
	{
		m_aPlayersTalking.RemoveItem(playerID);
		Replication.BumpMe();
	}
	
	//Advances the slotting state
	void AdvanceSlottingState()
	{
		m_SlottingState += 1;
		m_iSlotChanges++;
		Replication.BumpMe();
	}
	
	//Opens the menu on the player
	void OpenMenu()
	{
		//Close any menu that wriggles its way in
		MenuBase topMenu = GetGame().GetMenuManager().GetTopMenu();
		if (topMenu)
			topMenu.Close();
		GetGame().GetMenuManager().CloseAllMenus();
		//Opens menu based on current game state : )
		switch(m_GamemodeState)
		{
			case CLB_GamemodeState.INITIAL: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_PreviewMenu);											break;}
			case CLB_GamemodeState.SLOTTING:	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_SlottingMenu);											break;}
			case CLB_GamemodeState.GAME: 		{SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterGame(SCR_PlayerController.GetLocalPlayerId());		break;}
			case CLB_GamemodeState.AAR: 		{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_AARMenu);												break;}
		}
		if(m_GamemodeState != CLB_GamemodeState.GAME)
		{
			BaseContainer video = GetGame().GetEngineUserSettings().GetModule("VideoUserSettings");
			if(SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_iFPS)
				video.Get("MaxFps", SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_iFPS);
			video.Set("MaxFps", 30);
			GetGame().UserSettingsChanged();
			if(SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_iAudioSetting)
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_iAudioSetting = AudioSystem.GetMasterVolume(AudioSystem.SFX);
			AudioSystem.SetMasterVolume(AudioSystem.SFX, 0);
		}
	}
	
	
	//Sets the players entity, just used for a delay while the entity spawns
	void SetPlayerEntity(IEntity entity, int playerId)
	{
		IEntity oldEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId)).SetInitialMainEntity(entity);	
	}

	//Advances the overall gamemode state
	void AdvanceGamemodeState()
	{
		if(m_GamemodeState == CLB_GamemodeState.AAR)
			return;
		
		m_GamemodeState += 1;
		Replication.BumpMe();
		OnGamemodeStateChanged();
	}
	
	void OnGamemodeStateChanged()
	{
		if(!GetGame().GetPlayerController() || RplSession.Mode() == RplMode.Dedicated)
			return;
		
		if(Replication.IsServer())
		{
			foreach (CLB_GamemodeComponent component : m_aAdditionalCLBGamemodeComponents)
				component.OnGamemodeStateChanged();
			
			if(m_GamemodeState == CLB_GamemodeState.AAR)
				EnterAAR();
		}
		
		OpenMenu();
	}
	
	void EnterAAR()
	{
		ref array<int> players = {};
		GetGame().GetPlayerManager().GetAllPlayers(players);
		foreach(int player: players)
		{
			if(GetGame().GetPlayerManager().GetPlayerControlledEntity(player).GetPrefabData().GetPrefabName() == "{59886ECB7BBAF5BC}Prefabs/Characters/CLB_InitialEntity.et")
				continue;
			
			SpawnInitialEntity(player);
		}
	}
}

//Ditto the RL Devs WHY IS THIS HARDCODED
modded class SCR_ManualCamera
{
	override protected bool IsDisabledByMenu()
	{
		if (!m_MenuManager) return false;
		
		if (m_MenuManager.IsAnyDialogOpen()) return true;
		
		MenuBase topMenu = m_MenuManager.GetTopMenu();
		
		// WHY IT'S HARDCODED?
		return topMenu && (!topMenu.IsInherited(EditorMenuUI) && !topMenu.IsInherited(CLB_SpectatorMenuUI));
	}
};