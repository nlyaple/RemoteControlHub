
const remotesList = {"1": "3 Button - Up/Stop/Down (433MHz)",
            		 "2": "3 Button - Open/Stop/Close (433 MHz)",
            		 "3": "6 Button - Open/Stop/Close/Partial open (433 MHz)",
            		 "4": "MinkaAire AireControl (303 MHz)"
            		 };

const remoteTypes = {"1":["Up","Stop","Down"], 
                     "2":["Open", "Stop", "Close"],
                     "3":["Open", "Stop", "Close", "Qtr", "Half", "Third"],
   					 "4":["Speed 1", "Speed 2", "Speed 3", "Speed 4", "Speed 5", "Speed 6", "Stop", "Direction", "Light On", "Light Off"]
                    };

var remoteType =0 


function checkBrowserCaps() {
    var w = window.innerWidth;
    var h = window.innerHeight;
    if ((w < 480) || (h < 480))
    {
        console.log("Browser window is too small");
    }
}


function createRemote(devName, rmType) {
	// Create a break line element
	var br = document.createElement("br");
               
    // Create a form dynamically
    var form = document.createElement("form");
    form.setAttribute("method", "post");
    form.setAttribute("class", "rmbtns");
    form.setAttribute("id", devName);
    form.setAttribute("action", "/devaction");
    form.setAttribute("onchange","deviceFunc(id)"); 
    var FORMLBL = document.createElement("label");
    FORMLBL.setAttribute("for", devName);
    FORMLBL.innerHTML = devName;
    
    var FIELDSET = document.createElement("fieldset");
    FIELDSET.appendChild(FORMLBL);
    FIELDSET.appendChild(br.cloneNode());
    
    for (let i = 0; i < remoteTypes[rmType].length; i++) {
    	// Create an input element for Radio Button
    	var BTN = document.createElement("input");
    	BTN.setAttribute("type", "radio");
    	BTN.setAttribute("id", remoteTypes[rmType][i]);
    	BTN.setAttribute("name", rmType);
    	BTN.setAttribute("value", remoteTypes[rmType][i]);
    	var BTNLBL = document.createElement("label");
    	BTNLBL.setAttribute("for", remoteTypes[rmType][i]);
    	BTNLBL.innerHTML = remoteTypes[rmType][i];
                 
    	// Append the Button to the form
    	FIELDSET.appendChild(BTN);
    	FIELDSET.appendChild(BTNLBL);
    	// Inserting a line break
    	FIELDSET.appendChild(br.cloneNode());
	}                 

    form.appendChild(FIELDSET);
     
    document.getElementsByTagName("div")[0]
   .appendChild(form);
}


	
function  createTemplate(rmType) {
    var element = document.getElementById("rmType");
	 if(typeof(element) != 'undefined' && element != null) {    
    	console.log("Element already exists");
    	alert("ELement already exists");
   		return;
	}
    // Create a form dynamically
    // Create a break line element
    var br = document.createElement("br");
    
    var form = document.createElement("form");
    form.setAttribute("id", "rmType");
    var FIELDSET = document.createElement("fieldset");
    FIELDSET.setAttribute("id", "fieldSet");
    FIELDSET.appendChild(br.cloneNode());

    for (let i = 0; i < remoteTypes[rmType].length; i++) {
   
    	var BTNLBL = document.createElement("label");
    	BTNLBL.setAttribute("for", remoteTypes[rmType][i]);
    	BTNLBL.innerHTML = remoteTypes[rmType][i] + ' button: ';

    	var DBTN = document.createElement("input");
    	DBTN.setAttribute("type", "button");
    	DBTN.setAttribute("id", "detect" + remoteTypes[rmType][i]);
	    DBTN.setAttribute("class", "rmbtns");
    	DBTN.setAttribute("name", remoteTypes[rmType][i]);
    	DBTN.setAttribute("value", "Detect");
    	DBTN.setAttribute("onclick", "stateChg(this.id, this.value, this.name)");

    	var VBTN = document.createElement("input");
    	VBTN.setAttribute("type", "button");
    	VBTN.setAttribute("id", "verify" + remoteTypes[rmType][i]);
	    VBTN.setAttribute("class", "rmbtns");
    	VBTN.setAttribute("name", remoteTypes[rmType][i]);
    	VBTN.setAttribute("value", "Verify");
    	VBTN.setAttribute("disabled", "");
    	VBTN.setAttribute("onclick", "stateChg(this.id, this.value, this.name)");

    	// Append the Button to the form
    	FIELDSET.appendChild(BTNLBL);
    	FIELDSET.appendChild(DBTN);
    	FIELDSET.appendChild(VBTN);
    	// Inserting a line break
    	FIELDSET.appendChild(br.cloneNode());
    	FIELDSET.appendChild(br.cloneNode());
   	}
    form.appendChild(FIELDSET);

    document.getElementById("deviceWindow").appendChild(form);
    //document.getElementsByTagName("div")[0].appendChild(form);
}			 

