/*
 *Name: Xiangyuan Lian
 *Email: lianx@uoguelph.ca
 *Date: March 28th, 2019
 */
// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
  disableButton();

  $.ajax({
         type: 'get',
         dataType: 'json',
         url: '/filelog',
         success: function (data) { //if received the data successful

             let len = data.length;

             function addRowFileLog(tableID,content)
             {
                let tableRef = document.getElementById(tableID);
                let newRow = tableRef.insertRow(-1);

                let newCell0 = newRow.insertCell(0);
                let a = document.createElement('a');
                let linkText = document.createTextNode(content.filename);
                a.appendChild(linkText);
                a.title = content.filename;
                a.href = '/uploads/'+content.filename;
                newCell0.appendChild(a);

                let newCell1 = newRow.insertCell(1);
                let newText1 = document.createTextNode(content.version);
                newCell1.appendChild(newText1);

                let newCell2 = newRow.insertCell(2);
                let newText2 = document.createTextNode(content.prodID);
                newCell2.appendChild(newText2);

                let newCell3 = newRow.insertCell(3);
                let newText3 = document.createTextNode(content.numEvents);
                newCell3.appendChild(newText3);

                let newCell4 = newRow.insertCell(4);
                let newText4 = document.createTextNode(content.numProps);
                newCell4.appendChild(newText4);
             }
             for(let i=0;i<len;++i)
             {
                addRowFileLog('filelog_table',data[i]);
             }
             $('#meg').append("Just created the file log table"+"<br>");
         },

         fail: function(error) {
             $('#meg').html(error);
         }
         });

   $.ajax({
          type: 'get',
          dataType: 'json',
          url: '/somefilenames',
          success: function (data) { //if received the data successful

              let len = data.length;


              for(let i=0;i<len;++i)  //add filenames to the drop down list
              {
                  let filename_list = document.getElementById('filename_list');
                  let addEvent_filename_list = document.getElementById('addEvent_filename_list');
                  addFilename(filename_list,data[i],data[i]);
                  addFilename(addEvent_filename_list,data[i],data[i]);
              }
              $('#meg').append("Files have been loaded"+"<br>");
          },
          fail: function(error) {
              $('#meg').html(error);
          }
          });

  document.getElementById("filename_list").onchange = function(){
      let file = this.value;
      $('#meg').append("The file "+file+" has been selected<br>");
      console.log(file);
      $.ajax({
             type: 'get',
             dataType: 'json',
             url: '/events/'+file,
             success: function(data) {
                  let tableRef = document.getElementById("calendarview_table");
                  let len = data.length;
                  tableRef.innerHTML = ''
                      +'<thead>'
                          +'<tr>'
                              +'<th><center>Alarms</th>'
                              +'<th><center>Optional<br>Properties</th>'
                              +'<th><center>Event No</th>'
                              +'<th><center>Start Date</th>'
                              +'<th><center>Start Time</th>'
                              +'<th><center>Summary</th>'
                              +'<th><center>Props</th>'
                              +'<th><center>Alarms</th>'
                          +'</tr>'
                     +'</thead>';

                  for(let i=0;i<len;++i)
                  {
                        let eventno = i+1;

                        let newRow = tableRef.insertRow(-1);

                        let showalarm = document.createElement("input");
                        showalarm.type = "button";
                        showalarm.value = "Show Alarms";
                        showalarm.addEventListener("click", function(){
                              $.ajax({
                                    type: 'get',
                                    dataType: 'json',
                                    url: '/showalarms',
                                    data:{eventno: eventno,
                                          filename: file},
                                    success:function(alarm){

                                          if(alarm.hasOwnProperty('error')){
                                             $('#meg').append("***" + alarm.error + "***<br>");
                                          }
                                          else{
                                             $('#meg').append("Display the alarm successfully<br>");
                                             $('#meg').append("----------------------------------<br>");

                                              let ret = "***There are " + alarm.length +" alarms:<br>";

                                              for(let i=0;i<alarm.length;++i)
                                              {
                                                  ret = ret +"The action is : "+ alarm[i].Action +
                                                      ", and the trigger is: " + alarm[i].Trigger + ".<br>" ;
                                              }
                                              $('#meg').append(ret);
                                              $('#meg').append("----------------------------------<br>");
                                          }
                                    },
                                    fail: function(error) {
                                      console.log("Alarm fail");
                                        $('#meg').append("Alarm wrong<br>");
                                    },
                              });
                        });

                        let newCell0 = newRow.insertCell(0);
                        newCell0.appendChild(showalarm);

                        let showprop = document.createElement("input");
                        showprop.type = "button";
                        showprop.value = "Show Properties";
                        showprop.addEventListener("click", function(){
                              $.ajax({
                                    type: 'get',
                                    dataType: 'json',
                                    url: '/showprops',
                                    data:{eventno: eventno,
                                          filename: file},
                                    success:function(props){
                                          if(props.hasOwnProperty('error')){
                                              $('#meg').append("***"+props.error+"***<br>");
                                          }
                                          else{
                                              $('#meg').append("Display the property successfully<br>");
                                              $('#meg').append("----------------------------------<br>");

                                              let ret = "***There are " + props.length +" optional properties:<br>";
                                              $('#meg').append(ret);
                                              for(let i=0;i<props.length;++i)
                                              {
                                                  let jsonProps = JSON.stringify(props[i]);
                                                  jsonProps = jsonProps.replace(/\"/g,"");
                                                  jsonProps = jsonProps.replace(/\{/g,"");
                                                  jsonProps = jsonProps.replace(/\}/g,"");
                                                  console.log(jsonProps);
                                                  $('#meg').append(jsonProps+"<br>");
                                              }
                                              $('#meg').append("----------------------------------<br>");
                                          }
                                    },
                                    fail:function(error){
                                         $('#meg').append("Can not display the property !!!<br>");
                                    }
                              })
                        })
                        let newCell1 = newRow.insertCell(1);
                        newCell1.appendChild(showprop);

                        let newCell2 = newRow.insertCell(2);
                        let newText2 = document.createTextNode(eventno);
                        newCell2.appendChild(newText2);

                        let newDate = data[i].startDT.date[0]+data[i].startDT.date[1]+
                                      data[i].startDT.date[2]+data[i].startDT.date[3]+"/"+
                                      data[i].startDT.date[4]+data[i].startDT.date[5]+"/"+
                                      data[i].startDT.date[6]+data[i].startDT.date[7];
                        let newCell3 = newRow.insertCell(3);
                        let newText3 = document.createTextNode(newDate);
                        newCell3.appendChild(newText3);

                        let time = data[i].startDT.time[0]+data[i].startDT.time[1]+":"+
                                   data[i].startDT.time[2]+data[i].startDT.time[3]+":"+
                                   data[i].startDT.time[4]+data[i].startDT.time[5];
                        let newTime;
                        if(data[i].startDT.isUTC == true)
                        {
                            newTime = time + "(UTC)";
                        }
                        else
                        {
                            newTime = time;
                        }

                        let newCell4 = newRow.insertCell(4);
                        let newText4 = document.createTextNode(newTime);
                        newCell4.appendChild(newText4);

                        let newCell5 = newRow.insertCell(5);
                        let newText5 = document.createTextNode(data[i].summary);
                        newCell5.appendChild(newText5);

                        let newCell6 = newRow.insertCell(6);
                        let newText6 = document.createTextNode(data[i].numProps);
                        newCell6.appendChild(newText6);

                        let newCell7 = newRow.insertCell(7);
                        let newText7 = document.createTextNode(data[i].numAlarms);
                        newCell7.appendChild(newText7);
                  }
                  $('#meg').append("Calendar view table has been created<br>");
             },
             fail: function(error){
                  $('#meg').html("Calendar view table can not be created<br>");
             }
      });
    };
   });

    $('#login').submit(function(e){
        let username = document.getElementById('username').value;
        let password = document.getElementById('password').value;
        let database = document.getElementById('database').value;

        e.preventDefault();
        $.ajax({
          type:'get',
          url:'/userlogin',
          dataType:'',
          data:{username : username,
                password : password,
                database : database},
          success:function(data){
             if(data.hasOwnProperty('error'))
             {
                 $('#meg').html("---Can not connect to the database---");
                 console.log("Cannot connect to the database");
             }
             else
             {
                 $('#meg').html("Connected to database successfully!!!");
                 console.log("Connected to the database");
                 document.getElementById('storefiles').disabled = false;
                 document.getElementById('cleardata').disabled = false;
                 document.getElementById('displaystatus').disabled = false;
                 document.getElementById('executequery').disabled = false;
                 document.getElementById('selquery').disabled = false;
             }
          },
          fail:function(error){

          }
        });
    });

    $('#createCalendar').submit(function(e){
        let filename = document.getElementById('filename').value;
        let version = document.getElementById('version').value;
        let prodID = document.getElementById('prodID').value;
        let uid = document.getElementById('uid').value;
        let summary = document.getElementById('summary').value;
        let date = document.getElementById('date').value;
        let time = document.getElementById('time').value;
        let utc = document.getElementById('utc').value;

        let todaydate = getCurrentDate();
        let todaytime = getCurrentTime();

        $('#meg').append("Creating the calendar...<br>");
        e.preventDefault();
        $.ajax({
           type:'get',
           url:'/createCalendar',
           dataType:'json',
           data: {filename: filename,
                  version: version,
                  prodID: prodID,
                  uid: uid,
                  summary: summary,
                  date: date,
                  time: time,
                  utc: utc,
                  todaydate: todaydate,
                  todaytime: todaytime},
           success: function(data){
             if(data.hasOwnProperty('error')){
                $('#meg').append(error + "<br>");
             }
             else if(data.hasOwnProperty('errorFilename')){
                $('#meg').append(data.errorFilename);
             }
             else if(data.hasOwnProperty('errorDT'))
             {
                $('#meg').append(data.errorDT);
             }
             else if(data.hasOwnProperty('errorEVENT'))
             {
                $('#meg').append(data.errorEVENT);
             }
             else{
                $('#meg').append("The calendar have been created successfully<br>");
             }
           },
           fail:function(error){
               $('#meg').append(error);
           }

        });
    })

