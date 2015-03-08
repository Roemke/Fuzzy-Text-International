var VERSION = "1.4";

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
  console.log("call to readyCallback");
  while (callbacks.length > 0) { //if callbacks on stack, process them
    callback = callbacks.shift();
    callback(event);
  }
  
  //try to send a message 
  //Pebble.showSimpleNotificationOnPebble(title, text);
}

function showConfiguration(event) {
  onReady(function() {
    console.log("showConfiguration");
    var opts = getOptions(); //load from localStorage
    var url  = "http://zb42.de/pebble/fuzzy/configure-fuzzy-text.html";
    //var url = "http://192.168.2.54/roemke/pebble/fuzzy/configure-fuzzy-text.html";
    Pebble.openURL(url + "#v=" + encodeURIComponent(VERSION) + "&options=" + encodeURIComponent(opts));
    //console.log(url + "#v=" + encodeURIComponent(VERSION) + "&options=" + encodeURIComponent(opts));
  });
}

//response from configure-fuzzy-text.html
function webviewclosed(event) {
/*  console.log("closed event");
  for (var props in event)
  {
    console.log(props + " with data:" + event[props]);
  }*/
  var resp = event.response;
  //console.log('configuration response: '+ resp + ' ('+ typeof resp +')');
 /* leads to eg
    {"invert":false,"text_align":"right","lang":"de","delta":false,"battery":true,
    "timeTable":{"Tue":[{"start":"10:51","end":"11:51","own":false}],"Wed":[{"start":"10:51","end":"11:51","own":true}]}}
    now changed because c-app can only work with pairs of key value, so i used the above keys for the days : -) 
    now it looks like:
    response: {"2000":"11:44|12:44|1","2001":"13:44|14:44|0","4000":"13:44|14:44:0","invert":false,"text_align":"right","lang":"de","delta":false,"battery":true} (string)    
   */
  resp = resp.replace("|27","'"); 
  var options = JSON.parse(resp);
  //console.log(options.hours);
  //console.log(options.rels);  
  //all undefined - return? - yes it means cancel button
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
  //console.log("In prepare with " + serialized_settings);
  var settings = JSON.parse(serialized_settings);
  var result =  {
    "10": settings.invert ? 1 : 0,
    "1": alignments[settings.text_align],
    "2": langs[settings.lang],
    "3": settings.delta ? 1 : 0,
    "4": settings.done ? 1 : 0,
    "5": settings.battery ? 1 : 0, 
    "6": settings.warnown  ? 1 : 0,
    "7": settings.hours ? settings.hours.join('|') : '',
    "8": settings.rels ? settings.rels.join('|') : ''
  };
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
  //console.log('sending message: '+ JSON.stringify(settings));
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
//and one for message send from pebble
//ack is send by pebble js, we don't have to care for it 
//bad luck, api is not supported, maybe later
Pebble.addEventListener('appmessage',
  function(e) {
      console.log('Received message: '); //ack will be send automatically
      var payload = e.payload;
      if (payload)
      {
        if (payload.SEND_BATTERY_KEY)
        {
          console.log("received request for battery status");
          for (var prop in navigator)
          {
            console.log(prop + " with " + navigator[prop]);
          }
          var bat = navigator.battery || navigator.webkitBattery || navigator.mozBattery; //old style
          if (bat)
          {
            Pebble.sendAppMessage({ 'BATTERY_LEVEL_KEY': parseInt(bat.level*100) });
            console.log("send battery level " + parseInt(bat.level *100));
          }
          else
          {//ok this is the result, no battery object in sandbox of pebble app :-(
            console.log("no battery object");
            if (navigator.getBattery)
            {
              navigator.getBattery().then(
                  function(bat)
                        {
                          console.log("found new style bat object");
                        });
            }
            else 
            {
              console.log("no new style bat object");
            }
          }
        }
      }
  }
);
        

onReady(function(event) {
  //var message = prepareConfiguration(getOptions());
  //transmitConfiguration(message); don't commit setting after loading js app
});

