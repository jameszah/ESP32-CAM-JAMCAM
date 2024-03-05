/*  
    https://github.com/jameszah/ESP32-CAM-JAMCAM

    jameszah/ESP32-CAM-JAMCAM is licensed under the
    GNU General Public License v3.0
    March 1, 2024
*/

#include <pgmspace.h>

static const char config_txt[] PROGMEM =
  R"==x==(MST7MDT     // timezone
JAMCAM     // soft ssid
12344321   // soft password (MUST be 8 chars)
none       // ssid wifi name
mrpeanut   // ssid password
none       // ssid2 wifi name
mrpeanut   // ssid2 password
0          // touch effect video 0
9          // touch effect immortal snakes
4          // touch effect fire
5          // clock
123        // 31 length weather api account
90210      // canadian postal or amaerican zip, etc
~~~~~~~~~~~~~~~~~
0 video
1 difference
2 distance
3 pulsing
4 flames
5 time 
6 games of life
7 blinking
8 snakes
9 immortal snaks
10 tron snakes
11 colors
12 calm
13 blocktime
14 berlintime



)==x==";


///////////////////////////////
const char edit_html[] PROGMEM = R"===(
<!doctype html>
<html>
   <head>
   <style>
.slidecontainer {
  width: 100%;
}

.slider {
  -webkit-appearance: none;
  width: 600px;
  height: 25px;
  background: #d3d3d3;
  outline: none;
  opacity: 0.7;
  -webkit-transition: .2s;
  transition: opacity .2s;
}

.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 10px;
  height: 25px;
  background: #04AA6D;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 10px;
  height: 25px;
  background: #04AA6D;
  cursor: pointer;
}
</style>

      <title>JamCam Selfie VideoCam - Video Editor</title>
      <script>
var baseHost = document.location.origin
var streamUrl = baseHost + ':81/stream_avi'