document.getElementById("addEvent_filename_list").onchange = function(){
    let filename = this.value;
    console.log(filename);
    $('#addEvent').submit(function(e){
        let uid = document.getElementById('uidAE').value;
        let summary = document.getElementById('summaryAE').value;
        let date = document.getElementById('dateAE').value;
        let time = document.getElementById('timeAE').value;
        let utc = document.getElementById('utcAE').value;

        let todaydate = getCurrentDate();
        let todaytime = getCurrentTime();

        $('#meg').append("Adding the event...<br>");
        e.preventDefault();
        $.ajax({
          type:'get',
          url:'/addEvent',
          dataType:'json',
          data: {filename: filename,
                 uid: uid,
                 summary: summary,
                 date: date,
                 time: time,
                 utc: utc,
                 todaydate: todaydate,
                 todaytime: todaytime},
                 success:function(data){
                     if(data.hasOwnProperty('error')){
                        $('#meg').append("Can not add the event!!!-----<br>");
                     }
                     else if(data.hasOwnProperty('errorDT'))
                     {
                        $('#meg').append(data.errorDT);
                     }
                     else if(data.hasOwnProperty('errorEVENT'))
                     {
                        $('#meg').append(data.errorEVENT);
                     }
                     else{
                        $('#meg').append("Event has been added successfully<br>");
                     }
                 },
                 fail:function(error){
                    $('#meg').html(error);
                 }
    });
  });
};

