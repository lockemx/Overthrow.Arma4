class OVT_OverthrowConfigComponentClass: OVT_ComponentClass
{
};



enum OVT_FactionType {
	OCCUPYING_FACTION,
	RESISTANCE_FACTION,
	SUPPORTING_FACTION
}

enum OVT_PatrolType {
	DEFEND,
	PERIMETER
}

class OVT_CameraPosition : ScriptAndConfig
{
	[Attribute("0 0 0", UIWidgets.Coords)]
	vector position;

	[Attribute("0 0 0", UIWidgets.Coords)]
	vector angles;
}

class OVT_OverthrowConfigStruct
{
	string occupyingFaction;
	string discordWebHookURL;
	ref array<string> officers;
	void SetDefaults()
	{
		discordWebHookURL = "see wiki: https://github.com/ArmaOverthrow/Overthrow.Arma4/wiki/Discord-Web-Hook";
		occupyingFaction = "";
		officers = new array<string>;
	}
}

class OVT_OverthrowConfigComponent: OVT_Component
{
	[Attribute("$profile:Overthrow_Config.json")]
	string m_sConfigFilePath;

	ref OVT_OverthrowConfigStruct m_ConfigFile;

	[Attribute( defvalue: "FIA", uiwidget: UIWidgets.EditBox, desc: "Faction affiliation of the player's side", category: "Factions")]
	string m_sPlayerFaction;

	[Attribute( defvalue: "USSR", uiwidget: UIWidgets.EditBox, desc: "The faction occupying this map (the enemy)", category: "Factions")]
	string m_sDefaultOccupyingFaction;

	string m_sOccupyingFaction = "USSR";

	[Attribute( defvalue: "US", uiwidget: UIWidgets.EditBox, desc: "The faction supporting the player", category: "Factions")]
	string m_sSupportingFaction;

	[Attribute("", UIWidgets.Object)]
	ref array<ref OVT_CameraPosition> m_aCameraPositions;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Town Controller Prefab", params: "et", category: "Controllers")]
	ResourceName m_pTownControllerPrefab;

	[Attribute("", UIWidgets.Object)]
	ref OVT_DifficultySettings m_Difficulty;

	[Attribute("", UIWidgets.Object)]
	ref array<ref OVT_DifficultySettings> m_aDifficultyPresets;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Gun Dealer Prefab", params: "et")]
	ResourceName m_pGunDealerPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Civilian Prefab", params: "et")]
	ResourceName m_pCivilianPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Move Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pMoveWaypointPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Defend Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pDefendWaypointPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Patrol Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pPatrolWaypointPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Wait Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pWaitWaypointPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Cycle Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pCycleWaypointPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Search and Destroy Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pSearchAndDestroyWaypointPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Get In Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pGetInWaypointPrefab;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Smart Action Waypoint Prefab", params: "et", category: "Waypoints")]
	ResourceName m_pSmartActionWaypointPrefab;

	[Attribute(desc: "Starting Houses (these should have parking spot entities added to their prefabs)", params: "et", category: "Real Estate")]
	ref array<string> m_aStartingHouseFilters;

	[Attribute(desc: "Real estate configs to set prices and rents for building types", category: "Real Estate", UIWidgets.Object)]
	ref array<ref OVT_RealEstateConfig> m_aBuildingTypes;

	[Attribute(defvalue: "0.1", UIWidgets.EditBox, desc: "Civilians to spawn per population")]
	float m_fCivilianSpawnRate;

	[Attribute(defvalue: "1000", UIWidgets.EditBox, desc: "Civilian spawn distance")]
	int m_iCivilianSpawnDistance;

	[Attribute(defvalue: "1750", UIWidgets.EditBox, desc: "Military spawn distance")]
	int m_iMilitarySpawnDistance;

	[Attribute(defvalue: "0.1", UIWidgets.EditBox, desc: "NPC Shop Buy Rate", category: "Economy")]
	float m_fNPCBuyRate;

	[Attribute(defvalue: "0.25", UIWidgets.EditBox, desc: "Shop Profit Margin", category: "Economy")]
	float m_fShopProfitMargin;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Player Group Prefab", params: "et")]
	ResourceName m_pPlayerGroupPrefab;

	[Attribute( uiwidget: UIWidgets.Object, desc: "Civilian Loadout" )]
	ref OVT_LoadoutConfig m_CivilianLoadout;

