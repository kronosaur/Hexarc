//	HX1Main.js
//
//	HX1 main file
//	Copyright © 2011 by Kronosaur Productions, LLC. All Rights Reserved.

var FRAMES_PER_SECOND =	30;
var PARTICLE_COUNT = 1000;
var PARTICLE_SIZE = 5;
var canvas = null;
var ctx = null;

var allParticles = [];
var xPos = 0;
var yPos = 0;

window.onload = function ()
	{
	var i;
	
	canvas = document.getElementById("main");
	ctx = canvas.getContext("2d");
	
	//	Initialize array
	
	for (i = 0; i < PARTICLE_COUNT; i++)
		{
		var orbitRadius = 6671000.0;
		var angle = 2.0 * Math.random() * Math.PI;
		var tangent = angle + Math.PI / 2.0;
		var speed = 7800 + (Math.random() * 2000);
		
		allParticles[i] = new CTacticalGroup(
				orbitRadius * Math.cos(angle), 
				orbitRadius * Math.sin(angle),
				speed * Math.cos(tangent),
				speed * Math.sin(tangent)
				);
		}

	//	Start animation
		
	setInterval(drawFrame, 1000 / FRAMES_PER_SECOND);
	};
	
function Particle (x, y, xV, yV)
	{
	this.x = x;
	this.y = y;
	this.xV = xV;
	this.yV = yV;
	};

function drawFrame ()
	{
	var i;
	
	ctx.clearRect(0, 0, canvas.width, canvas.height);
	ctx.fillStyle = "#ff0000";
	
	//	Draw planet
	
	DrawCircle(0, 0, 6371000.0);
	
	//	Draw particles
	
	for (i = 0; i < PARTICLE_COUNT; i++)
		{
		allParticles[i].Draw();
		allParticles[i].Update();
		}
	};