//==============================================
//functions
function addFilename(form,text,value)
{
    let opt = document.createElement('option');
    opt.text = opt.value = value;
    form.options.add(opt);
}

function getCurrentDate()
{
    let today = new Date();
    let month = (today.getMonth()+1).toString();
    if(month.length < 2)
    {
        month = '0'+ month;
    }
    let d = today.getDate().toString();
    if(d < 2)
    {
        d = '0'+d;
    }

    let todayDate = today.getFullYear().toString()+ month +d;

    return todayDate;
};

function getCurrentTime()
{
    let today = new Date();
    let hour = today.getHours().toString();
    if(hour.length < 2)
    {
       console.log("hour length"+ hour.length);
       hour = '0'+hour;
    }
    let minute = today.getMinutes().toString();
    if(minute.length < 2)
    {
        minute = '0'+minute;
    }
    let second = today.getSeconds().toString();
    if(second.length < 1)
    {
       second = '0'+second;
    }

    let todayTime = hour + minute + second;

    return todayTime;
};

function clearBox(){
    document.getElementById("meg").innerHTML = "";
};

function disableButton(){
    document.getElementById('storefiles').disabled = true;
    document.getElementById('cleardata').disabled = true;
    document.getElementById('displaystatus').disabled = true;
    document.getElementById('executequery').disabled = true;
    document.getElementById('selquery').disabled = true;
};

function queryFilenameList()
{
    $.ajax({
           type: 'get',
           dataType: 'json',
           url: '/DBfilenames',
           success: function (data) {

               let len = data.length;

               for(let i=0;i<len;++i)
               {
                   let query_filename_list = document.getElementById('query_filename_list');
                   addFilename(query_filename_list,data[i],data[i]);
               }
               $('#meg').append("Get filename from database"+"<br>");
           },
           fail: function(error) {
               $('#meg').html(error);
           }
           });
}

function DBStoreFiles(){
    console.log("hello");
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/populateDBtable',
      success:function(data){

         queryFilenameList();

      },
      fail:function(error){

      }
    });
};

function DBClearData(){
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/clearalldata',
      success:function(data){

      },
      fail:function(error){

      }
    })
}

function DBDisplayStatus(){
   $.ajax({
     type: 'get',
     dataType: 'json',
     url:'/displaydatabase',
     success:function(data){
          console.log("filelen "+ data.filelen);
          console.log("eventfile "+ data.eventlen);
          console.log("alarmfile "+ data.alarmlen);
         //console.log("FILElen "+ JSON.parse(JSON.parse(data)));
         if(data.hasOwnProperty("filelen"))
         {
             console.log("YES!!!");
         }
         $('#meg').append("Database has "+data.filelen+" files, "+data.eventlen+" events, "+data.alarmlen+" alarms.");
     },
     fail:function(err){

     }
   })
}

