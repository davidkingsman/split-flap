//Used for local development use
const localDevelopment = false;

//Various variables
var unitCount = 0;
var timezoneOffset = 0;

//Used for submission!
const form = document.getElementById('form');
form.onsubmit = function () {
	//Show loading icon
	var containerSubmit = document.getElementById('containerSubmit');
	
	const loadingIconContainer = document.createElement("div");
	loadingIconContainer.className = "lds-facebook";

	for(var index = 0; index < 3; index++) {
		loadingIconContainer.appendChild(document.createElement("div"));
	}

	containerSubmit.replaceWith(loadingIconContainer);

	if (localDevelopment) {
		setTimeout(function() {
			location.reload();
		}, 3000);
		
		return false;
	}
	else {
		const deviceMode = document.querySelector('input[name="deviceMode"]:checked').value;
		const tzOffset = timezoneOffset * 60000;

		switch(deviceMode) {
			case "text":
				//Convert characters which don't translate directly, replaces ä, ö, ü with unused unicode characters #, $, &
				var inputTextValue = document.getElementById('inputText').value;
				inputTextValue = inputTextValue.replace(/ä/gi, '$');
				inputTextValue = inputTextValue.replace(/ö/gi, '&');
				inputTextValue = inputTextValue.replace(/ü/gi, '#');
				document.getElementById('inputText').value = inputTextValue;

				//Set the hidden date time to UNIX
				var currentScheduledDateTimeText = document.getElementById('inputScheduledDateTime').value;

				//Take into account the timezone offset when we generate the unix timestamp
				var currentScheduledDateTime = new Date(currentScheduledDateTimeText);
				var time = Math.floor((currentScheduledDateTime.getTime() - tzOffset) / 1000);
				document.getElementById('inputHiddenScheduledDateTimeUnix').value = time;

				break;
			case "countdown":
				//Set the hidden date time to UNIX
				var currentCountdownDateTimeText = document.getElementById('inputCountdownDateTime').value;

				//Take into account the timezone offset when we generate the unix timestamp
				var currentCountdownDateTime = new Date(currentCountdownDateTimeText);
				var time = Math.floor((currentCountdownDateTime.getTime() - tzOffset) / 1000);
				document.getElementById('inputHiddenCountdownDateTimeUnix').value = time;

				break;
		}
	}
}

// Retrieve current Split-Flap settings when the page loads/refreshes
window.addEventListener('load', loadPage);

// Request and retrieve settings from ESP-01s filesystem
function loadPage() {
	//Show messages from the server if need be
	const urlParams = new URLSearchParams(location.search);
	if (urlParams.get('invalid-submission') === "true") {
		showBannerMessage(`
			Something went wrong during submission. Feel free to try again, ensure that you have entered valid information.
			<br>
			Ensure things like dates provided for schedules/countdowns are in the future.
		`);
	}
	else if (urlParams.get('is-resetting-units') === "true") {
		showBannerMessage(`
			Display is now resetting/re-calibrating. It should only take a few seconds.
			<br>
			It will display different characters in order to carry this out and then go back to the last thing being displayed.
		`);
	}
	
	//Set date time fields to be a minimum of todays date/time add 1 minute
	var tzOffset = timezoneOffset * 60000;
	document.querySelectorAll('input[type="datetime-local"]').forEach((dateTimeElement) => {
		var currentDateTime = (new Date(Date.now() - tzOffset + 60000)).toISOString().slice(0, -8);
		dateTimeElement.value = dateTimeElement.min = currentDateTime;
	});

	if (localDevelopment) {
		setSpeed("80");
		setSavedMode("text");
		setAlignment("left");
		setVersion("Development")
		setUnitCount("10");
		setLastReceivedMessage(new Date().toLocaleString());
		setCountdownDate((Date.now() / 1000) + (24 * 60 * 60));
		showHideResetWifiSettingsAction(false);
		showHideOtaUpdateAction(false);
		showScheduledMessages([
			{
				"scheduledDateTimeUnix": 1690134480,
				"message": "Test Message 1"
			},
		]);

		setTimeout(function() {
			showContent();
		}, 1000);
	}
	else {
		var xhrRequest = new XMLHttpRequest();
		xhrRequest.onreadystatechange = function () {
			if (this.readyState == 4 && this.status == 200) {
				var responseObject = JSON.parse(this.responseText);
				
				timezoneOffset = responseObject.timezoneOffset;

				setSpeed(responseObject.flapSpeed);
				setSavedMode(responseObject.deviceMode);
				setAlignment(responseObject.alignment);
				setVersion(responseObject.version);
				setUnitCount(responseObject.unitCount);
				setCountdownDate(responseObject.countdownToDateUnix);
				setLastReceivedMessage(responseObject.lastTimeReceivedMessageDateTime);
				showHideResetWifiSettingsAction(responseObject.wifiSettingsResettable);
				showHideOtaUpdateAction(responseObject.otaEnabled);
				
				if (responseObject.scheduledMessages) {
					showScheduledMessages(responseObject.scheduledMessages);
				}

				showContent();
			}
		};

		xhrRequest.open("GET", "/settings", true);
		xhrRequest.send();
	}
}