	int m_iOccupyingFactionIndex = -1;
	int m_iPlayerFactionIndex = -1;

	[Attribute(defvalue: "false", UIWidgets.EditBox, desc: "Debug Mode")]
	bool m_bDebugMode;

	// Instance of this component
	private static OVT_OverthrowConfigComponent s_Instance = null;

	static OVT_OverthrowConfigComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode pGameMode = GetGame().GetGameMode();
			if (pGameMode)
				s_Instance = OVT_OverthrowConfigComponent.Cast(pGameMode.FindComponent(OVT_OverthrowConfigComponent));
		}

		return s_Instance;
	}

	bool LoadConfig()
	{
		Print("Overthrow: Trying to load configuration file "+m_sConfigFilePath, LogLevel.NORMAL);

#ifdef PLATFORM_XBOX
		m_ConfigFile = new OVT_OverthrowConfigStruct();
		m_ConfigFile.SetDefaults();
		return true;
#endif

		SCR_JsonLoadContext configLoadContext = new SCR_JsonLoadContext();

		if (!FileIO.FileExists( m_sConfigFilePath ))
		{
			Print("Overthrow: Configuration file does not exist. Creating new one.", LogLevel.WARNING);
			m_ConfigFile = new OVT_OverthrowConfigStruct();
			m_ConfigFile.SetDefaults();

			SaveConfig();
		};

		if (!configLoadContext.LoadFromFile( m_sConfigFilePath ))
		{
			Print("Overthrow: Configuration load failed", LogLevel.ERROR);
			return false;
		};

		if (!configLoadContext.ReadValue("", m_ConfigFile))
		{
			Print("Overthrow: Configuration load failed", LogLevel.ERROR);
			return false;
		};

		return true;
	};

	bool SaveConfig()
	{
		SCR_JsonSaveContext configSaveContext = new SCR_JsonSaveContext();
		configSaveContext.WriteValue("", m_ConfigFile);

		if (!configSaveContext.SaveToFile( m_sConfigFilePath ))
		{
			Print("Overthrow: Saving config file failed!", LogLevel.ERROR);
			return false;
		};

		return true;
	};

	int GetPlaceableCost(OVT_Placeable placeable)
	{
		return Math.Round(m_Difficulty.placeableCostMultiplier * placeable.m_iCost);
	}

	int GetBuildableCost(OVT_Buildable buildable)
	{
		return Math.Round(m_Difficulty.buildableCostMultiplier * buildable.m_iCost);
	}

	void SetOccupyingFaction(string key)
	{
		OVT_Faction of = GetOccupyingFaction();
		if(key == of.GetFactionKey()) return;
		FactionManager factionMgr = GetGame().GetFactionManager();
		Faction faction = factionMgr.GetFactionByKey(key);
		m_iOccupyingFactionIndex = factionMgr.GetFactionIndex(faction);

		m_sOccupyingFaction = faction.GetFactionKey();
	}

	void SetBaseAndTownOwners()
	{
		foreach(OVT_BaseData base : OVT_Global.GetOccupyingFaction().m_Bases)
		{
			base.faction = m_iOccupyingFactionIndex;
		}

		foreach(OVT_TownData town : OVT_Global.GetTowns().m_Towns)
		{
			town.faction = m_iOccupyingFactionIndex;
		}

		foreach(OVT_RadioTowerData tower : OVT_Global.GetOccupyingFaction().m_RadioTowers)
		{
			tower.faction = m_iOccupyingFactionIndex;
		}
	}

	OVT_Faction GetOccupyingFaction()
	{
		return OVT_Global.GetFactions().GetOverthrowFactionByKey(m_sOccupyingFaction);
	}

	Faction GetOccupyingFactionData()
	{
		return GetGame().GetFactionManager().GetFactionByKey(m_sOccupyingFaction);
	}

	int GetOccupyingFactionIndex()
	{
		if(m_iOccupyingFactionIndex == -1)
		{
			FactionManager fm = GetGame().GetFactionManager();
			m_iOccupyingFactionIndex = fm.GetFactionIndex(fm.GetFactionByKey(m_sOccupyingFaction));
		}
		return m_iOccupyingFactionIndex;
	}
	
	string GetOccupyingFactionKey()
	{		
		return m_sOccupyingFaction;
	}

	OVT_Faction GetPlayerFaction()
	{
		return OVT_Global.GetFactions().GetOverthrowFactionByKey(m_sPlayerFaction);
	}

	Faction GetPlayerFactionData()
	{
		return GetGame().GetFactionManager().GetFactionByKey(m_sPlayerFaction);
	}

	int GetPlayerFactionIndex()
	{
		if(m_iPlayerFactionIndex == -1)
		{
			FactionManager fm = GetGame().GetFactionManager();
			m_iPlayerFactionIndex = fm.GetFactionIndex(fm.GetFactionByKey(m_sPlayerFaction));
		}
		return m_iPlayerFactionIndex;
	}

	OVT_Faction GetFactionByType(OVT_FactionType type)
	{
		switch(type)
		{
			case OVT_FactionType.OCCUPYING_FACTION:
				return GetOccupyingFaction();
		}
		return GetPlayerFaction();
	}

	AIWaypoint SpawnWaypoint(ResourceName res, vector pos)
	{
		AIWaypoint wp = AIWaypoint.Cast(OVT_Global.SpawnEntityPrefab(res, pos));
		return wp;
	}

	AIWaypoint SpawnPatrolWaypoint(vector pos)
	{
		AIWaypoint wp = SpawnWaypoint(m_pPatrolWaypointPrefab, pos);
		return wp;
	}

	AIWaypoint SpawnSearchAndDestroyWaypoint(vector pos)
	{
		AIWaypoint wp = SpawnWaypoint(m_pSearchAndDestroyWaypointPrefab, pos);
		return wp;
	}

	AIWaypoint SpawnDefendWaypoint(vector pos, int preset = 0)
	{
		AIWaypoint wp = SpawnWaypoint(m_pDefendWaypointPrefab, pos);
		SCR_DefendWaypoint defend = SCR_DefendWaypoint.Cast(wp);
		defend.SetCurrentDefendPreset(preset);
		return wp;
	}

	SCR_EntityWaypoint SpawnGetInWaypoint(IEntity target)
	{
		SCR_EntityWaypoint wp = SCR_EntityWaypoint.Cast(OVT_Global.SpawnEntityPrefab(m_pGetInWaypointPrefab, target.GetOrigin()));

		wp.SetEntity(target);

		return wp;
	}

	SCR_TimedWaypoint SpawnWaitWaypoint(vector pos, float time)
	{
		SCR_TimedWaypoint wp = SCR_TimedWaypoint.Cast(OVT_Global.SpawnEntityPrefab(m_pWaitWaypointPrefab, pos));

		return wp;
	}

	SCR_SmartActionWaypoint SpawnActionWaypoint(vector pos, IEntity target, string action)
	{
		SCR_SmartActionWaypoint wp = SCR_SmartActionWaypoint.Cast(OVT_Global.SpawnEntityPrefab(m_pSmartActionWaypointPrefab, pos));

		wp.SetSmartActionEntity(target, action);

		return wp;
	}

	void GivePatrolWaypoints(SCR_AIGroup aigroup, OVT_PatrolType type, vector center = "0 0 0")
	{
		if(center[0] == 0) center = aigroup.GetOrigin();

		if(type == OVT_PatrolType.DEFEND)
		{
			aigroup.AddWaypoint(SpawnDefendWaypoint(center));
			return;
		}

		if(type == OVT_PatrolType.PERIMETER)
		{
			float dist = vector.Distance(aigroup.GetOrigin(), center);
			vector dir = vector.Direction(aigroup.GetOrigin(), center);
			float angle = dir.VectorToAngles()[1];

			array<AIWaypoint> queueOfWaypoints = new array<AIWaypoint>();
			AIWaypoint firstWP;
			for(int i=0; i< 4; i++)
			{
				vector pos = center + (Vector(0,angle,0).AnglesToVector() * dist);

				AIWaypoint wp = SpawnPatrolWaypoint(pos);
				queueOfWaypoints.Insert(wp);

				AIWaypoint wait = SpawnWaitWaypoint(pos, s_AIRandomGenerator.RandFloatXY(45, 75));
				queueOfWaypoints.Insert(wait);

				angle += 90;
				if(angle > 359) angle -= 360;
			}
			AIWaypointCycle cycle = AIWaypointCycle.Cast(SpawnWaypoint(m_pCycleWaypointPrefab, aigroup.GetOrigin()));
			cycle.SetWaypoints(queueOfWaypoints);
			cycle.SetRerunCounter(-1);
			aigroup.AddWaypoint(cycle);
			return;
		}
	}
}