function DBExecuteQuery(){
   let selquery = document.getElementById("selquery").value;

   if(selquery == "events_bydates"){
        display_events_by_dates();
   }
   else if(selquery == "events_from_file")
   {
        display_events_from_A_file();
   }
   else if(selquery == "conflict_events")
   {
        display_conflict_events();
   }
   else if(selquery == "events_in_guelph")
   {
        display_events_in_guelph();
   }
   else if(selquery == "alarm_bydates")
   {
        display_alarms_by_dates();
   }
   else if(selquery == "upcoming_event")
   {
        display_upcoming_events();
   }

}

function display_event_table(data){
  let tableRef = document.getElementById("execute_table");
  let len = data.length;
  tableRef.innerHTML = ''
      +'<thead>'
          +'<tr>'
              +'<th><center>Event No</th>'
              +'<th><center>Summary</th>'
              +'<th><center>Start time</th>'
              +'<th><center>Location</th>'
              +'<th><center>Organizer</th>'
          +'</tr>'
     +'</thead>';

   for(let i=0;i<len;++i)
   {
       let eventno = i+1;

       let newRow = tableRef.insertRow(-1);

       let newCell0 = newRow.insertCell(0);
       let newText0 = document.createTextNode(eventno);
       newCell0.appendChild(newText0);

       let newCell1 = newRow.insertCell(1);
       let newText1 = document.createTextNode(data[i].summary);
       newCell1.appendChild(newText1);

       let newCell2 = newRow.insertCell(2);
       let newText2 = document.createTextNode(data[i].start_time);
       newCell2.appendChild(newText2);

       let newCell3 = newRow.insertCell(3);
       let newText3 = document.createTextNode(data[i].location);
       newCell3.appendChild(newText3);

       let newCell4 = newRow.insertCell(4);
       let newText4 = document.createTextNode(data[i].organizer);
       newCell4.appendChild(newText4);
   }
}

function display_events_by_dates()
{
   $.ajax({
     type:'get',
     dataType:'json',
     url:'/displayeventsbydates',
     success:function(data){
         console.log(data[0].event_id);
         display_event_table(data);
     },
     fail:function(err){

     }
   })
}

function display_events_from_A_file(){
    let filename = document.getElementById('query_filename_list').value;
    console.log("The temp is "+ filename);

     $.ajax({
       type:'get',
       dataType:'json',
       url:'/displayeventsbyfile',
       data:{filename:filename},
       success:function(data){
         console.log("Hello");
         console.log("data length: "+data.length);

         display_event_table(data);

       },
       fail:function(err){

       }
     })
}

function  display_conflict_events(){
   $.ajax({
     type:'get',
     dataType:'json',
     url:'/displayconflictevents',
     success:function(data){
        display_event_table(data);
     },
     fail:function(err){

     }
   })
}

function display_events_in_guelph(){
  $.ajax({
    type:'get',
    dataType:'json',
    url:'/displayeventsinguelph',
    success:function(data){

       display_event_table(data);
    },
    fail:function(err){

    }
  })
}

function display_alarms_by_dates(){
  $.ajax({
    type:'get',
    dataType:'json',
    url:'/displayalarmsbydates',
    success:function(data){

      if(data.hasOwnProperty('error')){
         $('#meg').append("Something wrong with display alarm by dates !!!-----<br>");
      }
      let tableRef = document.getElementById("execute_table");
      let len = data.length;
      tableRef.innerHTML = ''
          +'<thead>'
              +'<tr>'
                  +'<th><center>Start time</th>'
                  +'<th><center>Location</th>'
                  +'<th><center>Action</th>'
              +'</tr>'
         +'</thead>';

       for(let i=0;i<len;++i)
       {
           let eventno = i+1;

           let newRow = tableRef.insertRow(-1);

           let newCell0 = newRow.insertCell(0);
           let newText0 = document.createTextNode(data[i].start_time);
           newCell0.appendChild(newText0);

           let newCell1 = newRow.insertCell(1);
           let newText1 = document.createTextNode(data[i].location);
           newCell1.appendChild(newText1);

           let newCell2 = newRow.insertCell(2);
           let newText2 = document.createTextNode(data[i].action);
           newCell2.appendChild(newText2);
       }
    },
    fail:function(err){

    }
  })
}

function display_upcoming_events(){
  $.ajax({
    type:'get',
    dataType:'json',
    url:'/displayupcomingevents',
    success:function(data){
      console.log(data.length);
       display_event_table(data);
    },
    fail:function(err){

    }
  })
}