// Shows a message up top of the page should the server request one to be shown
function showBannerMessage(message, hideAfterDuration) {
	var bannerMessageElement = document.getElementById('bannerMessage'); 
	bannerMessageElement.innerHTML = message;

	bannerMessageElement.classList.remove("hidden");

	if (hideAfterDuration) {
		setTimeout(function() {
			bannerMessageElement.classList.add("hidden");
		}, 7500);
	}
}

//Ongoing show how many characters are being used
function updateCharacterCount() {
	var inputText = document.getElementById('inputText').value;
	var length = inputText.replaceAll("\\n", "").length;

	var labelCharacterCount = document.getElementById("labelCharacterCount");
	var labelLineCount = document.getElementById("labelLineCount");

	labelCharacterCount.innerHTML = length;
	labelLineCount.innerHTML = Math.ceil(length / unitCount) + inputText.split("\\n").length - 1;
}

//Easy add a newline
function addNewline() {
	var inputTextElement = document.getElementById('inputText'); 
	var textWithNewline = inputTextElement.value + "\\n";
	inputTextElement.value = textWithNewline;

	updateCharacterCount();
}

//Send message to delete a message
function deleteScheduledMessage(id, message) {
	var confirmDeletion = confirm(`Delete Message '${message}'?`);
	if (!confirmDeletion) {
		return false;
	}

	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function () {
		//Reload the page
		if (this.readyState == 4 && this.status == 202) {
			window.location.reload();
		}
	};

	xhr.open("DELETE", `/scheduled-message/remove?id=${id}`, true);
	xhr.send();
}

//Updates slider value while sliding
function updateSpeedSlider() {
	var sliderValue = document.getElementById("rangeFlapSpeed").value;
	document.getElementById("rangeFlapSpeedValue").innerHTML = sliderValue + " %";
}

//Sets mode by checking corresponding radio button/tab
function setSavedMode(mode) {
	switch (mode) {
		case "text":
			document.getElementById("modeText").checked = true;
			break;
		case "date":
			document.getElementById("modeDate").checked = true;
			break;
		case "clock":
			document.getElementById("modeClock").checked = true;
			break;
		case "countdown":
			document.getElementById("modeCountdown").checked = true;
			break;
	}

	setDeviceModeTab(mode);
}

//Shows/hides the tab associated with the device mode
function setDeviceModeTab(mode) {
	document.querySelectorAll('.tab').forEach(function(tab) {
		if (!tab.classList.contains("hidden")) {
			tab.classList.add("hidden");
		}
	});

	var tabName = `tab-${mode}`;
	var tab = document.getElementById(tabName);
	if (tab !== null) {
		tab.classList.remove("hidden");
	}
}

//Sets flap speed by setting the ranges
function setSpeed(speed) {
	document.getElementById("rangeFlapSpeedValue").innerHTML = speed + " %";
	document.getElementById("rangeFlapSpeed").value = speed;
}

//Sets alignment by checking corresponding radio button
function setAlignment(alignment) {
	switch (alignment) {
		case "left":
			document.getElementById("radioLeft").checked = true;
			break;
		case "center":
			document.getElementById("radioCenter").checked = true;
			break;
		case "right":
			document.getElementById("radioRight").checked = true;
			break;
	}
}

//Sets the version on the UI just for awareness
function setVersion(version) {
	document.getElementById("labelVersion").innerHTML = version;
}

//Sets the version on the UI just for awareness
function setUnitCount(count) {
	document.getElementById("labelUnits").innerHTML = count;
	unitCount = count;
}

