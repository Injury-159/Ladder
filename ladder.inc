#if defined _inc_gameplay_
	#undef _inc_gameplay_
#endif


#include <YSI_Coding\y_hooks>


hook OnPlayerDisconnect(playerid, reason)
{
	playerLadderTick[playerid] = 0;
	playerExitingLadder{playerid} = false;
	playerLadderID[playerid] = INVALID;
}


hook OnPlayerStateChange(playerid, newstate, oldstate)
{
	playerLadderID[playerid] = INVALID;
	playerLadderTick[playerid] = 0;
	playerExitingLadder{playerid} = false;
}


hook OnPlayerDeath(playerid)
{
	playerLadderID[playerid] = INVALID;
	playerLadderTick[playerid] = 0;
	playerExitingLadder{playerid} = false;
}


hook OnPlayerUpdate(playerid)
{
	if(playerLadderID[playerid] < 0)
		return 1;


	if(!playerExitingLadder{playerid})
	{
		static unused;
		new updown;
		GetPlayerKeys(playerid, unused, updown, unused);		

		if(updown < 0)
		{
			SetPlayerVelocity(playerid, 0.0, 0.0, 0.07);
		}
		else if(updown > 0)
		{
			SetPlayerVelocity(playerid, 0.0, 0.0, -0.07);
		}
		else 
		{
			SetPlayerVelocity(playerid, 0.0, 0.0, 0.020);
		}
		ApplyAnimation(playerid, "PED", "FIGHTIDLE", 4.1, false, false, false, false, 0, true);

		_CheckForExitLadder(playerid);
	}

	return 1;
}


hook OnPlayerEnterDynZone(playerid, areaid, bool:onfoot)
{
	if(onfoot && !playerExitingLadder{playerid} && gettime() > playerLadderTick[playerid])
	{
		new data[2];
		Streamer_GetArrayData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID, data);

		if(data[0] == LADDER_EXTRA_ID)
		{
			ClearAnimations(playerid);

			new
				Float:x, 
				Float:y, 
				Float:minz, 
				Float:maxz, 
				Float:pz,
				Float:angle = Ladder[data[1]][ldAngle]
			;

			Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_X, x);
			Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_Y, y);
			Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_MAX_Z, maxz);
			Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_MIN_Z, minz);
							
			GetPlayerPos(playerid, pz, pz, pz);


			if(floatabs(maxz - pz) <= 0.3)
			{
				SetPlayerPos(playerid, x, y, maxz - 1.4);
			}
			else if(pz - minz <= 0.3)
			{
				SetPlayerPos(playerid, x, y, minz + 0.4);
			}
			else 
			{
				SetPlayerPos(playerid, x, y, pz);
			}

			SetPlayerFacingAngle(playerid, angle);

			playerLadderID[playerid] = data[1];
		}	
	}
}


static timer _FinishLadderExit[350](playerid, Float:angle, Float:x, Float:y, Float:z)
{
	GetPointInFront2D(x, y, angle, 1.05, x, y);
	SetPlayerPos(playerid, x, y, z + 0.3);
	SetPlayerFacingAngle(playerid, angle);

	playerExitingLadder{playerid} = false;
	playerLadderID[playerid] = INVALID;

	TogglePlayerControllable(playerid, true);
}


static _CheckForExitLadder(playerid)
{
	if(gettime() < playerLadderTick[playerid])
		return;

	new Float:z, Float:minz, Float:maxz;
	new areaid = Ladder[playerLadderID[playerid]][ldCylinder];

	GetPlayerPos(playerid, z, z, z);
	Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_MAX_Z, maxz);
	Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_MIN_Z, minz);

	if(floatabs(z - minz) < 0.35)
	{
		playerLadderID[playerid] = INVALID;
		ClearAnimations(playerid);
	}
	
	else if(z >= (maxz - 0.11))
	{
		new Float:px, Float:py;
		Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_X, px);
		Streamer_GetFloatData(STREAMER_TYPE_AREA, areaid, E_STREAMER_Y, py);

		playerLadderTick[playerid] = gettime() + 1;
		playerExitingLadder{playerid} = true;

		TogglePlayerControllable(playerid, false);

		ApplyAnimation(playerid, "PED", "CLIMB_STAND", 3.1, false, false, false, true, 0, true);
		defer _FinishLadderExit(playerid, Ladder[playerLadderID[playerid]][ldAngle], px, py, maxz);
	}
}


stock CreateLadder(Float:angle, Float:x, Float:y, Float:minz, Float:maxz, worldid = -1, interiorid = 0)
{
	if(minz > maxz)
		return INVALID;

	if(maxz - minz < 4.0)
		return INVALID;

	new idx;
	if((idx = Iter_Alloc(Ladders)) > -1)
	{
		new data[2];
		data[0] = LADDER_EXTRA_ID;
		data[1] = idx;

		new cylinder = CreateDynamicCylinder(x, y, minz, maxz, 0.9, worldid, interiorid);
		Streamer_SetArrayData(STREAMER_TYPE_AREA, cylinder, E_STREAMER_EXTRA_ID, data);

		Ladder[idx][ldCylinder] = cylinder;
		Ladder[idx][ldAngle] = angle;
		return idx;
	}

	return INVALID;
}

CMD:ladder(playerid)
{
	CreateLadder(45.0, -1115.7736, 335.3625, 1.9609, 13.1);
	SetPlayerPos(playerid, -1118.0087, 330.5699, 1.9609);
	return 1;
}