function getUnitNumber() {
    var element = document.getElementById("rmType");
	 if(typeof(element) != 'undefined' && element != null) {    
    	console.log("Element already exists");
    	alert("ELement already exists");
   		return;
	}
    // Create a form dynamically
    // Create a break line element
    var br = document.createElement("br");
    
    var form = document.createElement("form");
    form.setAttribute("id", "rmType");
    form.appendChild(br.cloneNode());

  	var unitLbl = document.createElement("label");
   	unitLbl.setAttribute("for", "unitNum");
   	unitLbl.innerHTML = 'Enter unit number for Fan named above: ';
    
   	var unitNum = document.createElement("input");
   	unitNum.setAttribute("type", "number");
    unitNum.setAttribute("id", "unitNum");
    unitNum.setAttribute("min", "0");
    unitNum.setAttribute("max", "31");
//    unitNum.setAttribute("onsubmit", "processUnitNum(this.value)");

   form.appendChild(unitLbl);
   form.appendChild(unitNum);

    document.getElementById("deviceWindow").appendChild(form);
}

    
function remoteTypeFunc() {
    var selectBox = document.getElementById("typeList");
    var element = document.getElementById("rmType");
	 if(typeof(element) != 'undefined' && element != null) {    
    	console.log("Element already exists");
    	if (selectBox.options[selectBox.selectedIndex].value != remoteType) {
			window.location.reload();
		} else {
	    	return;
	    }
	}
    remoteType = selectBox.options[selectBox.selectedIndex].value;
    console.log("remoteTypeFunc value: ", remoteType);
    document.getElementById("deviceName").disabled=false;
    //document.getElementById("nextBtn").disabled=false;
}    

    
const buttonsPushed = [];	
function devNameChg(val) {
	console.log("devNameChg value: ", val);
    if (val)
    {
    	var alertText = "New device name is: " + val;
    	if (remoteType < 4) {
	        createTemplate(remoteType);
	    } else {
	        getUnitNumber();
            document.getElementById("addDone").disabled=false;
	    }   
    }
}
    
function stateChg(idVal, val, name) {
	console.log("Entered stateChg, val:", val);
	console.log("Entered stateChg, name:", name);
	deviceFunc(val, name);
	if (buttonsPushed.indexOf(idVal) == -1) {
		buttonsPushed.push(idVal);
	}
    if (val.includes("Detect"))
	{
		document.getElementById("verify" + name).disabled=false;
	}
    if (buttonsPushed.length == (remoteTypes[remoteType].length) * 2 ) {
        document.getElementById("addDone").disabled=false;
    }
	console.log(buttonsPushed);
}

function processUnitNum(val) {
	console.log("UnitNumber value: ", val);
    document.getElementById("addDone").disabled=false;
}

function processSubmit() {
   	if (remoteType == 4) {
        unitID = document.getElementById("unitNum");
        deviceFunc("Create", unitID.value);
   	}
	window.location.reload();
}

function deviceAddFunc(val, name) {
    devName = document.getElementById("deviceName");
    deviceName = devName.value;
	console.log("Device Name value: ", deviceName);
	console.log("Button value: ", val);
	JSONBody = JSON.stringify({
			"device": deviceName,
			"remoteType": remoteType,
			"action": val,
			"actionName": name,
		});
	console.log("Json body: ", JSONBody);

	fetch('http://%IPADDRESS%/devadd', {   
		method: "POST",
		headers: {
			"x-api-key": "69c45815-ca2e-45e5-a2ea-e07aafb37595",
		},
		body: JSON.stringify({
			"device": deviceName,
			"remoteType": remoteType,
			"action": val,
			"actionName": name,
		})
	})
	.then(response => {
		console.log(response); 
    })
    .catch((err) => {
		console.log(err);
	});
}