//Sets the version on the UI just for awareness
function setCountdownDate(dateUnix) {
	//Set date fields to be a minimum of tomorrows date
	var currentCountdownDate = document.getElementById('inputCountdownDateTime');
	var tzOffset = timezoneOffset * 60000;

	var currentDate = (new Date(Date.now() - tzOffset));
	var nextDayDate = new Date();
	nextDayDate.setDate(currentDate.getDate() + 1);

	//If one has been set and it is not exceeded
	if (dateUnix !== 0) {
		var countdownDate = new Date(dateUnix * 1000);
		if (countdownDate >= currentDate) {
			currentCountdownDate.value = countdownDate.toISOString().slice(0, 10);

			if (countdownDate - currentDate < 24000) {
				currentCountdownDate.min = nextDayDate.toISOString().slice(0, 10);
				return;
			}
		}
	}
	else {
		//Set date fields to be a minimum of tomorrows date	
		currentCountdownDate.value = nextDayDate.toISOString().slice(0, 10);
	}

	currentCountdownDate.min = nextDayDate.toISOString().slice(0, 10);
}

//Sets the last received post message to the server
function setLastReceivedMessage(time) {
	const timeMessage = time == "" ? "N/A" : time;
	document.getElementById("labelLastMessageReceived").innerHTML = timeMessage;
}

//Used for scheduling messages
function showHideScheduledMessageInput() {
	var dateTimeElement = document.getElementById("inputScheduledDateTime");
	var checkboxScheduled = document.getElementById("inputCheckboxScheduleEnabled");

	if (checkboxScheduled.checked) {
		dateTimeElement.classList.remove("hidden")
	}
	else {
		dateTimeElement.classList.add("hidden")
	}
}

function showHideResetWifiSettingsAction(isWifiApMode) {
	if (!isWifiApMode) {
		var linkActionResetWifi = document.getElementById("linkActionResetWifi");
		linkActionResetWifi.classList.add("hidden");
	}
}

function showHideOtaUpdateAction(isOtaEnabled) {
	if (!isOtaEnabled) {
		var linkActionOtaUpdate = document.getElementById("linkActionOtaUpdate");
		linkActionOtaUpdate.classList.add("hidden");
	}
}

//Formats and displays all scheduled messages in a "nice" format
function showScheduledMessages(scheduledMessages) {
	var elementMessageCount = document.getElementById("spanScheduledMessageCount");
	elementMessageCount.innerText = scheduledMessages.length;

	//Closest to being shown first
	scheduledMessages = scheduledMessages.sort((a, b) => a.scheduledDateTimeUnix - b.scheduledDateTimeUnix);

	for (var scheduledMessageIndex = 0; scheduledMessageIndex < scheduledMessages.length; scheduledMessageIndex++) {
		var scheduledMessage = scheduledMessages[scheduledMessageIndex];

		//Create a container for a message
		var messageElement = document.createElement("div");
		messageElement.className = "message";

		//Create a element to show the time
		var timeElement = document.createElement("div");
		timeElement.className = "time";
		timeElement.innerText = new Date((scheduledMessage.scheduledDateTimeUnix * 1000) + (timezoneOffset * 60000)).toString().slice(0, -34);

		//Create a element to show the text
		var textElement = document.createElement("div");
		textElement.className = "text";
		textElement.innerText = scheduledMessage.message.trim() == "" ? "<Blank>" : scheduledMessage.message;

		//Create a remove button
		var actionElement = document.createElement("div");
		var actionButtonElement = document.createElement("span");
		actionElement.className = "action";
		actionButtonElement.className = "remove-button";
		actionButtonElement.innerText = "Remove";
		actionButtonElement.setAttribute('onclick', `deleteScheduledMessage(${scheduledMessage.scheduledDateTimeUnix}, '${scheduledMessage.message}')`);
		actionElement.appendChild(actionButtonElement);

		//Add all the elements to the message
		messageElement.appendChild(timeElement);
		messageElement.appendChild(textElement);
		messageElement.appendChild(actionElement);

		//Append to the message container
		var container = document.getElementById("containerScheduledMessages");
		container.appendChild(messageElement);
	}
}

function showContent() {
	var elementInitialLoading = document.getElementById("initialLoading");
	var elementContent = document.getElementById("loadedContent");

	elementInitialLoading.classList.add("hidden");
	elementContent.classList.remove("hidden");
}