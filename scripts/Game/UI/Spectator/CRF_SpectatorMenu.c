modded enum ChimeraMenuPreset : ScriptMenuPresetEnum
{
	CRF_SpectatorMenu
}

class CRF_SpectatorMenuUI: ChimeraMenuBase
{
	ref array<IEntity> m_aEntityIcons = {};
	ref array<ref CRF_SpectatorLabelIconCharacter> m_aSpectatorIcons = {};
	CRF_Gamemode m_Gamemode;
	
	override void OnMenuOpen()
	{
		m_Gamemode = CRF_Gamemode.GetInstance();
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
	}
	
	void UpdateIcons()
	{
		foreach(CRF_SpectatorLabelIconCharacter spectatorIcon: m_aSpectatorIcons)
		{
			spectatorIcon.Update();
		}
	}
} 