document.addEventListener('DOMContentLoaded', function (event) {

   const view = document.getElementById('liveFeed')
   const videoButton = document.getElementById('video')

   videoButton.onclick = () => {
      
      //const query = `${baseHost}/find?f=/JamCam0090.0001.avi&n=12` // "/JamCam0090.0001.avi", 12
      const query = `${baseHost}/find?f=/JamCam0481.0007.avi&n=0` 
      
      view.src = query; //streamUrl;

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
   }
}
)
   const stopStream = () => {}


   
   function initialize() {
         
//      const query = `${baseHost}/find?f=/JamCam0481.0007.avi&n=0` 

//
   
   const query2 = `${baseHost}/status`
   fetch(query2)
      .then(function (response) {
         return response.json()
      })
      .then(function (state) {
         var x = state.toString()
         //console.log(x)
         //document.getElementById("file_to_edit").innerHTML = state.file_to_edit
         console.log(`request to ${query2} finished, OnOff: ${state.OnOff}`)
         console.log(`File: ${state.File}`)
         console.log(`Remain: ${state.Remain}`)
         console.log(`Total: ${state.Size}`)
         console.log(`Free: ${state.Free}`)
         console.log(`file_to_edit: ${state.file_to_edit}`)
         document.getElementById("file_to_edit").value = state.file_to_edit
         file_to_write = state.file_to_edit
         let newText = file_to_write.replace(".avi", "x.avi");
         document.getElementById("file_to_write").value = newText
         
         console.log( document.getElementById("file_to_edit").value)


        //
// +/JamCam0481.0007.avi
    console.log(document.getElementById("file_to_edit").value)
    const query = `${baseHost}/find?f=` + document.getElementById("file_to_edit").value + `&n=0` 
    console.log(`the query --- ${query}`)
    
     //document.getElementById('liveFeed').src = query // does query twice?

fetch(query)
  .then(response => { 
    response.blob()
    .then(blob => { 
    
     document.getElementById('liveFeed').src = URL.createObjectURL(blob); 
     document.getElementById("fsta").value =  0;
     document.getElementById("fcur").value =  response.headers.get("framenum");
     document.getElementById("ftotal").value =  response.headers.get("total");
     document.getElementById("fend").value =  response.headers.get("total");
     document.getElementById("myRange").max = response.headers.get("total");
     
     console.log(`request to ${query} finished, status: ${response.status}`)
     console.log("filename: " + response.headers.get("file"));   
     console.log("filepct: " + response.headers.get("framepct")); 
           })
        })  
      })
   }

      </script>
   </head>
   <body onload="initialize()" style="background-color: white">
      <div id="second">
         <h2>JamCam Selfie VideoCam</h2>
         <h3>Video Recorder With Viewfinder, Streamer, Clock, ... and Snakes!</h3>

      </div>
      <div id="third">
         <img id="liveFeed" style="font-family: monospace;  text-align: center;
            color: white; position: relative"
            alt="There seems to be a problem with the live feed..."/>
         <br>
         <div class="slidecontainer">
  <input type="range" min="0" max="100" value="0" class="slider" id="myRange">
  
  <p>Start: <input type="number" id="fsta" min=0 max=99999 style="color:blue;" value="0"> 
  --- <button id="setstart">Set Start</button> ---
  Current: <input type="number" id="fcur" min=0 max=99999 onchnage="fcurchange()" style="color:blue;" value="0"> 
  --- <button id="setend">Set End</button> ---
  End: <input type="number" id="fend" min=0 max=99999 style="color:blue;" value="0"> </p>
  <p>Total: <input type="number" id="ftotal" min=0 max=99999 readonly style="color:red;" value="0"> 
  Skip: <input type="number" id="fskip" min=0 max=999 style="color:blue;" value="0"> Keep 1 frame, then skip 1 (half size) or 9 (10%) etc </p>
  <p>File: <input type="text" id="file_to_edit" readonly size=25 style="color:red;" value="???"> 
  
  <button id="saveedited">Save Edited File</button>
  New File Name: <input type="text" id="file_to_write" size=25 style="color:blue;" value="???"></p>
  If no image, re-index the file, and edit that one:  <button id="reindex">ReIndex the File</button>

      </div>
      <div id="four">
         <hr>
         <br>
         <textarea id="freeform" name="freeformname" rows="5" cols="40">
Click Record Status ...
             </textarea>
      </div>
      <a href="https://github.com/jameszah/ESP32-CAM-JAMCAM">https://github.com/jameszah/ESP32-CAM-JAMCAM</a> <br>ver 64
<script>
var slider = document.getElementById("myRange");
var fcur = document.getElementById("fcur");
var fsta = document.getElementById("fsta");
var fend = document.getElementById("fend");
fcur.value = slider.value;

//document.getElementById('liveFeed').src = query // does query twice?

function fcurchange() {
    //if (fcur.value != this.value){
    //fcur.value = this.value;
    const query = `${baseHost}/find?f=/JamCam0481.0007.avi&n=` + fcur.value 

fetch(query)
  .then(response => { 
    response.blob()
    .then(blob => { 
    
     document.getElementById('liveFeed').src = URL.createObjectURL(blob); 
     document.getElementById("fcur").value =  response.headers.get("framenum");
     //document.getElementById("ftotal").value =  response.headers.get("total");
     console.log(`request to ${query} finished, status: ${response.status}`)
     //console.log("filename: " + response.headers.get("file"));   
     //console.log("filepct: " + response.headers.get("framepct")); 
    })
  })  
  //}
}
slider.onchange = function() {
  if (fcur.value != this.value){
    fcur.value = this.value;
    var fn = document.getElementById("file_to_edit").value
    const query = `${baseHost}/find?f=` + fn + `&n=` + this.value 

fetch(query)
  .then(response => { 
    response.blob()
    .then(blob => { 
    
     document.getElementById('liveFeed').src = URL.createObjectURL(blob); 
     document.getElementById("fcur").value =  response.headers.get("framenum");
     //document.getElementById("ftotal").value =  response.headers.get("total");
     console.log(`request to ${query} finished, status: ${response.status}`)
     //console.log("filename: " + response.headers.get("file"));   
     //console.log("filepct: " + response.headers.get("framepct")); 
    })
  })  
  }
}

