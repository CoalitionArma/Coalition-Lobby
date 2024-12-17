class CRF_PlayableCharacterClass: ScriptComponentClass
{
}

class CRF_PlayableCharacter : ScriptComponent
{
	[Attribute()]
	protected string m_sName;
	[Attribute()]
	protected bool m_bIsPlayable;
	[Attribute()]
	protected bool m_bIsLeaderOrMedic;
	[Attribute()]
	protected bool m_bIsSpecialty;
	
	bool IsPlayable()
	{
		return m_bIsPlayable;
	}
	
	string GetName()
	{
		return m_sName;
	}
	
	bool IsLeader()
	{
		return m_bIsLeaderOrMedic;
	}
	
	bool IsSpecialty()
	{
		return m_bIsSpecialty;
	}
	
		override void OnPostInit(IEntity owner)
	{
		super.EOnInit(owner);
		GetGame().GetCallqueue().CallLater(SetInitialEntity, 500, false, owner);
	}
	
	void SetInitialEntity(IEntity owner)
	{
		//Logs entity on server and disables AI
		if(CRF_PlayableCharacter.Cast(owner.FindComponent(CRF_PlayableCharacter)).IsPlayable() && Replication.IsServer())
		{
			CRF_Gamemode.GetInstance().AddPlayableEntity(owner);
			AIControlComponent.Cast(owner.FindComponent(AIControlComponent)).GetAIAgent().DeactivateAI();			
			Print(AIControlComponent.Cast(owner.FindComponent(AIControlComponent)).GetAIAgent().IsAIActivated());
		}
			
		//Sets location and all the physics BS on all machines
		if(owner.GetPrefabData().GetPrefabName() == "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
		{
			owner.GetPhysics().EnableGravity(false);
			owner.SetOrigin("0 10000 0");
			Physics physics = owner.GetPhysics();
			if (physics)
			{
				physics.SetVelocity("0 0 0");
				physics.SetAngularVelocity("0 0 0");
				physics.SetMass(0);
				physics.SetDamping(1, 1);
				physics.SetActive(ActiveState.INACTIVE);
			}
		}	
	}
}