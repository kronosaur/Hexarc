//	Console.js
//
//	Implements AI2 console.
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

//	Main -----------------------------------------------------------------------

$(document).ready(function () {

	//	Initialize dialogs

	initChangePasswordDlg();	
	initSignInDlg();
	initSignOutDlg();

	//	Focus

	$("#fieldInput").focus();

	//	Trap keys

	$("#fieldInput").on("keydown", OnKeydown);

	//	Periodic update (every 1/4 second)

	setInterval(OnUpdate, 250);

	//	Ready

	$Status = "ready";
	});

var $AutoCommand = "";

var $History = [""];
var $HistoryPos = 0;

//	$Status
//
//	ready: No outstanding requests; ready for user commands.
//	executing: Waiting for result from user command.
//	autoCommand: Waiting to send autoCommand
//	autoCommandExecuting: Waiting for result from autoCommand.
//	abortCommand: We should abort when we return from the server.

var $Status = null;

var K_KEY = 75;

//	Execution functions --------------------------------------------------------

function OnKeydown (e)
	{
	var fieldInput = $("#fieldInput");

	if (e.which == KEY_ARROW_UP)
		{
		if ($HistoryPos + 1 < $History.length)
			{
			var newCmd = $History[++$HistoryPos];
			fieldInput.val(newCmd);
			fieldInput.setCursorPosition(newCmd.length);
			}
		e.preventDefault();
		}
	else if (e.which == KEY_ARROW_DOWN)
		{
		if ($HistoryPos - 1 >= 0)
			{
			var newCmd = $History[--$HistoryPos];
			fieldInput.val(newCmd);
			fieldInput.setCursorPosition(newCmd.length);
			}
		e.preventDefault();
		}
	else if (e.which == KEY_ESCAPE)
		{
		$("#fieldInput").val("");
		$HistoryPos = 0;
		}
	else if (e.which == K_KEY && e.ctrlKey)
		{
		if ($Status == "autoCommand")
			{
			$Status = "ready";
			OutputText("Command terminated by user.");
			}
		else if ($Status != "ready")
			$Status = "abortCommand";

		e.preventDefault();
		}
	}

function OnExecute ()
	{
	//	Only if we're in a valid state.
	//	LATER: We need some UI to indicate that we're processing something.

	if ($Status != "ready")
		return;

	//	Get the input
	
	var input = $("#fieldInput").val();
	
	//	Echo the command
	
	OutputText("> " + input, "cmdEcho");
	$("#fieldInput").val("");

	//	Submit request to server
	
	$Status = "executing";
	$.getJSON("api/execute", { cmdLine: input }, OnResponse);
	
	//	Remember in history

	$History[0] = input;
	$History.unshift("");
	$HistoryPos = 0;
	}

function OnResponse (data)
	{
	var i;

	//	If aborting, cancel

	if ($Status == "abortCommand")
		{
		$Status = "ready";
		OutputText("Command terminated by user.");
		return;
		}

	//	Reset

	$Status = "ready";

	//	Figure out how to output based on the data.
	//
	//	We start by looking for a Hexarc error pattern.
	
	if (data[0] == "AEON2011:hexeError:v1")
		OutputText("ERROR: " + data[3]);

	//	If this is a list result the we have an array of data

	else if (data.ai2Directive == "listResult")
		{
		if (data.listResult != null && data.listResult.length > 0)
			{
			for (i = 0; i < data.listResult.length; i++)
				{
				OutputData(data.listResult[i]);
				}
			}
		}

	//	Is this a partial result? If so, output the result and set up an auto-
	//	command to continue execution.

	else if (data.ai2Directive == "partialResult")
		{
		if (data.listResult != null && data.listResult.length > 0)
			{
			for (i = 0; i < data.listResult.length; i++)
				{
				OutputData(data.listResult[i]);
				}
			}
		else if (data.partialResult != null && data.partialResult != "")
			OutputData(data.partialResult);
		
		$Status = "autoCommand";
		$AutoCommand = data.continueCommand;
		}
		
	//	Otherwise, output the result
	
	else
		OutputData(data);
		
	//	See if we need to scroll
	
	var visibleHeight = 50;
	var maxVisible = $(window).height() + $("html").scrollTop();
	if ($("#fieldInput").offset().top + visibleHeight > maxVisible)
		$("html").scrollTop($("#fieldInput").offset().top + visibleHeight - $(window).height());
	}

function OnUpdate ()
	{
	//	If we're processing auto commands, do it now

	if ($Status == "autoCommand")
		{
		if ($AutoCommand != null && $AutoCommand != "")
			{
			var command = $AutoCommand;
			$AutoCommand = "";
			$Status = "autoCommandExecuting";

			$.getJSON("api/execute", { cmdLine: command }, OnResponse);
			}
		else
			{
			OutputText("Invalid continueCommand.");
			$Status = "ready";
			}
		}
	}

