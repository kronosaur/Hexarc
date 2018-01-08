//	core.js
//
//	Core functions and definitions.
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.
//
//	VERSION
//
//	1:	Initial implementation
//	2:	Enhancements required for Multiverse

//	UI -------------------------------------------------------------------------

var KEY_BACKSPACE = 8;
var KEY_ENTER = 13;
var KEY_ESCAPE = 27;
var KEY_ARROW_LEFT = 37;
var KEY_ARROW_UP = 38;
var KEY_ARROW_RIGHT = 39;
var KEY_ARROW_DOWN = 40;
var KEY_PLUS = 43;
var KEY_MINUS = 45;
var KEY_0 = 48;
var KEY_9 = 57;
var KEY_KEYPAD_PLUS = 107;
var KEY_KEYPAD_MINUS = 109;

var $UI = {
	windowKeydown: null,
	windowKeydownSaved: [],
	windowKeypress: null,
	windowKeypressSaved: [],
	
	//	Selector for current dialog element.
	currentDialog: null
	};
	
$UI.centerElement = function (elementSel)
	{
	var dlgTop = ($(window).height() / 3) - ($(elementSel).height() / 2);
	var dlgLeft = ($(window).width() - $(elementSel).width()) / 2;
	
	$(elementSel).css({top:dlgTop, left:dlgLeft});
	}

$UI.enterDialog = function (dialogFrameSel)
	{
	if ($UI.currentDialog != null)
		return;

	//	Gray out the page
	
	$("#pageCover").show();
	
	//	Center the dialog box
	
	$UI.centerElement(dialogFrameSel);
	
	//	Show it.
	
	$(dialogFrameSel).show();
	
	//	Remember that we have it up
	
	$UI.currentDialog = dialogFrameSel;
	
	//	Disable keyboard input
	
	if ($UI.windowKeydown != null)
		{
		$UI.windowKeydownSaved.push($UI.windowKeydown);
		$UI.windowKeydown = null;
		$(window).off("keydown");
		}
	else
		$UI.windowKeydownSaved.push(null);
	
	if ($UI.windowKeypress != null)
		{
		$UI.windowKeypressSaved.push($UI.windowKeypress);
		$UI.windowKeypress = null;
		$(window).off("keypress");
		}
	else
		$UI.windowKeypressSaved.push(null);
	}

$UI.exitDialog = function ()
	{
	//	Dialog must be up
	
	if ($UI.currentDialog == null)
		return;
		
	//	Hide error, if we displayed it
	
	$UI.hideDialogError();
		
	//	Remove keyboard events, in case dialog added them.

	if ($UI.windowKeydown != null)
		$UI.keydown(null);
	if ($UI.windowKeypress != null)
		$UI.keypress(null);
	
	//	Reenable keyboard input
	
	$UI.windowKeypress = $UI.windowKeypressSaved.pop();
	if ($UI.windowKeypress != null)
		$(window).on("keypress", $UI.windowKeypress);
		
	$UI.windowKeydown = $UI.windowKeydownSaved.pop();
	if ($UI.windowKeydown != null)
		$(window).on("keydown", $UI.windowKeydown);
	
	//	Hide the dialog
	
	$($UI.currentDialog).hide();
	$("#pageCover").hide();
	$UI.currentDialog = null;
	}
	
$UI.hideDialogError = function ()
	{
	if ($UI.currentDialog == null)
		return;

	var errorBar = $($UI.currentDialog + " .dialogErrorBar");
	if (errorBar == null)
		return;
		
	errorBar.hide();	
	}
	
$UI.keydown = function (onKeydown)
	{
	$UI.windowKeydown = onKeydown;
	
	if (onKeydown != null)
		$(window).on("keydown", onKeydown);
	else
		$(window).off("keydown");
	}
	
$UI.keypress = function (onKeypress)
	{
	$UI.windowKeypress = onKeypress;
	
	if (onKeypress != null)
		$(window).on("keypress", onKeypress);
	else
		$(window).off("keypress");
	}
	
$UI.showDialogError = function (message)
	{
	if ($UI.currentDialog == null)
		return;
		
	var errorBar = $($UI.currentDialog + " .dialogErrorBar");
	if (errorBar == null)
		return;

	errorBar.show();
	errorBar.text(message);
	}
	
//	Hexarc ---------------------------------------------------------------------

var $Hexarc = {};

$Hexarc.challengeResponse = function (username, password, challenge)
	{
	//	Generate a string combining the username and password (plus some salt)
	//	so that we can hash it. This is the standard Hexarc algorithm for
	//	hashing a username and password.
	
	var credentialsString = username.toLowerCase() + ":HEXARC01:" + password;
	
	//	Convert the string to an array of bytes.
	
	//	Generate a SHA1 digest of the username+password. This is the password
	//	hash. (The result is also an array of bytes).
	
	//	Convert the challenge from a ipInteger to an array of bytes
	//	(big endian).
	
	//	Concatenate the password hash and the challenge (separated by a colon)
	
	//	Generate a SHA1 digest of the result and return it.
	

	}
	
$Hexarc.errorGetText = function (data)
	{
	return data[3];
	}
	
$Hexarc.getURLParam = function (param)
	{
	name = param.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
	var regexS = "[\\?&]" + name + "=([^&#]*)"; 
	var regex = new RegExp(regexS); 
	var results = regex.exec(window.location.search); 
	if (results == null) 
		return ""; 
	else 
		return decodeURIComponent(results[1].replace(/\+/g, " ")); 
	}
	
$Hexarc.hasRights = function (rights, rightsRequired)

//	Returns TRUE if rights is a superset of rightRequired.

	{
	var i;

	if (rights == null)
		return false;
	
	for (i = 0; i < rightsRequired.length; i++)
		if (rights.indexOf(rightsRequired[i]) == -1)
			return false;
			
	return true;
	}
	
$Hexarc.isError = function (data)
	{
	return (data != null && data[0] == "AEON2011:hexeError:v1");
	}
	
$Hexarc.getErrorMessage = function (data)
	{
	return data[3];
	}

$Hexarc.setAuthTokenCookie = function (authToken)
	{
	var expirationDate = new Date();
	expirationDate.setDate(expirationDate.getDate() + 30);
	
	var cookieValue = "";
	cookieValue += "ai2_authToken=";
	cookieValue += encodeURIComponent(authToken);
	cookieValue += "; path=/; domain=";
	cookieValue += document.domain;
	cookieValue += "; expires=";
	cookieValue += expirationDate.toUTCString();
	
	//	Set
	
	document.cookie = cookieValue;
	}
