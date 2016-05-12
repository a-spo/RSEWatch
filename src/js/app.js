Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://rsewatch.weebly.com/';
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  try{
    var configData = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration page returned: ' + JSON.stringify(configData));

    var api_key = configData['API_KEY'];
    var temp_units = configData['UNITS'];
    var date_format = configData['DATE_FORMAT'];
    //var steps_goal = configData['STEPS_GOAL'];
    if (api_key !== ''){
      localStorage.setItem('apikey', api_key);
      localStorage.setItem('tempunits', temp_units);
      localStorage.setItem('dateformat', date_format);
      //localStorage.setItem('stepsgoal', steps_goal);
      //console.log('Submitted API key is: ' + api_key);
      console.log('Submitted units is: ' + temp_units);
      console.log('Submitted date format is: ' + date_format);
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_UNITS": temp_units,
        "KEY_DATE": date_format
        //"KEY_GOAL": steps_goal
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Config info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending config info to Pebble!");
        }
      );
      getWeather();
    }
  }
  catch (e) {
    console.log('User cancelled api key input');
  }
});

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var myAPIKey = localStorage.getItem('apikey');
  var temp_units = localStorage.getItem('tempunits');
  var date_format = localStorage.getItem('dateformat');
  //console.log('Retrieved API key is: ' + myAPIKey);
  //console.log('Retrieved unit is: ' + temp_units);
  //console.log('Retrieved format is : ' + date_format);
  var endstring = '.json';
  if (pos.coords.longitude === 0){
    endstring = '.0.json';
  }
  var url = 'http://api.wunderground.com/api/' + myAPIKey + '/conditions/q/' + pos.coords.latitude + ',' + pos.coords.longitude + endstring;
  console.log(url);

  // Send request to Wunderground
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log("You are in " + json.current_observation.observation_location.full);

      var temperature = 0;
      var temp_units = localStorage.getItem('tempunits');
      //console.log("Units are: " + temp_units);
      if (temp_units == "true"){
        //console.log("Using c units here: " + temp_units);
        temperature = Math.round(json.current_observation.temp_c);
      }
      else {
        //console.log("Using f units here: " + temp_units);
        temperature = Math.round(json.current_observation.temp_f);
      }
      console.log('Temperature is ' + temperature);
      //console.log('Using celsius is: ' + temp_units);

      // Conditions
      var conditions = json.current_observation.weather;      
      console.log('Conditions are ' + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather
    // get rid of this
    //getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }                     
);