// From Events.html


var selectedEvent = -1;

function setupEventSched(id) {
	console.log("Event id: ", id);
	selectedEvent = id;
	document.getElementById("main").innerHTML = "";	
	buildRemotes();	
	const task = async () => {
		await new Promise(r => setTimeout(r, 250));
	}
	getEventData(id);
}


function getEventData(eventNum) {
	fetch('http://%IPADDRESS%/getevent?event=' + eventNum, 
	{
    	method: "GET",
    	headers: {
        	"x-api-key": "69c45815-ca2e-45e5-a2ea-e07aafb37595"
    	}
	})
	.then((response) => {
    	console.log(response); 
    	response.json().then((jsonResponse) => {
        	console.log("JSON Response: ",jsonResponse);
        	eventTime = jsonResponse["time"];
	    	var timeSet = document.getElementById("eventTime");
			timeSet.value = eventTime;
        	events = jsonResponse["events"]; 
 
        	Object.entries(jsonResponse["events"]).forEach(([key, value]) => {
             	console.log("Key: ", key);
             	console.log("Value: ", value);
				var device = value.slice(0, value.indexOf("~"));
				var action =  value.slice(value.indexOf("~")+1);
				console.log("device: ", device);
				console.log("action: ", action);

    			var selection = document.getElementById(device);
    			var ele = selection.getElementsByTagName('input');
    			for(x = 0; x < ele.length; x++) {
        			if (ele[x].type="radio") {
        				//console.log("Button: ", ele[x].value)
            			if (ele[x].value == action)
            			{
	                		ele[x].checked = true;
    	        		}
        			}
    			}
        	})
    	})
    })
    .catch((err) => {
		console.log(err);
	});
}


function createRemote(devName, rmType) {
	// Create a break line element
	var br = document.createElement("br");
               
    // Create a form dynamically
    var form = document.createElement("form");
    form.setAttribute("method", "post");
    form.setAttribute("class", "rmbtns");
    form.setAttribute("id", devName);
    form.setAttribute("action", "/devaction");
    var FORMLBL = document.createElement("label");
    FORMLBL.setAttribute("for", devName);
    FORMLBL.setAttribute("id", "Label");
    FORMLBL.innerHTML = devName;
    
    var FIELDSET = document.createElement("fieldset");
    FIELDSET.setAttribute("id", "fieldSet");
    FIELDSET.appendChild(FORMLBL);
    FIELDSET.appendChild(br.cloneNode());
    
    for (let i = 0; i < remoteTypes[rmType].length; i++) {
    	// Create an input element for Radio Button
    	var BTN = document.createElement("input");
    	BTN.setAttribute("type", "radio");
    	BTN.setAttribute("id", remoteTypes[rmType][i]);
    	BTN.setAttribute("name", rmType);
    	BTN.setAttribute("value", remoteTypes[rmType][i]);
    	var BTNLBL = document.createElement("label");
    	BTNLBL.setAttribute("for", remoteTypes[rmType][i]);
	    BTNLBL.setAttribute("id", "BTNLabel");
    	BTNLBL.innerHTML = remoteTypes[rmType][i];
                 
    	// Append the Button to the form
    	FIELDSET.appendChild(BTN);
    	FIELDSET.appendChild(BTNLBL);
    	// Inserting a line break
    	FIELDSET.appendChild(br.cloneNode());
	}                 

    form.appendChild(FIELDSET);
     
    document.getElementsByTagName("div")[0]
   .appendChild(form);
}


