modded class SCR_EditorManagerEntity
{
	override void StartEvents(EEditorEventOperation type = EEditorEventOperation.NONE)
	{
		super.StartEvents(type);
		
		if(type == EEditorEventOperation.OPEN)
		{	
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_PreviewMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SlottingMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SpectatorMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_AARMenu);
		}
		else if(type == EEditorEventOperation.CLOSE)
		{	
			if(CRF_Gamemode.GetInstance())
			{
				GetGame().GetCallqueue().CallLater(OpenUI, 100, false);
			}
		}
	}
	
	void OpenUI()
	{
		CRF_Gamemode gamemode = CRF_Gamemode.GetInstance();
		if(gamemode)
			if(gamemode.m_aSlots.Find(SCR_PlayerController.GetLocalPlayerId()) != -1)
				if(RplComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(RplComponent)).Id() != gamemode.m_aEntitySlots.Get(gamemode.m_aSlots.Find(SCR_PlayerController.GetLocalPlayerId())))
					return;
		
		switch(CRF_Gamemode.GetInstance().m_GamemodState)
		{
			case CRF_GamemodeState.INITIAL: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_PreviewMenu);	break;}
			case CRF_GamemodeState.SLOTTING: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_SlottingMenu);	break;}
			case CRF_GamemodeState.GAME: 		{SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterGame(SCR_PlayerController.GetLocalPlayerId());	break;}
			case CRF_GamemodeState.AAR: 		{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_AARMenu);	break;}
		}
	}
}