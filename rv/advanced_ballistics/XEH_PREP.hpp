
PREP(calculateAmmoTemperatureVelocityShift);
PREP(calculateAtmosphericCorrection);
PREP(calculateBarrelLengthVelocityShift);
PREP(calculateRetardation);
PREP(calculateStabilityFactor);
PREP(diagnoseWeapons);
PREP(displayProtractor);

if (isNull (uiNamespace getVariable ["", scriptNull])) then {
    PREP(handleFired);
    PREP(handleFirePFH);
} else {
	
};


PREP(initializeTerrainExtension);
PREP(initModuleSettings);
PREP(readAmmoDataFromConfig);
PREP(readWeaponDataFromConfig);