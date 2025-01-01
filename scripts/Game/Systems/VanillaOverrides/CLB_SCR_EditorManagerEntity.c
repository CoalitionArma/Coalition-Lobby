modded class SCR_EditorManagerEntity
{
	override void StartEvents(EEditorEventOperation type = EEditorEventOperation.NONE)
	{
		super.StartEvents(type);
		
		if(type == EEditorEventOperation.OPEN)
		{	
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_PreviewMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_SlottingMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_SpectatorMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CLB_AARMenu);
		}
		else if(type == EEditorEventOperation.CLOSE)
		{	
			if(CLB_Gamemode.GetInstance())
			{
				GetGame().GetCallqueue().CallLater(OpenUI, 200, false);
			}
		}
	}
	
	void OpenUI()
	{
		CLB_Gamemode gamemode = CLB_Gamemode.GetInstance();
		if(gamemode)
			if(!CLB_PlayableCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(CLB_PlayableCharacter)).IsPlayable() && SCR_PlayerController.GetLocalControlledEntity().GetPrefabData().GetPrefabName() != "{59886ECB7BBAF5BC}Prefabs/Characters/CLB_InitialEntity.et")
				return;
		
		Print("Opening UI");
		switch(CLB_Gamemode.GetInstance().m_GamemodeState)
		{
			case CLB_GamemodeState.INITIAL: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_PreviewMenu);	break;}
			case CLB_GamemodeState.SLOTTING: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_SlottingMenu);	break;}
			case CLB_GamemodeState.GAME: 		{SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterGame(SCR_PlayerController.GetLocalPlayerId());	break;}
			case CLB_GamemodeState.AAR: 		{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CLB_AARMenu);	break;}
		}
	}
}