function OutputData (data)
	{
	//	Is it an object? If so, we format into a table with one row per entry.
	
	if (typeof data == "object")
		{
		var i;

		//	Figure out how to display this object. We have the following choices:
		//
		//	"array" : Show a list of simple types
		//	"table" : Show a list of rows and columns
		//	"structure" : Show a list of field/value pairs

		var displayType;
		if (data instanceof Array)
			{
			//	If all the elements in the array are objects, then we turn it into a table.

			var isTable = true;
			for (i = 0; i < data.length && isTable; i++)
				if (typeof data[i] != "object" || data[i].ai2Directive)
					isTable = false;

			//	Either a table or a plain array.

			if (isTable)
				displayType = "table";
			else
				displayType = "array";
			}
		else if (data.ai2Directive == "imageResult")
			displayType = "image";

		else
			displayType = "structure";

		//	Now output the appropriate HTML

		var theHTML = "";
		if (displayType == "array")
			{
			for (i = 0; i < data.length; i++)
				theHTML = theHTML + "<li class='outputListEntry'>" + data[i] + "</li>";

			OutputHTML("ol", theHTML);
			}
		else if (displayType == "table")
			{
			//	Make a list of all columns

			var columnSet = { };
			for (i = 0; i < data.length; i++)
				{
				for (var j in data[i])
					columnSet[j] = j;
				}

			//	Generate a columns list

			var columnList = [];
			for (i in columnSet)
				columnList.push(i);

			//	Output the header

			theHTML = theHTML + "<tr>";
			for (i = 0; i < columnList.length; i++)
				theHTML = theHTML + "<td class='outputObjKey'>" + columnList[i] + "</td>";
			theHTML = theHTML + "</tr>";

			//	Output the table

			for (i = 0; i < data.length; i++)
				{
				theHTML = theHTML + "<tr>";
				for (var j = 0; j < columnList.length; j++)
					{
					if (data[i][columnList[j]] != null)
						theHTML = theHTML + "<td class='outputObjValue'>" + data[i][columnList[j]] + "</td>";
					else
						theHTML = theHTML + "<td class='outputObjValue'></td>";
					}
				theHTML = theHTML + "</tr>";
				}

			OutputHTML("table", theHTML);
			}
		else if (displayType == "image")
			{
			OutputImage(data.image, data.caption);
			}
		else
			{
			for (i in data)
				{
				var fieldValue;
				if (data[i] instanceof Array)
					fieldValue = data[i].join(", ");
				else
					fieldValue = data[i];

				theHTML = theHTML + "<tr><td class='outputObjKey'>" + i + "</td><td class='outputObjValue'>" + fieldValue + "</td></tr>";
				}
			
			OutputHTML("table", theHTML);
			}
		}

	//	If nil, then convert to "(no result)"

	else if (data == null)
		OutputText("(no result)");

	//	If this is "true" then convert to "OK"

	else if (data == true)
		OutputText("OK");

	//	If a number, then output

	else if (typeof data == "number")
		OutputText(data.toString());
		
	//	Otherwise, just display text
	
	else
		OutputText(data);
	}
	
function OutputHTML (theTag, theHTML)
	{
	var lineElement = document.createElement(theTag);
	lineElement.innerHTML = theHTML;
	
	//	Append the response to the console
	
	var consoleOutput = document.getElementById("consoleOutput");
	consoleOutput.appendChild(lineElement);
	}

function OutputImage (theImage, theCaption)
	{
	//	theImage is a Hexarc encoded image that looks as follows:
	//
	//	["aeon2011:image32:v1", Base64-encoded-PNG]

	var lineElement = document.createElement("img");
	lineElement.setAttribute("src", "data:image/png;base64, " + theImage[1]);

	//	Append the response to the console
	
	var consoleOutput = document.getElementById("consoleOutput");
	consoleOutput.appendChild(lineElement);

	//	Caption

	if (theCaption)
		{
		OutputText(theCaption);
		}
	}

function OutputText (theText, theClass)
	{
    var i;

    if (theText == null)
        theText = "";

    //  Split the text by lines

    var lines = theText.split("\n");

    //  Append each line as text

    for (i = 0; i < lines.length; i++)
        {
        var lineElement = $("<p class='" + theClass + "'></p>");
        lineElement.text(lines[i]);
        $("#consoleOutput").append(lineElement);
        }
	}

//	Sign In Dialog -------------------------------------------------------------

