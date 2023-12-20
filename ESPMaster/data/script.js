//Used for local development use
const localDevelopment = false;

// Exception for german umlaute, replaces ä, ö, ü with unused unicode characters #, $, %
const form = document.getElementById('form');
form.onsubmit = function () {
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
		var r = document.getElementById('inputText').value;
		r = r.replace(/ä/gi, '$');
		r = r.replace(/ö/gi, '&');
		r = r.replace(/ü/gi, '#');
		document.getElementById('inputText').value = r;

		//Set the hidden date time to UNIX
		var currentScheduledDateTimeText = document.getElementById('inputScheduledDateTime').value;

		//Take into account the timezone offset when we generate the unix timestamp
		var currentScheduledDateTime = new Date(currentScheduledDateTimeText);
		var tzOffset = (new Date().getTimezoneOffset() * 60000);
		var time = Math.floor((currentScheduledDateTime.getTime() - tzOffset) / 1000);
		document.getElementById('inputHiddenScheduledDateTimeUnix').value = time;
	}
}

// Retrieve current Split-Flap settings when the page loads/refreshes
window.addEventListener('load', loadPage);

// Request and retrieve settings from ESP-01s filesystem
function loadPage() {
	const urlParams = new URLSearchParams(location.search);
	const isResetting = urlParams.get('is-resetting-units');
	if (isResetting !== undefined && isResetting == "true") {
		var bannerMessageElement = document.getElementById('bannerMessage'); 
		bannerMessageElement.innerHTML = `
			Display is now resetting/re-calibrating. It should only take a few seconds.
			<br>
			It will display different characters in order to carry this out and then go back to the last thing being displayed.
		`;

		bannerMessageElement.style = "display: block;";
		setTimeout(function() {
			bannerMessageElement.style = "display: none;";
		}, 5000);
	}

	if (localDevelopment) {
		setSpeed("80");
		setSavedMode("text");
		setAlignment("left");
		setVersion("Development")
		setUnitCount("10");
		showScheduledMessages([
			{
				"scheduledDateTimeMillis": 1690134480,
				"message": "Test Message 1"
			},
		]);
	}
	else {
		var tzOffset = (new Date().getTimezoneOffset() * 60000);
		var currentTime = (new Date(Date.now() - tzOffset)).toISOString().slice(0, -8);
		var dateTimeElement = document.querySelector('input[type="datetime-local"]');
		dateTimeElement.value = dateTimeElement.min = currentTime;

		var xhr = new XMLHttpRequest();
		xhr.onreadystatechange = function () {
			if (this.readyState == 4 && this.status == 200) {
				var responseObject = JSON.parse(this.responseText);
				console.log(responseObject);

				//set slider value from retrieved value
				if (responseObject.flapSpeed) {
					setSpeed(responseObject.flapSpeed);
				}

				//set mode from retrieved value
				if (responseObject.deviceMode) {
					setSavedMode(responseObject.deviceMode);
				}

				//set text alignment from retrieved value
				if (responseObject.alignment) {
					setAlignment(responseObject.alignment);
				}

				//set the version of the server
				if (responseObject.version) {
					setVersion(responseObject.version);
				}

				//set the amount of flaps we will be working with
				if (responseObject.unitCount) {
					setUnitCount(responseObject.unitCount);
				}

				//set the scheduled messages
				if (responseObject.scheduledMessages) {
					showScheduledMessages(responseObject.scheduledMessages);
				}

				setLastReceviedMessage(responseObject.lastTimeReceivedMessage);
			}
		};

		xhr.open("GET", "/settings", true);
		xhr.send();
	}
}

function updateCharacterCount() {
	var length = document.getElementById('inputText').value.replaceAll("\\n", "").length;
	document.getElementById("labelCharacterCount").innerHTML = length;
}

function addNewline() {
	var inputTextElement = document.getElementById('inputText'); 
	var textWithNewline = inputTextElement.value + "\\n";
	inputTextElement.value = textWithNewline;
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
		if (this.readyState == 4 && this.status == 201) {
			window.location.reload();
		}
	};

	xhr.open("DELETE", `/scheduled-message/remove?id=${id}`, true);
	xhr.send();
}

//ppdates slider value while sliding
function updateSpeedSlider() {
	var sliderValue = document.getElementById("rangeFlapSpeed").value;
	document.getElementById("rangeFlapSpeedValue").innerHTML = sliderValue + " %";
}

//sets mode by checking corresponding radio button
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
	}
}

//sets flap speed by setting the ranges
function setSpeed(speed) {
	document.getElementById("rangeFlapSpeedValue").innerHTML = speed + " %";
	document.getElementById("rangeFlapSpeed").value = speed;
}

//sets alignment by checking corresponding radio button
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

//sets the version on the UI just for awareness
function setVersion(version) {
	document.getElementById("labelVersion").innerHTML = version;
}

//sets the version on the UI just for awareness
function setUnitCount(unitCount) {
	document.getElementById("labelUnits").innerHTML = unitCount;
}

//sets the last received post message to the server
function setLastReceviedMessage(time) {
	document.getElementById("labelLastMessageReceived").innerHTML = time == "" ? "N/A" : time;
}

function showHideScheduledMessageInput() {
	var dateTimeElement = document.getElementById("inputScheduledDateTime");
	var checkboxScheduled = document.getElementById("inputCheckboxScheduleEnabled");

	if (checkboxScheduled.checked) {
		dateTimeElement.style.display = "inline-block";
	}
	else {
		dateTimeElement.style.display = "none";
	}
}

function showScheduledMessages(scheduledMessages) {
	var elementMessageCount = document.getElementById("spanScheduledMessageCount");
	elementMessageCount.innerText = scheduledMessages.length;

	//Closest to being shown first
	scheduledMessages = scheduledMessages.sort((a, b) => a.scheduledDateTimeMillis - b.scheduledDateTimeMillis);

	for (var scheduledMessageIndex = 0; scheduledMessageIndex < scheduledMessages.length; scheduledMessageIndex++) {
		var scheduledMessage = scheduledMessages[scheduledMessageIndex];

		//Create a container for a message
		var messageElement = document.createElement("div");
		messageElement.className = "message";

		//Create a element to show the time
		var timeElement = document.createElement("div");
		timeElement.className = "time";
		timeElement.innerText = new Date((scheduledMessage.scheduledDateTimeMillis * 1000) + (new Date().getTimezoneOffset() * 60000)).toString().slice(0, -34);

		//Create a element to show the text
		var textElement = document.createElement("div");
		textElement.className = "text";
		textElement.innerText = scheduledMessage.message;

		//Create a remove button
		var actionElement = document.createElement("div");
		var actionButtonElement = document.createElement("span");
		actionElement.className = "action";
		actionButtonElement.className = "remove-button";
		actionButtonElement.innerText = "Remove";
		actionButtonElement.setAttribute('onclick', `deleteScheduledMessage(${scheduledMessage.scheduledDateTimeMillis}, '${scheduledMessage.message}')`);
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