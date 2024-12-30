modded enum ChimeraMenuPreset : ScriptMenuPresetEnum
{
	CRF_SpectatorMenu
}

class CRF_SpectatorMenuUI: ChimeraMenuBase
{
	protected ref array<IEntity> m_aEntityIcons = {};
	protected ref array<ref CRF_SpectatorLabelIconCharacter> m_aSpectatorIcons = {};
	protected CRF_Gamemode m_Gamemode;
	protected SCR_ChatPanel m_ChatPanel;
	
	override void OnMenuOpen()
	{
		Widget wChatPanel = GetRootWidget().FindAnyWidget("ChatPanel");
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
		m_Gamemode = CRF_Gamemode.GetInstance();
		GetGame().GetInputManager().AddActionListener("ChatToggle", EActionTrigger.DOWN, Action_OnChatToggleAction);
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, Action_Exit);
		GetGame().GetInputManager().AddActionListener("VONDirect", EActionTrigger.DOWN, Action_VONon);
		GetGame().GetInputManager().AddActionListener("VONDirect", EActionTrigger.UP, Action_VONOff);
	}
	override void OnMenuUpdate(float tDelta)
	{
		foreach(RplId entityID: m_Gamemode.m_aEntitySlots)
		{
			if(!Replication.FindItem(entityID))
				continue;
			
			IEntity entity = RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
			
			if(m_aEntityIcons.Find(entity) != -1)
				continue;
			
			Widget spectatorIconWidget = GetGame().GetWorkspace().CreateWidgets("{68625BAD23CEE68F}UI/Spectator/SpectatorLabelIconCharacter.layout", FrameWidget.Cast(GetRootWidget().FindAnyWidget("IconsFrame")));
			CRF_SpectatorLabelIconCharacter spectatorIcon = CRF_SpectatorLabelIconCharacter.Cast(spectatorIconWidget.FindHandler(CRF_SpectatorLabelIconCharacter));
			spectatorIcon.SetEntity(entity, "Spine3");
			m_aEntityIcons.Insert(entity);
			m_aSpectatorIcons.Insert(spectatorIcon);
		}
		UpdateIcons();
		
		if (m_ChatPanel)
        	m_ChatPanel.OnUpdateChat(tDelta);
	}
	
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("ChatToggle", EActionTrigger.DOWN, Action_OnChatToggleAction);
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, Action_Exit);
		GetGame().GetInputManager().RemoveActionListener("VONDirect", EActionTrigger.DOWN, Action_VONon);
		GetGame().GetInputManager().RemoveActionListener("VONDirect", EActionTrigger.UP, Action_VONOff);
	}

	//From RL
	RadioTransceiver GetVoNTransiver()
	{
		IEntity entity = GetGame().GetPlayerController().GetControlledEntity();
		ref array<IEntity> items = {};
		SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent)).GetItems(items);
		IEntity radioEntity;
		foreach(IEntity item: items)
		{
			if(item.FindComponent(BaseRadioComponent))
				radioEntity = item;
		}
		BaseRadioComponent radio = BaseRadioComponent.Cast(radioEntity.FindComponent(BaseRadioComponent));
		radio.SetPower(true);
		RadioTransceiver transiver = RadioTransceiver.Cast(radio.GetTransceiver(0));
		transiver.SetFrequency(1);
		return transiver;
	}
	
	void Action_VONon()
	{
		GetGame().GetCallqueue().Remove(LobbyVoNDisableDelayed);
		SCR_VoNComponent von = SCR_VoNComponent.Cast(GetGame().GetPlayerController().GetControlledEntity().FindComponent(SCR_VoNComponent));
		von.SetTransmitRadio(GetVoNTransiver());
		von.SetCommMethod(ECommMethod.SQUAD_RADIO);
		von.SetCapture(true);
	}
	
	//From reforger lobby <3
	void Action_VONOff()
	{
		GetGame().GetCallqueue().CallLater(LobbyVoNDisableDelayed, 400);
	}
	
	void LobbyVoNDisableDelayed()
	{
		SCR_VoNComponent von = SCR_VoNComponent.Cast(GetGame().GetPlayerController().GetControlledEntity().FindComponent(SCR_VoNComponent));
		von.SetCommMethod(ECommMethod.DIRECT);
		von.SetCapture(false);
	}
	
	void UpdateIcons()
	{
		foreach(CRF_SpectatorLabelIconCharacter spectatorIcon: m_aSpectatorIcons)
		{
			spectatorIcon.Update();
		}
	}
	
	void Action_OnChatToggleAction()
	{
		if (!m_ChatPanel)
			return;
		
		// Frame delay
		GetGame().GetCallqueue().CallLater(OpenChatWrap, 5);
	}
	
	void OpenChatWrap()
	{
		if (!m_ChatPanel.IsOpen())
		{
			SCR_ChatPanelManager.GetInstance().OpenChatPanel(m_ChatPanel);
		}
	}
	void Action_Exit()
	{
		// For some strange reason players all the time accidentally exit game, maybe jus open pause menu
		//GameStateTransitions.RequestGameplayEndTransition();  
		//Close();
		GetGame().GetCallqueue().CallLater(OpenPauseMenuWrap, 0); //  Else menu auto close itself
	}
	void OpenPauseMenuWrap()
	{
		ArmaReforgerScripted.OpenPauseMenu();
	}
} 