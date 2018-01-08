//	Tactical.js
//
//	Tactical display, including combat
//	Copyright © 2011 by Kronosaur Productions, LLC. All Rights Reserved.

var xLow = -PARTICLE_SIZE;
var xHigh = 1024;
var yLow = -PARTICLE_SIZE;
var yHigh = 768;

var TACTICAL_SCALE = 100000.0;		//	Meters per pixel
var CENTER_X = 1024 / 2;
var CENTER_Y = 768 / 2;

function CTacticalGroup (x, y, xV, yV)
	{
	this.orbit = new COrbit(
			1.0,
			x,
			y,
			xV,
			yV
			);
			
	this.Draw = function ()
		{
		ctx.fillRect(X2Screen(this.orbit.x) - PARTICLE_SIZE / 2, Y2Screen(this.orbit.y) - PARTICLE_SIZE / 2, PARTICLE_SIZE, PARTICLE_SIZE);
		}
		
	this.Update = function ()
		{
		this.orbit.Update();
		}
	};