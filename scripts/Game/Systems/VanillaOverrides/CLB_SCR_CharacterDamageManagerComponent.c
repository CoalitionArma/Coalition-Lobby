modded class SCR_CharacterDamageManagerComponent
{
	//Logs death of a playable character on server
	override void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		super.OnLifeStateChanged(previousLifeState, newLifeState);
		if(newLifeState == 10 && CLB_PlayableCharacter.Cast(GetOwner().FindComponent(CLB_PlayableCharacter)).IsPlayable())
			CLB_Gamemode.GetInstance().SetDeathState(GetOwner(), true);
	}
}