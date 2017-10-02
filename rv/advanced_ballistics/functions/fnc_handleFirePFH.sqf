/*
 * Author: Glowbal, Ruthberg, joko // Jonas
 * Handle the PFH for Bullets
 *
 * Arguments:
 * None
 *
 * Return Value:
 * None
 *
 * Example:
 * [] call ace_advanced_ballistics_fnc_handleFirePFH
 *
 * Public: No
 */
#include "script_component.hpp"

private _aceTimeSecond = floor CBA_missionTime;

if (isNil QGVAR(lastFrameTime)) then {
    GVAR(lastFrameTime) = diag_tickTime;
};
toFixed 8;

isNil {"allBullets";
{
    _x params ["_bullet","_caliber","_bulletTraceVisible","_index"];

    private _bulletVelocity = velocity _bullet;

    private _bulletSpeed = vectorMagnitude _bulletVelocity;
	advancedBallisticsLOG _bulletSpeed;
   if (isNil QGVAR(lastBulletSpeed)) then {
       GVAR(lastBulletSpeed) = _bulletSpeed;
   };
    if (!alive _bullet || _bulletSpeed < 100) then {
        GVAR(allBullets) deleteAt (GVAR(allBullets) find _x);
    } else {
        private _bulletPosition = getPosASL _bullet;

        if (_bulletTraceVisible && _bulletSpeed > 500) then {
            drop ["\A3\data_f\ParticleEffects\Universal\Refract","","Billboard",1,0.1,getPos _bullet,[0,0,0],0,1.275,1,0,[0.02*_caliber,0.01*_caliber],[[0,0,0,0.65],[0,0,0,0.2]],[1,0],0,0,"","",""];
        };

		_dt = advancedBallisticsGetDeltaT;//(diag_tickTime - GVAR(lastFrameTime));
		//advancedBallisticsLOG _dt;
        _avgSpeed = GVAR(lastBulletSpeed);
        _dragRef = GVAR(airFriction) * _avgSpeed * _avgSpeed;
        _accel = (vectorNormalized _bulletVelocity) vectorMultiply abs(_dragRef);
        _bulletVelocity = _bulletVelocity vectorAdd (_accel vectorMultiply _dt);
        _bulletVelocity = _bulletVelocity vectorAdd ([0, 0, 9.8066] vectorMultiply _dt);

		
		
		
		isNil {
			//_bullet advancedBallisticsSimulate [_index,_bulletVelocity, _bulletPosition, ACE_wind, ASLToATL(_bulletPosition) select 2, _aceTimeSecond, CBA_missionTime - _aceTimeSecond];
			//_bullet setVelocity (_bulletVelocity vectorAdd (parseSimpleArray 
				//("ace_advanced_ballistics" callExtension format["simulate:%1:%2:%3:%4:%5:%6:%7", _index, _bulletVelocity, _bulletPosition, ACE_wind, ASLToATL(_bulletPosition) select 2, _aceTimeSecond, CBA_missionTime - _aceTimeSecond])
			//));
			true;
		};
		
		
		_bullet setVelocity _bulletVelocity;
		_dt advancedBallisticsLOG2 _bulletVelocity;
		GVAR(lastFrameTime) = diag_tickTime;
        //GVAR(lastBulletSpeed) = _bulletSpeed;
		GVAR(lastBulletSpeed) = vectorMagnitude _bulletVelocity;
				
    };
    nil
} count +GVAR(allBullets);
};
if (GVAR(allBullets) isEqualTo []) then {
    [_this select 1] call CBA_fnc_removePerFrameHandler;
    GVAR(BulletPFH) = nil;
	    GVAR(lastFrameTime) = nil;
    GVAR(lastBulletSpeed) = nil;
};