function saveEvent() {
	console.log("saveEvent, saving event: ", selectedEvent);
    var timeSet = document.getElementById("eventTime");
	eventTime = timeSet.value;
	var events = [];
	
	var divMain = document.getElementById("main");
	if (divMain) {
		var divElem = divMain.firstChild;
		while (divElem) {
			//console.log("Found a divElem: ", divElem.id);
			var divSubElem = divElem.firstChild;
			if (divSubElem) {
				while (divSubElem) {
					if (divSubElem.type == "fieldset") {
						//console.log("found divSubElem: ", divSubElem.id);	
						var elemBtn = divSubElem.firstChild;
						if (elemBtn) {
							while (elemBtn) {
								if (elemBtn.type == "radio") {
									//console.log("found elemBtn id: ", elemBtn.id);
									if (elemBtn.checked) {
										var eventName = divElem.id + '~' + elemBtn.id;
										console.log("Button is checked: ", eventName);
										events.push(eventName);
									}
								}
								elemBtn = elemBtn.nextSibling;
							}
						}
					}
					divSubElem = divSubElem.nextSibling;	
				}
			}
			divElem = divElem.nextSibling;
		}
	} else {
		console.log("divMain not found");
	}
	JSONBody = JSON.stringify({
			"eventNum": selectedEvent,
			"time": eventTime,
			"events": events,
		});
	console.log("Json body: ", JSONBody);

	fetch('http://%IPADDRESS%/setevent', {   
		method: "POST",
		headers: {
			"x-api-key": "69c45815-ca2e-45e5-a2ea-e07aafb37595",
		},
		body: JSON.stringify({
			"eventNum": selectedEvent,
			"time": eventTime,
			"events": events,
		})
	})
	.then(response => {
		console.log(response); 
    })
    .catch((err) => {
		console.log(err);
	});
	document.getElementById("main").innerHTML = "";	
    var timeSet = document.getElementById("eventTime");
	timeSet.value = "00:00";
	
//	From remdevice.html
let list = document.getElementById("devList");


function remDeviceFunc() {
    var selectBox = document.getElementById("devList");
    device = selectBox.options[selectBox.selectedIndex].text;
	console.log("Selected text: ", device);
    deviceVal = selectBox.options[selectBox.selectedIndex].value;
    selectBox.remove(selectBox.selectedIndex);
	console.log("Selected value: ", deviceVal);
	fetch('http://%IPADDRESS%/devrem', {   
		method: "POST",
		headers: {
			"x-api-key": "69c45815-ca2e-45e5-a2ea-e07aafb37595",
		},
		body: device,
	})
	.then(response => {
		console.log(response); 
    })
    .catch((err) => {
		console.log(err);
	});
}



function deviceActionFunc(id) {
    var device = id + "~";
	console.log("Selected id: ", id);
    var selection = document.getElementById(id);
    var ele = selection.getElementsByTagName('input');
    for(i = 0; i < ele.length; i++) {
        if (ele[i].type="radio") {
            if (ele[i].checked)
            {
                device += ele[i].value;
                break;
            }
        }
    }	
	
	console.log("Selected: ", device);
	fetch('http://%IPADDRESS%/devaction', {   
		method: "POST",
		headers: {
			"x-api-key": "69c45815-ca2e-45e5-a2ea-e07aafb37595",
		},
		body: device,
	})
	.then(response => {
		console.log(response); 
    })
    .catch((err) => {
		console.log(err);
	});
}

// Build all the remote devices to choose from.
function buildRemotes()
{
    // The following builds the list of remotes.					 
    const sb = document.querySelector('#typeList');
   
    for (let key in remotesList) {
	   const option = new Option(remotesList[key], key);
    	sb.add(option, undefined);                 
    }                 
    
	fetch('http://%IPADDRESS%/deviceList', 
	{
    	method: "GET",
    	headers: {
        	"x-api-key": "69c45815-ca2e-45e5-a2ea-e07aafb37595"
    	}
	})
	.then((response) => {
    	console.log(response); 
    	response.json().then((jsonResponse) => {
        	console.log(jsonResponse); 
        	Object.entries(jsonResponse).forEach(([key, value]) => {
            	createRemote(key, value);
        	})
    	})
	})
    .catch((err) => {
		console.log(err);
	});
}
   
   
function buildDeleteList()
{
    fetch('http://%IPADDRESS%/deviceList', 
    {
        method: "GET",
        headers: {
            "x-api-key": "69c45815-ca2e-45e5-a2ea-e07aafb37595"
        }
    })
    .then((response) => {
        response.json().then((jsonResponse) => {
            console.log(jsonResponse); 
            Object.entries(jsonResponse).forEach(([key, value]) => {
//	 		list.options[list.options.length] = new Option(value, '0', false, false);
			
                let option_elem = document.createElement("option");
	      		console.log("Setting option: ", key);
                option_elem.value = key;
                option_elem.innerText = key;
                list.appendChild(option_elem);
                list.size++;
            })
        })
    })
    .catch((err) => {
	   console.log(err);
    	list.options[list.options.length] = new Option("No devices found", '0', false, false);
    });
}
	
}
