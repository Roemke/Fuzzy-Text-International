var VERSION = "1.3.1";

var isReady = false;
var callbacks = []; //stack for callbacks

var alignments = {
  center: 0,
  left:   1,
  right:  2
};

var langs = {
  ca:    0,
  de:    1,
  en_GB: 2,
  en_US: 3,
  es:    4,
  fr:    5,
  no:    6,
  sv:    7,
  cl:    8 // custom language
};

//var dayKeys = [1000,2000,3000,4000,5000,6000,7000]; as defined in configure-fuzzy-text.html 

function readyCallback(event) {
  isReady = true;
  var callback;
  while (callbacks.length > 0) { //if callbacks on stack, process them
    callback = callbacks.shift();
    callback(event);
  }
}

function showConfiguration(event) {
  onReady(function() {
    var opts = getOptions(); //load from localStorage
    //var url  = "http://zb42.de/pebble/fuzzy/configure-fuzzy-text.html";
    var url = "http://192.168.2.54/roemke/pebble/fuzzy/configure-fuzzy-text.html";
    Pebble.openURL(url + "#v=" + encodeURIComponent(VERSION) + "&options=" + encodeURIComponent(opts));
    console.log(url + "#v=" + encodeURIComponent(VERSION) + "&options=" + encodeURIComponent(opts));
  });
}

function webviewclosed(event) {
  var resp = event.response;
  console.log('configuration response: '+ resp + ' ('+ typeof resp +')');
 /* leads to eg
    {"invert":false,"text_align":"right","lang":"de","delta":false,"battery":true,
    "timeTable":{"Tue":[{"start":"10:51","end":"11:51","own":false}],"Wed":[{"start":"10:51","end":"11:51","own":true}]}}
    now changed because c-app can only work with pairs of key value, so i used the above keys for the days : -) 
    now it looks like:
    response: {"2000":"11:44|12:44|1","2001":"13:44|14:44|0","4000":"13:44|14:44:0","invert":false,"text_align":"right","lang":"de","delta":false,"battery":true} (string)    
   */
 
  var options = JSON.parse(resp);
  
  //all undefined - return?
  
  if (typeof options.invert === 'undefined' &&
      typeof options.text_align === 'undefined' &&
      typeof options.lang === 'undefined' &&
      typeof options.delta === 'undefined' &&
      typeof options.battery === 'undefined') 
  {
    return;
  }

  onReady(function() {
    setOptions(resp); //store in local Storage 
    var message = prepareConfiguration(resp);
    transmitConfiguration(message);
  });
}

// Retrieves stored configuration from localStorage.
function getOptions() {
  return localStorage.getItem("options") || ("{}");
}

// Stores options in localStorage.
function setOptions(options) {
  localStorage.setItem("options", options);
}

// Takes a string containing serialized JSON as input.  This is the
// format that is sent back from the configuration web UI.  Produces
// a JSON message to send to the watch face.
function prepareConfiguration(serialized_settings) {
  var settings = JSON.parse(serialized_settings);
  var result =  {
    "10": settings.invert ? 1 : 0,
    "1": alignments[settings.text_align],
    "2": langs[settings.lang],
    "3": settings.delta ? 1 : 0,
    "4": settings.done ? 1 : 0,
    "5": settings.battery ? 1 : 0, 
    "6": settings.warnown  ? 1 : 0,
  };
  //handle two array: hours and rels
  result['7'] = settings.hours.toString("\n");
  result['8'] = settings.rels.toString("\n");
  //need to append time-table to result, no can't handle that
  var dataCounter = 0; 
  for (var prop in settings)
  {
  		if (prop >>> 0 === parseFloat(prop)) //prop is integer
  		{
  			dataCounter++;
  			result[prop]= settings[prop]; //transfer it to result 
  		} //all strings wich are submitted to the c-Programm (timetable
  }
  //console.log("dataCounter is: " + dataCounter);
  result["0"] = dataCounter; //send back how many entries I have in the timetable 
  //time table has indices like defined in dayKeys see above
  return result;
}

// Takes a JSON message as input.  Sends the message to the watch.
function transmitConfiguration(settings) {
  console.log('sending message: '+ JSON.stringify(settings));
  Pebble.sendAppMessage(settings, function(event) {
    // Message delivered successfully
  }, logError);
 
}

function logError(event) {
  console.log('Unable to deliver message with transactionId='+
              event.data.transactionId +' ; Error is'+ event.error.message);
}

//he build an on stack for callback. 
//work is started if onReady event from watch set isReady to true
function onReady(callback) {
  if (isReady) {
    callback();
  }
  else {
    callbacks.push(callback);
  }
}

//we need three Listeners
Pebble.addEventListener("ready", readyCallback); //called when app is loaded on watch
Pebble.addEventListener("showConfiguration", showConfiguration);
Pebble.addEventListener("webviewclosed", webviewclosed);


onReady(function(event) {
  var message = prepareConfiguration(getOptions());
  //transmitConfiguration(message); don't commit setting after loading js app
});

