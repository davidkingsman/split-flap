// Exception for german umlaute, replaces ä, ö, ü with unused unicode characters #, $, %
const form = document.getElementById('form');
form.onsubmit = function () {
	var r = document.getElementById('input1').value;
	r = r.replace(/ä/gi, '$');
	r = r.replace(/ö/gi, '&');
	r = r.replace(/ü/gi, '#');
	document.getElementById('input1').value = r;
}

//Updates slider value while sliding
function updateSpeedSlider(element) {
	var sliderValue = document.getElementById("SpeedSlider").value;
	document.getElementById("textSpeedSliderValue").innerHTML = sliderValue + " %";
}

// Retrieve current Split-Flap settings when the page loads/refreshes
window.addEventListener('load', getValues);
// Request and retrieve settings from ESP-01s filesystem
function getValues() {
	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function () {
		if (this.readyState == 4 && this.status == 200) {
			var myObj = JSON.parse(this.responseText);
			console.log(myObj);
			//set slider value from retrieved value
			if (myObj.speedSlider) {
				document.getElementById("textSpeedSliderValue").innerHTML = myObj.speedSlider + " %";
				document.getElementById("SpeedSlider").value = myObj.speedSlider;
			}
			//set mode from retrieved value
			if (myObj.devicemode) {
				setSavedMode(myObj.devicemode);
			}
			//set text alignment from retrieved value
			if (myObj.alignment) {
				setAlignment(myObj.alignment);
			}
		}
	};
	xhr.open("GET", "/values", true);
	xhr.send();

}

//sets mode by checking corresponding checkbox
function setSavedMode(mode) {
	switch (mode) {
	case "text":
		document.getElementById("modetext").checked = true;
		break;
	case "date":
		document.getElementById("modedate").checked = true;
		break;
	case "clock":
		document.getElementById("modeclock").checked = true;
		break;
	}
}

//sets alignment by checking corresponding checkbox
function setAlignment(alignment) {
	switch (alignment) {
	case "left":
		document.getElementById("textleft").checked = true;
		break;
	case "center":
		document.getElementById("textcenter").checked = true;
		break;
	case "right":
		document.getElementById("textright").checked = true;
		break;
	}
}
