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
		CRF_Gamemode.GetInstance().AddGroup(this);
	}
}