function initChangePasswordDlg ()
	{
	var waitingForResponse = false;
	
	function cmdCancel (e)
		{
		if (waitingForResponse)
			{
			//	LATER: Cancel request
			return;
			}

		$UI.exitDialog();
		}
		
	function cmdOK (e)
		{
		if (waitingForResponse)
			return;
			
		waitingForResponse = true;
		$UI.hideDialogError();

		var params = {
			actual: true,
			username: $UserInfo.username,
			oldPassword: $("#idOldPassword").val(),
			newPassword: $("#idNewPassword").val()
			};

		var request = $.ajax({
			url: "api/changePassword",
			type: "POST",
			data: JSON.stringify(params),
			contentType: "application/json",
			dataType: "json",
			
			success: (function (data) {
				waitingForResponse = false;
				
				//	If the result was a Hexarc error, then we display it in the
				//	dialog box (and leave the dialog box up).
				
				if ($Hexarc.isError(data))
					{
					$UI.showDialogError($Hexarc.getErrorMessage(data));
					}
					
				//	Otherwise, creation succeeded
				
				else
					{
					//	Store the authToken in a cookie and refresh the page.
					
					$Hexarc.setAuthTokenCookie(data);
					location.reload();
					}
				}),

			error: (function (jqXHR, textStatus, errorThrown) {
				waitingForReponse = false;
				})
			});
		}

	function onKeydown (e)
		{
		switch (e.which)
			{
			case KEY_ESCAPE:
				{
				cmdCancel();
				break;
				}
				
			case KEY_ENTER:
				{
				cmdOK();
				break;
				}
			}
		}

	//	Initialize dialog ------------------------------------------------------
	
	var dialog = $("dlgChangePassword");
	if (dialog == null)
		return;
		
	//	Hook up the OK/Cancel buttons
	
	$("#idChangePasswordCancel").on("click", cmdCancel);
	$("#idChangePasswordOK").on("click", cmdOK);
		
	//	Hook up the sign in link to bring up the dialog box
	
	$("#accountEdit").on("click", (function (e) { 
	
		//	Clear values
		
		$("#idOldPassword").val("");
		$("#idNewPassword").val("");
		$("#idConfirmPassword").val("");
		
		//	Bring up the dialog box
		
		$UI.enterDialog("#dlgChangePassword");

		//	Keyboard

		$UI.keydown(onKeydown);
		$("#idOldPassword").focus();
		}));
	}

function initSignOutDlg ()
	{
	//	Hook up the sign in link to bring up the dialog box
	
	$("#accountSignOut").on("click", (function (e) {
	
		//	Clear out the cookie and reload
		
		$Hexarc.setAuthTokenCookie("");
		location.reload();
		}));
	}

function initSignInDlg ()
	{
	var waitingForResponse = false;
	
	function cmdCancel (e)
		{
		if (waitingForResponse)
			{
			//	LATER: Cancel request
			return;
			}

		$UI.exitDialog();
		}
		
	function cmdOK (e)
		{
		if (waitingForResponse)
			return;
			
		waitingForResponse = true;
		$UI.hideDialogError();

		var params = {
			actual: true,
			username: $("#idSignInName").val(),
			password: $("#idSignInPassword").val()
			};

		var request = $.ajax({
			url: "api/login",
			type: "POST",
			data: JSON.stringify(params),
			contentType: "application/json",
			dataType: "json",
			
			success: (function (data) {
				waitingForResponse = false;
				
				//	If the result was a Hexarc error, then we display it in the
				//	dialog box (and leave the dialog box up).
				
				if ($Hexarc.isError(data))
					{
					$UI.showDialogError($Hexarc.getErrorMessage(data));
					}
					
				//	Otherwise, creation succeeded
				
				else
					{
					//	Store the authToken in a cookie and refresh the page.
					
					$Hexarc.setAuthTokenCookie(data.authToken);
					location.reload();
					}
				}),

			error: (function (jqXHR, textStatus, errorThrown) {
				waitingForReponse = false;
				})
			});
		}

	function onKeydown (e)
		{
		switch (e.which)
			{
			case KEY_ESCAPE:
				{
				cmdCancel();
				break;
				}
				
			case KEY_ENTER:
				{
				cmdOK();
				break;
				}
			}
		}

	//	Initialize dialog ------------------------------------------------------
	
	var signInDlg = $("dlgSignIn");
	if (signInDlg == null)
		return;
		
	//	Hook up the OK/Cancel buttons
	
	$("#idSignInCancel").on("click", cmdCancel);
	$("#idSignInOK").on("click", cmdOK);
		
	//	Hook up the sign in link to bring up the dialog box
	
	$("#accountSignIn").on("click", (function (e) { 
	
		//	Clear values
		
		$("#idSignInName").val("");
		$("#idSignInPassword").val("");
		
		//	Bring up the dialog box
		
		$UI.enterDialog("#dlgSignIn");

		//	Keyboard

		$UI.keydown(onKeydown);
		$("#idSignInName").focus();
		}));
	}

//	Helpers for JQuery ---------------------------------------------------------

$.fn.setCursorPosition = function(pos) {
  this.each(function(index, elem) {
    if (elem.setSelectionRange) {
	  elem.focus();
      elem.setSelectionRange(pos, pos);
    } else if (elem.createTextRange) {
      var range = elem.createTextRange();
      range.collapse(true);
      range.moveEnd('character', pos);
      range.moveStart('character', pos);
      range.select();
    }
  });
  return this;
};