const setstartButton = document.getElementById('setstart')
setstartButton.onclick = () => {
  fsta.value = fcur.value;
}

const setendButton = document.getElementById('setend')
setendButton.onclick = () => {
  fend.value = fcur.value;
}

const saveeditedButton = document.getElementById('saveedited')
saveeditedButton.onclick = () => {
  oldname = document.getElementById("file_to_edit").value 
  newname =  document.getElementById("file_to_write").value 
  newstart = document.getElementById("fsta").value 
  newend =  document.getElementById("fend").value 
  newskip = document.getElementById("fskip").value 
     
  const query = `${baseHost}/reparse?o=` + oldname +  `&n=` + newname + `&s=` + newstart + `&e=` + newend + `&k=` + newskip 


  document.getElementById('freeform').value =  "This will take a minute - wait here!\n" 
  
  fetch(query)
     .then(response => {
        console.log(`request to ${query} finished, status: ${response.status}`)
        document.getElementById('freeform').value =  "DONE DONE DONE\n"  
   })
     
}

const reindexButton = document.getElementById('reindex')
reindexButton.onclick = () => {
  oldname = document.getElementById("file_to_edit").value 
  newname =  document.getElementById("file_to_write").value 

  document.getElementById('freeform').value =  "This will take a minute - wait here!\n" 
   
  const query = `${baseHost}/reindex?o=` + oldname +  `&n=` + newname 

  
  fetch(query)
     .then(response => {
        console.log(`request to ${query} finished, status: ${response.status}`)
        document.getElementById('freeform').value =  "DONE - go back to the list of files\n"  
   })
  
}



</script>
   </body>
</html>
)===";

////////////////////////////////

const char page_html[] PROGMEM = R"===(
<!doctype html>
<html>
    <head>
       <title>JamCam Selfie VideoCam</title>
     
     <script>
var baseHost = document.location.origin
var streamUrl = baseHost + ':81/stream_avi'
var streamUrl2 = baseHost + ':81/stream_bmp'

