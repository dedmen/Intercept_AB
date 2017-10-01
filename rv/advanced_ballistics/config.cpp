class CfgPatches
{
	class ace_advanced_ballistics
	{
		name = "ACE3 - Advanced Ballistics";
		units[] = {"ace_advanced_ballistics_ModuleSettings"};
		weapons[] = {};
		requiredVersion = 1.76;
		requiredAddons[] = {"ace_ballistics","ace_weather"};
		author = "$STR_ace_common_ACETeam";
		authors[] = {"Ruthberg"};
		url = "$STR_ace_main_URL";
		version = "3.10.2.22";
		versionStr = "3.10.2.22";
		versionAr[] = {3,10,2,22};
	};
};
class Extended_PreStart_EventHandlers
{
	class ace_advanced_ballistics
	{
		init = "call compile preProcessFileLineNumbers '\z\ace\addons\advanced_ballistics\XEH_preStart.sqf'";
	};
};
class Extended_PreInit_EventHandlers
{
	class ace_advanced_ballistics
	{
		init = "call compile preProcessFileLineNumbers '\z\ace\addons\advanced_ballistics\XEH_preInit.sqf'";
	};
};
class Extended_PostInit_EventHandlers
{
	class ace_advanced_ballistics
	{
		init = "call compile preProcessFileLineNumbers '\z\ace\addons\advanced_ballistics\XEH_postInit.sqf'";
	};
};
class CfgVehicles
{
	class ACE_Module;
	class ace_advanced_ballistics_ModuleSettings: ACE_Module
	{
		scope = 2;
		displayName = "$STR_ace_advanced_ballistics_DisplayName";
		icon = "\z\ace\addons\advanced_ballistics\UI\Icon_Module_Wind_ca.paa";
		category = "ACE";
		function = "ace_advanced_ballistics_fnc_initModuleSettings";
		functionPriority = 1;
		isGlobal = 1;
		isTriggerActivated = 0;
		author = "Ruthberg";
		class Arguments
		{
			class enabled
			{
				displayName = "$STR_ace_advanced_ballistics_enabled_DisplayName";
				description = "$STR_ace_advanced_ballistics_enabled_Description";
				typeName = "BOOL";
				defaultValue = 0;
			};
			class simulateForSnipers
			{
				displayName = "$STR_ace_advanced_ballistics_simulateForSnipers_DisplayName";
				description = "$STR_ace_advanced_ballistics_simulateForSnipers_Description";
				typeName = "BOOL";
				defaultValue = 1;
			};
			class simulateForGroupMembers
			{
				displayName = "$STR_ace_advanced_ballistics_simulateForGroupMembers_DisplayName";
				description = "$STR_ace_advanced_ballistics_simulateForGroupMembers_Description";
				typeName = "BOOL";
				defaultValue = 0;
			};
			class simulateForEveryone
			{
				displayName = "$STR_ace_advanced_ballistics_simulateForEveryone_DisplayName";
				description = "$STR_ace_advanced_ballistics_simulateForEveryone_Description";
				typeName = "BOOL";
				defaultValue = 0;
			};
			class disabledInFullAutoMode
			{
				displayName = "$STR_ace_advanced_ballistics_disabledInFullAutoMod_DisplayName";
				description = "$STR_ace_advanced_ballistics_disabledInFullAutoMod_Description";
				typeName = "BOOL";
				defaultValue = 0;
			};
			class ammoTemperatureEnabled
			{
				displayName = "$STR_ace_advanced_ballistics_ammoTemperatureEnabled_DisplayName";
				description = "$STR_ace_advanced_ballistics_ammoTemperatureEnabled_Description";
				typeName = "BOOL";
				defaultValue = 1;
			};
			class barrelLengthInfluenceEnabled
			{
				displayName = "$STR_ace_advanced_ballistics_barrelLengthInfluenceEnabled_DisplayName";
				description = "$STR_ace_advanced_ballistics_barrelLengthInfluenceEnabled_Description";
				typeName = "BOOL";
				defaultValue = 1;
			};
			class bulletTraceEnabled
			{
				displayName = "$STR_ace_advanced_ballistics_bulletTraceEnabled_DisplayName";
				description = "$STR_ace_advanced_ballistics_bulletTraceEnabled_Description";
				typeName = "BOOL";
				defaultValue = 1;
			};
			class simulationInterval
			{
				displayName = "$STR_ace_advanced_ballistics_simulationInterval_DisplayName";
				description = "$STR_ace_advanced_ballistics_simulationInterval_Description";
				typeName = "NUMBER";
				defaultValue = 0.05;
			};
			class simulationRadius
			{
				displayName = "$STR_ace_advanced_ballistics_simulationRadius_DisplayName";
				description = "$STR_ace_advanced_ballistics_simulationRadius_Description";
				typeName = "NUMBER";
				defaultValue = 3000;
			};
		};
		class ModuleDescription
		{
			description = "$STR_ace_advanced_ballistics_Description";
		};
	};
};
class RscTitles
{
	class RscTurretDial
	{
		idd = -1;
		onLoad = "with uiNameSpace do { RscTurretDial = _this select 0 };";
		movingEnable = 0;
		duration = 5;
		fadeIn = "false";
		fadeOut = "false";
		class controls
		{
			class RscTurretDial
			{
				idc = 132949;
				type = 0;
				style = 128;
				font = "TahomaB";
				colorBackground[] = {0,0,0,0.8};
				colorText[] = {1,1,1,1};
				x = "SafeZoneX + 0.0025";
				y = "SafeZoneY + 0.0025";
				w = 0.1;
				h = 0.05;
				sizeEx = 0.03;
				text = "";
			};
		};
	};
	class RscProtractor
	{
		idd = -1;
		onLoad = "with uiNameSpace do { RscProtractor = _this select 0 };";
		movingEnable = 0;
		duration = 60;
		fadeIn = "false";
		fadeOut = "false";
		class controls
		{
			class RscProtractorBase
			{
				idc = 132950;
				type = 0;
				style = 48;
				font = "TahomaB";
				colorBackground[] = {0,0,0,0};
				colorText[] = {1,1,1,1};
				x = "SafeZoneX + 0.001";
				y = "SafeZoneY + 0.001";
				w = 0.2;
				h = "0.2*4/3";
				size = 0.034;
				sizeEx = 0.027;
				text = "";
			};
			class RscProtractorMarker: RscProtractorBase
			{
				idc = 132951;
			};
		};
	};
};
class ACE_Settings
{
	class ace_advanced_ballistics_enabled
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_enabled_DisplayName";
		description = "$STR_ace_advanced_ballistics_enabled_Description";
		typeName = "BOOL";
		value = 0;
	};
	class ace_advanced_ballistics_simulateForSnipers
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_simulateForSnipers_DisplayName";
		description = "$STR_ace_advanced_ballistics_simulateForSnipers_Description";
		typeName = "BOOL";
		value = 1;
	};
	class ace_advanced_ballistics_simulateForGroupMembers
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_simulateForGroupMembers_DisplayName";
		description = "$STR_ace_advanced_ballistics_simulateForGroupMembers_Description";
		typeName = "BOOL";
		value = 0;
	};
	class ace_advanced_ballistics_simulateForEveryone
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_simulateForEveryone_DisplayName";
		description = "$STR_ace_advanced_ballistics_simulateForEveryone_Description";
		typeName = "BOOL";
		value = 0;
	};
	class ace_advanced_ballistics_disabledInFullAutoMode
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_disabledInFullAutoMod_DisplayName";
		description = "$STR_ace_advanced_ballistics_disabledInFullAutoMod_Description";
		typeName = "BOOL";
		value = 0;
	};
	class ace_advanced_ballistics_ammoTemperatureEnabled
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_ammoTemperatureEnabled_DisplayName";
		description = "$STR_ace_advanced_ballistics_ammoTemperatureEnabled_Description";
		typeName = "BOOL";
		value = 1;
	};
	class ace_advanced_ballistics_barrelLengthInfluenceEnabled
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_barrelLengthInfluenceEnabled_DisplayName";
		description = "$STR_ace_advanced_ballistics_barrelLengthInfluenceEnabled_Description";
		typeName = "BOOL";
		value = 1;
	};
	class ace_advanced_ballistics_bulletTraceEnabled
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_bulletTraceEnabled_DisplayName";
		description = "$STR_ace_advanced_ballistics_bulletTraceEnabled_Description";
		typeName = "BOOL";
		value = 1;
	};
	class ace_advanced_ballistics_simulationInterval
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_simulationInterval_DisplayName";
		description = "$STR_ace_advanced_ballistics_simulationInterval_Description";
		typeName = "SCALAR";
		value = 0.05;
	};
	class ace_advanced_ballistics_simulationRadius
	{
		category = "$STR_ace_advanced_ballistics_DisplayName";
		displayName = "$STR_ace_advanced_ballistics_simulationRadius_DisplayName";
		description = "$STR_ace_advanced_ballistics_simulationRadius_Description";
		typeName = "SCALAR";
		value = 3000;
	};
};
class ACE_Extensions
{
	extensions[] += {"ace_advanced_ballistics"};
};

class Intercept {
    class ACE {
        class advanced_ballistics {
            pluginName = "advanced_ballistics";
        };
    };
};