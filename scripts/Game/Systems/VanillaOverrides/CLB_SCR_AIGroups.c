modded class SCR_AIGroup
{
	[Attribute("0", category: "Group")]
	bool m_bIsPlayable;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		if(Replication.IsServer() && m_bIsPlayable)
			GetGame().GetCallqueue().CallLater(SaveAIGRoup, 500, false);
	}
	
	//Saves the group on the server
	void SaveAIGRoup()
	{
		CLB_Gamemode.GetInstance().AddGroup(this);
	}
	
	override void OnEmpty()
	{
		Event_OnEmpty.Invoke(this);
		
		//--- Delete after delay, doing it directly in this event would be unsafe
//		if (m_bDeleteWhenEmpty)
//			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1, false, this);		
	}
}