document.addEventListener('DOMContentLoaded', function (event) {

   const view = document.getElementById('liveFeed2')
   const viewBmp = document.getElementById('liveFeedBmp')
   const viewContainer = document.getElementById('third')
   const streamButton = document.getElementById('toggle-stream')
   const streamButtonBmp = document.getElementById('toggle-bmp-stream')
   const videoButton = document.getElementById('video')
   const differenceButton = document.getElementById('difference')
   const distanceButton = document.getElementById('distance')
   const pulsingButton = document.getElementById('pulsing')
   const flamesButton = document.getElementById('flames')
   const timeButton = document.getElementById('time')
   const blocktimeButton = document.getElementById('blocktime')
   const berlintimeButton = document.getElementById('berlintime')
   const colorsButton = document.getElementById('colors')
   const lifeButton = document.getElementById('life')
   const calmButton = document.getElementById('calm')
   const cliveButton = document.getElementById('clive')
   const snakeButton = document.getElementById('snake')
   const immortalButton = document.getElementById('immortal')
   const tronButton = document.getElementById('tron')
   const recordButton = document.getElementById('record')
   const rec_tlButton = document.getElementById('rec_tl')
   const statusButton = document.getElementById('status')
   const editButton = document.getElementById('edit')
   const filemanButton = document.getElementById('fileman')
   const rebootButton = document.getElementById('reboot')
   const powerLowButton = document.getElementById('plow')
   const powerMedButton = document.getElementById('pmed')
   const powerHighButton = document.getElementById('phigh')
   const deleteButton = document.getElementById('delete')

   viewBmp.setAttribute("hidden", "hidden");
   view.setAttribute("hidden", "hidden");


   videoButton.onclick = () => {
      const query = `${baseHost}/effect?e=0`
      clear_the_button()
      document.getElementById('video').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   differenceButton.onclick = () => {
      const query = `${baseHost}/effect?e=1`
      clear_the_button()
      document.getElementById('difference').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }
   distanceButton.onclick = () => {
      const query = `${baseHost}/effect?e=2`
      clear_the_button()
      document.getElementById('distance').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }
   pulsingButton.onclick = () => {
      const query = `${baseHost}/effect?e=3`
      clear_the_button()
      document.getElementById('pulsing').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }
   flamesButton.onclick = () => {
      const query = `${baseHost}/effect?e=4`
      clear_the_button()
      document.getElementById('flames').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   timeButton.onclick = () => {
      const query = `${baseHost}/effect?e=5`
      clear_the_button()
      document.getElementById('time').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }
   blocktimeButton.onclick = () => {
      const query = `${baseHost}/effect?e=13`
      clear_the_button()
      document.getElementById('blocktime').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }
   berlintimeButton.onclick = () => {
      const query = `${baseHost}/effect?e=14`
      clear_the_button()
      document.getElementById('berlintime').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   colorsButton.onclick = () => {
      const query = `${baseHost}/effect?e=11`
      clear_the_button()
      document.getElementById('colors').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   lifeButton.onclick = () => {
      const query = `${baseHost}/effect?e=6`
      clear_the_button()
      document.getElementById('life').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   calmButton.onclick = () => {
      const query = `${baseHost}/effect?e=12`
      clear_the_button()
      document.getElementById('calm').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }


   cliveButton.onclick = () => {
      const query = `${baseHost}/effect?e=7`
      clear_the_button()
      document.getElementById('clive').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   snakeButton.onclick = () => {
      const query = `${baseHost}/effect?e=8`
      clear_the_button()
      document.getElementById('snake').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   immortalButton.onclick = () => {
      const query = `${baseHost}/effect?e=9`
      clear_the_button()
      document.getElementById('immortal').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   tronButton.onclick = () => {
      const query = `${baseHost}/effect?e=10`
      clear_the_button()
      document.getElementById('tron').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      //set_the_button()
   }

   rebootButton.onclick = () => {
      const query = `${baseHost}/reboot`

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
   }

   powerLowButton.onclick = () => {
      const query = `${baseHost}/power?p=300`

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
   }
   powerMedButton.onclick = () => {
      const query = `${baseHost}/power?p=1000`

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
   }
   powerHighButton.onclick = () => {
      const query = `${baseHost}/power?p=1500`

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
   }

   deleteButton.onclick = () => {
      const query = `${baseHost}/delete`

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
   }

   const stop_rec_and_stream = () => {
      viewBmp.src = "";
      viewBmp.setAttribute("hidden", "hidden");
      streamButtonBmp.innerHTML = 'Start Display Stream'
      streamButtonBmp.style.background = '#F0F0F0'
      view.src = "";
      view.setAttribute("hidden", "hidden");
      streamButton.innerHTML = 'Start Stream'
      streamButton.style.background = '#F0F0F0'

      const streamRecording = recordButton.innerHTML === 'Start 24fps Record'
      if (streamRecording) {

      } else {
         recordButton.innerHTML = 'Start 24fps Record'
         recordButton.style.background = '#F0F0F0'

         const query = `${baseHost}/record`
         fetch(query)
            .then(response => {
               console.log(`request to ${query} finished, status: ${response.status}`)

            })
         const streamtlRecording = rec_tlButton.innerHTML === 'Start 1fps Record'
         if (streamtlRecording) {

         } else {
            rec_tlButton.innerHTML = 'Start 1fps Record'
            rec_tlButton.style.background = '#F0F0F0'

            const query3 = `${baseHost}/rec_tl`
            fetch(query3)
               .then(response => {
                  console.log(`request to ${query3} finished, status: ${response.status}`)

               })

            const query2 = `${baseHost}/status`
            fetch(query2)
               .then(response => {
                  console.log(`request to ${query2} finished, status: ${response.status}`)

               })

         }
      }
   }

   let editwin;
   editButton.onclick = () => {
      const query = `${baseHost}:8080/e?edit=config.txt`

      window.open(query, '_blank').focus();
   }


   filemanButton.onclick = () => {
      const query = `${baseHost}/effect?e=13`
      clear_the_button()
      document.getElementById('calm').style.background = '#E8B1EA';
      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)
         })
      set_the_button()


      stop_rec_and_stream()

      //const query2 = `${baseHost}/edit`
      //window.open(query2, '_blank').focus();

      const query2 = `${baseHost}:8080`
      window.open(query2, '_blank').focus();
   }

   statusButton.onclick = () => {
      updateStatus()
      set_the_button()
   }


   recordButton.onclick = () => {
      const query = `${baseHost}/record`


      const streamRecording = recordButton.innerHTML === 'Start 24fps Record'
      if (streamRecording) {
         recordButton.innerHTML = 'Stop 24fps Recording'
         recordButton.style.background = '#E8B1EA'
         filemanButton.disabled = true
         deleteButton.disabled = true
         rec_tlButton.disabled = true
      } else {
         recordButton.innerHTML = 'Start 24fps Record'
         recordButton.style.background = '#F0F0F0'
         filemanButton.disabled = false
         deleteButton.disabled = false
         rec_tlButton.disabled = false
      }

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)

            document.getElementById('freeform').value = 'Starting'
         })

      setTimeout(function () {
         updateStatus();
      }, 500);
   }

   rec_tlButton.onclick = () => {
      const query = `${baseHost}/rec_tl`


      const streamRecording = rec_tlButton.innerHTML === 'Start 1fps Record'
      if (streamRecording) {
         rec_tlButton.innerHTML = 'Stop 1fps Recording'
         rec_tlButton.style.background = '#E8B1EA'
         filemanButton.disabled = true
         deleteButton.disabled = true
         recordButton.disabled = true
      } else {
         rec_tlButton.innerHTML = 'Start 1fps Record'
         rec_tlButton.style.background = '#F0F0F0'
         filemanButton.disabled = false
         deleteButton.disabled = false
         recordButton.disabled = false
      }

      fetch(query)
         .then(response => {
            console.log(`request to ${query} finished, status: ${response.status}`)

            document.getElementById('freeform').value = 'Starting'
         })

      setTimeout(function () {
         updateStatus();
      }, 500);
   }


   const updateStatus = () => {
      fupdateStatus()
   }

   const stopStreamBmp = () => {
      //window.stop();
      viewBmp.src = "";
      viewBmp.setAttribute("hidden", "hidden");
      streamButtonBmp.innerHTML = 'Start Display Stream'
      streamButtonBmp.style.background = '#F0F0F0'
      filemanButton.disabled = false
      deleteButton.disabled = false
   }

   const startStreamBmp = () => {
      view.src = "";
      view.setAttribute("hidden", "hidden");
      streamButton.innerHTML = 'Start Stream'
      streamButton.style.background = '#F0F0F0'
      filemanButton.disabled = true
      deleteButton.disabled = true


      viewBmp.removeAttribute("hidden");
      viewBmp.src = streamUrl2; 
      // show(viewContainer)
      streamButtonBmp.innerHTML = 'Stop Display Stream'
      streamButtonBmp.style.background = '#E8B1EA'
   }

   streamButtonBmp.onclick = () => {
      const streamEnabledBmp = streamButtonBmp.innerHTML === 'Stop Display Stream'
      if (streamEnabledBmp) {
         stopStreamBmp()
      } else {
         startStreamBmp()
      }
   }

   const stopStream = () => {
      // window.stop();
      view.src = "";
      view.setAttribute("hidden", "hidden");
      streamButton.innerHTML = 'Start Stream'
      streamButton.style.background = '#F0F0F0'
      filemanButton.disabled = false
      deleteButton.disabled = false
   }

   const startStream = () => {
      viewBmp.src = "";
      viewBmp.setAttribute("hidden", "hidden");
      streamButtonBmp.innerHTML = 'Start Display Stream'
      streamButtonBmp.style.background = '#F0F0F0'
      filemanButton.disabled = true
      deleteButton.disabled = true


      view.removeAttribute("hidden");
      view.src = streamUrl; 
      //viewBmp.src = streamUrl2;
      //show(viewContainer)
      streamButton.innerHTML = 'Stop Stream'
      streamButton.style.background = '#E8B1EA'
   }

   streamButton.onclick = () => {
      const streamEnabled = streamButton.innerHTML === 'Stop Stream'
      if (streamEnabled) {
         stopStream()
      } else {
         startStream()
      }
   }
})

function fupdateStatus() {
   const recordButton = document.getElementById('record')
   const rec_tlButton = document.getElementById('rec_tl')
   const statusButton = document.getElementById('status')
   const editButton = document.getElementById('edit')
   const filemanButton = document.getElementById('fileman')
   const deleteButton = document.getElementById('delete')

   const query2 = `${baseHost}/status`
   fetch(query2)
      .then(function (response) {
         return response.json()
      })
      .then(function (state) {
         var x = state.toString()

         console.log(`request to ${query2} finished, OnOff: ${state.OnOff}`)
         console.log(`File: ${state.File}`)
         console.log(`Remain: ${state.Remain}`)
         console.log(`Total: ${state.Size}`)
         console.log(`Free: ${state.Free}`)
         console.log(`EditFile: ${state.File_to_edit}`)

         if (state.OnOff == 'On') {
            recordButton.innerHTML = 'Stop 24fps Recording'
            recordButton.style.background = '#E8B1EA'
            rec_tlButton.innerHTML = 'Start 1fps Record'
            rec_tlButton.style.background = '#F0F0F0'
            filemanButton.disabled = true
            deleteButton.disabled = true
            rec_tlButton.disabled = true
            document.getElementById('freeform').value = state.IP + " " + state.rssi + '\n' + 'Recording ' + state.OnOff + ', Power: ' + state.Power + '\n' + state.File + '\n' + state.Remain + ' seconds left\n' + 'SD Free: ' + state.Free + 'MB/' + state.Size + 'MB  ' + String((100 * (state.Free / state.Size)).toFixed(1)) + '% free'
         } else if (state.OnOff == 'TL') {
            recordButton.innerHTML = 'Start 24fps Record'
            recordButton.style.background = '#F0F0F0'
            rec_tlButton.innerHTML = 'Stop 1fps Recording'
            rec_tlButton.style.background = '##E8B1EA'

            filemanButton.disabled = true
            deleteButton.disabled = true
            recordButton.disabled = true
            //document.getElementById('freeform').value = state.IP + " " + state.rssi + '\n' + 'Recording ' + state.OnOff + '\n'  +  'SD Free: ' +  state.Free + 'MB/' + state.Size + 'MB  '   + String((100*(state.Free/state.Size)).toFixed(1)) + '% free' 
            document.getElementById('freeform').value = state.IP + " " + state.rssi + '\n' + 'Recording ' + state.OnOff + ', Power: ' + state.Power + '\n' + state.File + '\n' + state.Remain + ' seconds left\n' + 'SD Free: ' + state.Free + 'MB/' + state.Size + 'MB  ' + String((100 * (state.Free / state.Size)).toFixed(1)) + '% free'
         } else {
            recordButton.innerHTML = 'Start 24fps Record'
            recordButton.style.background = '#F0F0F0'
            rec_tlButton.innerHTML = 'Start 1fps Record'
            rec_tlButton.style.background = '#F0F0F0'

            filemanButton.disabled = false
            deleteButton.disabled = false
            recordButton.disabled = false
            rec_tlButton.disabled = false
            document.getElementById('freeform').value = state.IP + " " + state.rssi + '\n' + 'Recording ' + state.OnOff + ', Power: ' + state.Power + '\n' + 'SD Free: ' + state.Free + 'MB/' + state.Size + 'MB  ' + String((100 * (state.Free / state.Size)).toFixed(1)) + '% free'
         }
      })
}


function clear_the_button() {
   document.getElementById('video').style.background = '#f0f0f0';
   document.getElementById('difference').style.background = '#F0F0F0';
   document.getElementById('distance').style.background = '#F0F0F0';
   document.getElementById('pulsing').style.background = '#F0F0F0';
   document.getElementById('flames').style.background = '#F0F0F0';
   document.getElementById('time').style.background = '#F0F0F0';
   document.getElementById('blocktime').style.background = '#F0F0F0';
   document.getElementById('berlintime').style.background = '#F0F0F0';
   document.getElementById('colors').style.background = '#F0F0F0';
   document.getElementById('life').style.background = '#F0F0F0';
   document.getElementById('calm').style.background = '#F0F0F0';
   document.getElementById('clive').style.background = '#F0F0F0';
   document.getElementById('snake').style.background = '#F0F0F0';
   document.getElementById('immortal').style.background = '#F0F0F0';
   document.getElementById('tron').style.background = '#F0F0F0';
}

function set_the_button() {
   const query3 = `${baseHost}/which`


   fetch(query3)
      .then(function (response) {
         return response.json()
      })
      .then(function (state) {
         var x = state.toString()
         console.log(`request to ${query3} finished, which: ${state.effect}`)
         document.getElementById('video').style.background = '#f0f0f0';
         document.getElementById('difference').style.background = '#F0F0F0';
         document.getElementById('distance').style.background = '#F0F0F0';
         document.getElementById('pulsing').style.background = '#F0F0F0';
         document.getElementById('flames').style.background = '#F0F0F0';
         document.getElementById('time').style.background = '#F0F0F0';
         document.getElementById('blocktime').style.background = '#F0F0F0';
         document.getElementById('berlintime').style.background = '#F0F0F0';
         document.getElementById('colors').style.background = '#F0F0F0';
         document.getElementById('life').style.background = '#F0F0F0';
         document.getElementById('calm').style.background = '#F0F0F0';
         document.getElementById('clive').style.background = '#F0F0F0';
         document.getElementById('snake').style.background = '#F0F0F0';
         document.getElementById('immortal').style.background = '#F0F0F0';
         document.getElementById('tron').style.background = '#F0F0F0';

         if (state.effect == 0) {
            document.getElementById('video').style.background = '#E8B1EA';
         }
         if (state.effect == 1) {
            document.getElementById('difference').style.background = '#E8B1EA';
         }
         if (state.effect == 2) {
            document.getElementById('distance').style.background = '#E8B1EA';
         }
         if (state.effect == 3) {
            document.getElementById('pulsing').style.background = '#E8B1EA';
         }
         if (state.effect == 4) {
            document.getElementById('flames').style.background = '#E8B1EA';
         }
         if (state.effect == 5) {
            document.getElementById('time').style.background = '#E8B1EA';
         }
         if (state.effect == 13) {
            document.getElementById('blocktime').style.background = '#E8B1EA';
         }
         if (state.effect == 14) {
            document.getElementById('berlintime').style.background = '#E8B1EA';
         }

         if (state.effect == 6) {
            document.getElementById('life').style.background = '#E8B1EA';
         }
         if (state.effect == 7) {
            document.getElementById('clive').style.background = '#E8B1EA';
         }
         if (state.effect == 8) {
            document.getElementById('snake').style.background = '#E8B1EA';
         }
         if (state.effect == 9) {
            document.getElementById('immortal').style.background = '#E8B1EA';
         }
         if (state.effect == 10) {
            document.getElementById('tron').style.background = '#E8B1EA';
         }
         if (state.effect == 11) {
            document.getElementById('colors').style.background = '#E8B1EA';
         }
         if (state.effect == 12) {
            document.getElementById('calm').style.background = '#E8B1EA';
         }
         if (state.effect == 13) {
            document.getElementById('blocktime').style.background = '#E8B1EA';
         }
         if (state.effect == 14) {
            document.getElementById('berlintime').style.background = '#E8B1EA';
         }
      })
}


function initialize() {


   fupdateStatus()
   set_the_button()
   const query = `${baseHost}/time?t=`
   const x = new Date();
   var timing = x.getTime() / 1000;
   const query2 = query + String(timing)

   fetch(query2)
      .then(response => {
         console.log(`request to ${query2} finished, status: ${response.status}`)
      })
}   
   </script>
      </head>
       <body onload="initialize()" style="background-color: white">
        
         <div id="second">
            <h2>JamCam Selfie VideoCam</h2>
            
            <h3>Video Recorder With Viewfinder, Streamer, Clock, ... and Snakes!</h3>
            <button id="toggle-stream">Start Stream</button>
            <button id="toggle-bmp-stream">Start Display Stream</button>
            <hr>
            <h3>Viewer Modes</h3>
            <button id="video"> Video </button>
            <button id="difference">Difference</button>
            <button id="distance">Distance</button>
            <button id="pulsing">Pulsing</button>
            <button id="flames">Flames</button>
            <br>
            <button id="time">Time</button>
            <button id="blocktime">BlockTime</button>
            <button id="berlintime">BerlinTime</button>
            <br>
            <button id="colors">Colors</button>
            <button id="life">Life</button>
            <button id="clive">Blink</button>
            <button id="calm">Calm</button>
            <br>
            <button id="snake">Snakes</button>
            <button id="immortal">Immortal Snakes</button>
            <button id="tron">Tron / GO</button>
            <br><br>
            <button id="plow">Dim</button>
            <button id="pmed">Medium</button>
            <button id="phigh">Bright</button>
            <br><br>
          <div id="third">
          <img id="liveFeed2" style="font-family: monospace;  text-align: center;
            color: white; position: relative"
            alt="There seems to be a problem with the live feed..."/>
          <br>
          <img id="liveFeedBmp" style="font-family: monospace; text-align: center;
            color: white; width="240" height="240" position: relative"
            alt="There seems to be a problem with the live feed..."/>
          </div>
          <hr>
            <h3>Video Recording</h3>
            <button id="record">Start 24fps Record</button>
            <button id="rec_tl">Start 1 fps Recording</button>
            <button id="status">Refresh Record Status</button>
            <br>
            <textarea id="freeform" name="freeformname" rows="5" cols="40">
Click Record Status ...
             </textarea>

          </div>
            <hl>
            <h3>WiFi and Files</h3>
            
            <button id="fileman">Download / Delete / Edit / View Video</button><br><br>
            <button id="edit">Edit Wifi and password</button>
            <button id="delete">Delete All Video</button>
            <button id="reboot">Reboot Camera</button>
             <br>
          <br>
          
          <a href="https://github.com/jameszah/ESP32-CAM-JAMCAM">https://github.com/jameszah/ESP32-CAM-JAMCAM</a> <br>
          James Zahary - Mar 1, 2024 -- ver 68<br>
          <a href="https://ko-fi.com/jameszah">Free coffee </a>
            
    </body>
</html>
)===";
   
