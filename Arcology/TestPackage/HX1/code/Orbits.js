//	Orbits.js
//
//	Orbit objects
//	Copyright © 2011 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	Mass is in Earth masses
//	Distance is in meters
//	Time is in seconds

var G_CONSTANT = 3.9868e14;			//	Gravitational constant, scaled to Earth-masses
var UPDATE_TIME = 12.2;				//	Seconds of game time per frame of animation
var TACTICAL_SCALE = 100000.0;		//	Meters per pixel

function COrbit (mass, x, y, xV, yV)

//	x and y are the position of the object in Cartessian coordinates with the
//	planetary body at 0,0.
//
//	xV and yV are the velocity of the object.

	{
	this.mass = mass;
	this.x = x;
	this.y = y;
	this.xV = xV;
	this.yV = yV;
	
	this.Update = function ()
		{
		//	Compute force of gravity
		
		var dist2 = (this.x * this.x) + (this.y * this.y);
		var dist = Math.sqrt(dist2);
		var accel = -G_CONSTANT * this.mass / dist2;
		
		//	Accelerate towards center of planet
		
		this.xV += accel * UPDATE_TIME * this.x / dist;
		this.yV += accel * UPDATE_TIME * this.y / dist;
		
		//	Update position
		
		this.x += this.xV * UPDATE_TIME;
		this.y += this.yV * UPDATE_TIME;
		}
	};
	
function Pixels2X (x)
	{
	return TACTICAL_SCALE * x;
	};
	
function Pixels2Y (y)
	{
	return -TACTICAL_SCALE * y;
	};
	
function Screen2X (x)
	{
	return TACTICAL_SCALE * (x - CENTER_X);
	};
	
function Screen2Y (y)
	{
	return TACTICAL_SCALE * (CENTER_Y - y);
	};
	
function X2Screen (x)
	{
	return CENTER_X + (x / TACTICAL_SCALE);
	};
	
function Y2Screen (y)
	{
	return CENTER_Y - (y / TACTICAL_SCALE);
	};
	
function X2Pixels (x)
	{
	return x / TACTICAL_SCALE;
	};
	
function Y2Pixels (y)
	{
	return -y / TACTICAL_SCALE;
	};
	
function DrawCircle (x, y, radius)
	{
	//	Convert to screen coordinates
	
	var xCenter = X2Screen(x);
	var yCenter = Y2Screen(y);
	
	ctx.beginPath();
	ctx.arc(X2Screen(x), Y2Screen(y), X2Pixels(radius), 0.0, 2.0 * Math.PI, true);
	ctx.closePath();
	
	ctx.fill();
	};