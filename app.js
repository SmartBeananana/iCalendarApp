/*
*Name: Xiangyuan Lian
*Email: lianx@uoguelph.ca
*Date: March 28th, 2019
*/
'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ********************
const mysql = require('mysql');


let lib = ffi.Library('./libcal',{
  'EventListToJS':['string',['string']],
  'CalendarToJS':['string',['string']],
  'AlarmToJS':['string',['string','int']],
  'vaildateCalendarToJS':['string',['string']],
  'writeCalendarToJS':['string',['string','string','string','string','string']],
  'addEventToJS':['string',['string','string','string','string']],
  'PropToJS':['string',['string','int']]
});

var connection;

app.get('/displayupcomingevents',function(req,res){
  let today = new Date();
  let month = (today.getMonth()+1).toString().length;
  if(month < 2)
  {
      month = '0'+ month;
  }
  let d = today.getDate().toString().length;
  if(d < 2)
  {
      d = '0'+d;
  }

  let todayDate = today.getFullYear().toString()+"-"+ month +"-"+ d;
  console.log(todayDate);

  connection.query("select * from EVENT where start_time > \"2019-04-05 00:00:00\"",
  function(err,rows,fields){
    if(err){
      res.send({error:err});
    }
    else{
      console.log("upcoming"+rows.length);
      if(rows == undefined || rows == null) res.send({error:"Rows is undefined"});
      if(rows.length == 0) res.send({error:"Rows' length is 0"});
      else{
        let event_upcoming = new Array(rows.length);
        let i = 0;
        for(let row of rows){
          event_upcoming[i] = {"summary":row.summary,"start_time":row.start_time,"location":row.location,"organizer":row.organizer};
          i++;
        }
        res.send(event_upcoming);
      }
    }
  });

})

app.get('/displayalarmsbydates',function(req,res){
  connection.query("select e.start_time, e.location, a.action from EVENT e, ALARM a WHERE e.event_id = a.event ORDER BY start_time DESC",
  function(err,rows,fields){
    if(err){
      res.send({error:err});
    }
    else {
      if(rows == undefined || rows == null) res.send({error:"Rows is undefined"});
      if(rows.length == 0) res.send({error:"Rows' length is 0"});
      else{
        let event_bydates = new Array(rows.length);
        let i = 0;
        for(let row of rows){
          event_bydates[i] ={"start_time":row.start_time,"location":row.location,"action":row.action};
          i++;
        }
        res.send(event_bydates);
      }
    }
  });
})

app.get('/displayeventsinguelph',function(req,res){
  connection.query("SELECT * FROM EVENT WHERE location = \'guelph\'",
  function(err,rows,fields){
    if(err){
      res.send({error:err});
    }
    else{
      if(rows == undefined || rows == null) res.send({error:"Rows is undefined"});
      if(rows.length == 0) res.send({error:"Rows' length is 0"});
      console.log("inguelph "+rows.length);
      let event_inguelph = new Array(rows.length);
      let i = 0;
      for(let row of rows){
        event_inguelph[i] = {"summary":row.summary,"start_time":row.start_time,"location":row.location,"organizer":row.organizer};
        i++;
      }
      res.send(event_inguelph);
    }
  });
})

app.get('/displayconflictevents',function(req,res){
  connection.query("SELECT start_time FROM EVENT",
  function(err,rows,fields){
    if(err) {
      res.send({error:"Something wring "+err});
    }
    else{

      connection.query("select * from EVENT where start_time in (select start_time from EVENT GROUP BY start_time having count(*)>1)",
      function(err,rows,fields){
        if(err){
          res.send({error:err});
        }
        else{
          if(rows == undefined || rows == null) res.send({error:"Rows is undefined"});
          if(rows.length == 0) res.send({error:"Rows' length is 0"});
          let event_byconflict = new Array(rows.length);
          let i = 0;
          for(let row of rows){
            event_byconflict[i] = {"summary":row.summary,"start_time":row.start_time,"location":row.location,"organizer":row.organizer};
            i++;
          }
          res.send(event_byconflict);
        }

      })
    }
  });
})

app.get('/displayeventsbyfile',function(req,res){
  console.log("Hello");
  let filename = req.query.filename;
  connection.query("SELECT * FROM FILE WHERE file_Name = \'"+filename+"\'",
  function(err,rows,fields){
    if(err){
      res.send({error:"Somgthing wrong "+err});
    }
    else{
      if(rows == undefined || rows == null) res.send({error:"Rows is undefined"});
      if(rows.length == 0) res.send({error:"Rows' length is 0"});
      let calid = rows[0].cal_id;

      connection.query("SELECT * FROM EVENT WHERE cal_file = "+calid,
      function(err,rows,fields){
        if(err){
          console.log("Something wrong "+err);
          res.send({error:err});
        }
        else {
          if(rows == undefined || rows == null) res.send({error:"Rows is undefined"});
          if(rows.length == 0) res.send({error:"Rows' length is 0"});
          let event_byfile = new Array(rows.length);
          let i = 0;
          for(let row of rows){
            event_byfile[i] = {"summary":row.summary,"start_time":row.start_time,"location":row.location,"organizer":row.organizer};
            i++;
          }
          res.send(event_byfile);
        }
      });
    }

  });

})

app.get('/DBfilenames',function(req,res){
  connection.query("SELECT file_Name FROM FILE",
  function(err,rows,fields){
    if(err){
      console.log("Something wrong "+err);
    }
    else {
      let filename = new Array(rows.length);
      let i=0;
      for(let row of rows)
      {
        filename[i] = row.file_Name;
        i++;
      }
      res.send(filename);
    }
  });
})

app.get('/displayeventsbydates',function(req,res){
  connection.query("SELECT * FROM EVENT ORDER BY start_time",
  function(err,rows,fields){
    if(err){
      console.log("Something wrong "+err);
    }
    else {
      let event_bydates = new Array(rows.length);
      let i = 0;
      for(let row of rows){
        event_bydates[i] ={"event_id":row.event_id,"summary":row.summary,"start_time":row.start_time,"location":row.location,"organizer":row.organizer,"cal_file":row.cal_file};
        i++;
      }
      res.send(event_bydates);
    }
  });
});

app.get('/displaydatabase',function(req,res){
  let FILElen = 0;
  let EVENTlen = 0;
  let ALARMlen = 0;
  connection.query("select * from FILE",
  function(err,rows,fields){
    if(err){
      console.log("Something wrong "+err);
    }
    else{
      FILElen = rows.length;
      connection.query("select * from EVENT",
      function(err,rows,fields){
        if(err){
          console.log("Something wrong "+err);
        }
        else {
          EVENTlen = rows.length;
          connection.query("select * from ALARM",
          function(err,rows,fileds){
            if(err){
              console.log("Something wrong "+err);
            }
            else{
              ALARMlen = rows.length;
              res.send({filelen:FILElen,eventlen:EVENTlen,alarmlen:ALARMlen});
            }
          });
        }
      });
    }
  });
});

app.get('/populateDBtable',function(req,res){
  let filenames = fs.readdirSync('./uploads');
  let len = filenames.length;
  let jsonstructCal = new Array(len);

  connection.query("create table IF NOT EXISTS FILE (cal_id INT AUTO_INCREMENT, file_Name VARCHAR(60) NOT NULL, version INT NOT NULL, prod_id VARCHAR(256) NOT NULL, PRIMARY KEY (cal_id))",
  function(err,rows,fields){
    if(err) res.send({error:err});
  });

  connection.query("create table IF NOT EXISTS EVENT (event_id INT AUTO_INCREMENT, summary VARCHAR(1024), start_time DATETIME NOT NULL, location VARCHAR(60), organizer VARCHAR(256), cal_file INT NOT NULL, PRIMARY KEY (event_id),FOREIGN KEY(cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE)",
  function(err,rows,fields){
    if(err) res.send({error:err});
  });

  connection.query("create table IF NOT EXISTS ALARM (alarm_id INT AUTO_INCREMENT, action VARCHAR(256) NOT NULL, `trigger` VARCHAR(256) NOT NULL, event INT NOT NULL, PRIMARY KEY (alarm_id), FOREIGN KEY(event) REFERENCES EVENT(event_id) ON DELETE CASCADE)",
  function(err,rows,fields){
    if(err) res.send({error:err});
  });

var counter = 0;
  for(let i=0;i<len;i++)
  {
    let jsonCal = lib.CalendarToJS('./uploads/'+filenames[i]);
    jsonstructCal[i] = JSON.parse(jsonCal);
    connection.query("SELECT COUNT(*) AS filenameCount FROM FILE WHERE file_Name = \'"+filenames[i]+"\'",
    function(err,rows,fields){
      if(err)
      {
        console.log("ERROR"+err);
        res.send({error:err});
      }
      else{
        if(rows[0].filenameCount == 0)
        {

          connection.query("INSERT INTO FILE (file_Name, version, prod_id) VALUES (\'"+filenames[i]+"\',"+Number(jsonstructCal[i].version)+",\'"+jsonstructCal[i].prodID+"\')",
          function(err,row,fields){
            if(err) {
              console.log("Something went wrong "+ err);
            }
            else{
              let strEvent = lib.EventListToJS('./uploads/'+filenames[i]);
              let jsonEvent = JSON.parse(strEvent);
              let lenOfEventList = jsonEvent.length;
              //counter++;
              for(let j=0;j<lenOfEventList;j++)
              {
                connection.query("SELECT cal_id FROM FILE WHERE file_Name = \'"+filenames[i]+"\'",
                function(err,rows,fields){
                  if(err) {
                    console.log("Something went wrong in event list "+ err);
                  }
                  else
                  {
                      connection.query("INSERT INTO EVENT (summary, start_time, location, organizer, cal_file) VALUES (\'"+
                      jsonEvent[j].summary+"\',\'"+jsonEvent[j].startDT.date[0]+jsonEvent[j].startDT.date[1]+
                      jsonEvent[j].startDT.date[2]+jsonEvent[j].startDT.date[3]+"/"+jsonEvent[j].startDT.date[4]+
                      jsonEvent[j].startDT.date[5]+"/"+jsonEvent[j].startDT.date[6]+jsonEvent[j].startDT.date[7]+
                      " "+jsonEvent[j].startDT.time[0]+jsonEvent[j].startDT.time[1]+":"+jsonEvent[j].startDT.time[2]+
                      jsonEvent[j].startDT.time[3]+":"+jsonEvent[j].startDT.time[4]+jsonEvent[j].startDT.time[5]+"\',\'"+
                      jsonEvent[j].location +"\',\'"+jsonEvent[j].organizer +"\',"+rows[0].cal_id+")",
                      function(err,rows,fields){
                        if(err){
                          console.log("Something wrong "+ err);
                        }
                        else
                        {

                          let strAlarm = lib.AlarmToJS('./uploads/'+filenames[i],j+1);
                          let jsonAlarm = JSON.parse(strAlarm);
                          let lenOfAlarmList = jsonAlarm.length;

                          for(let k=0;k<lenOfAlarmList;k++)
                          {
                            connection.query("SELECT * FROM EVENT",
                            function(err,rows,fields){
                              if(err){
                                res.send({error:err});
                              }
                              else
                              {
                                connection.query("INSERT INTO ALARM (action, `trigger`, event) VALUES (\'"+jsonAlarm[k].Action+"\',\'"+jsonAlarm[k].Trigger+"\',"+rows[j].event_id+")",
                                function(err,rows,fields){
                                  if(err)
                                  {
                                    console.log(err);
                                  }
                                });
                              }
                            });
                          }
                        }
                      });
                    //}
                  }
                });
              }
              counter++;
            }
          });
        }
      }
    });
  }
  res.send({success:"Populate the table successfully"});
});

app.get('/clearalldata',function(req,res){
  connection.query("DELETE FROM FILE",function(err,rows, fields){
    if(err)
    {
      console.log("Something wrong "+err);
    }
  });
});


app.get('/userlogin',function(req,res){
  let username = req.query.username;
  let password = req.query.password;
  let database = req.query.database;

  console.log("username "+username);
  console.log("password "+password);
  console.log("database "+database);

  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : username,
    password : password,
    database : database,
  });

  connection.connect(function(err){
    if(err)
    {
      console.log("Error "+err);
      res.send({error: 'Can not connect to the server'});
    }
    else
    {
      res.send({success: 'connected to database successfully'});
    }
  });
});

app.get('/somefilenames', function(req , res){
  let filenames = fs.readdirSync('./uploads');
  res.send(filenames)
});

app.get('/filelog', function(req , res){
  let filenames = fs.readdirSync('./uploads');
  let len = filenames.length;
  let jsonstruct = new Array(len);
  let ret = new Array(len);

  for(let i=0;i<len;++i)
  {
    let jsonstr = lib.CalendarToJS('./uploads/'+filenames[i]);
    jsonstruct[i] = JSON.parse(jsonstr);
    ret[i] = {"filename":filenames[i],"version":jsonstruct[i].version,"prodID":jsonstruct[i].prodID,"numProps":jsonstruct[i].numProps,"numEvents":jsonstruct[i].numEvents};
  }
  res.send(ret);
});

app.get('/events/:file',function(req,res){
  var filename = req.params.file;
  var jsonstr = lib.EventListToJS('./uploads/'+filename);

  res.send(jsonstr);
});

app.get('/createCalendar',function(req,res){
  let json = req.query;
  let filename = "./uploads/"+json.filename;
  let temp = json.filename;
  let filenameExits = fs.readdirSync('./uploads');
  console.log(temp +"<br>");
  console.log("filename: "+  filenameExits);
  for(let i=0; i<filenameExits.length; i++)
  {
    if(temp == filenameExits[i])
    {
      res.send({errorFilename:"The filename already exists!!!"});
      return;
    }
  }

  let jsoniCal = { version:Number(json.version),
    prodID:json.prodID};
    let jsonEvent = "{\"UID\":\""+json.uid+"\",\"summary\":\""+json.summary+"\"}";

    let jsonDTSTART = { date: json.date,
      time: json.time,
      isUTC: json.utc == 'true' ? true : false,};

      let jsonDTSTAMP = {date:json.todaydate,
        time:json.todaytime,
        isUTC: true};

        let striCal = JSON.stringify(jsoniCal);
        let strEvent = jsonEvent;
        let strDTSTART = JSON.stringify(jsonDTSTART);
        let strDTSTAMP = JSON.stringify(jsonDTSTAMP);

        let errCode = lib.writeCalendarToJS(filename,striCal,strEvent,strDTSTART,strDTSTAMP);

        if(errCode != "OK")
        {
          if(errCode == "INV_DT")
          {
            res.send({errorDT: 'The date and time is invalid!'});
          }
          else if(errCode == "INV_EVENT")
          {
            res.send({errorEVENT: 'The event is invalid!'});
          }
          else
          {
            res.send({error: 'Can not create the calendar!'});
          }
        }
        else {
          res.send("OK");
        }
      });

      app.get('/addEvent',function(req,res){
        let json = req.query;
        let filename = "./uploads/"+json.filename;
        console.log("ADDEVENT: "+filename);
        let jsonEvent = "{\"UID\":\""+json.uid+"\",\"summary\":\""+json.summary+"\"}";

        let jsonDTSTART = { date: json.date,
          time: json.time,
          isUTC: json.utc == 'true' ? true : false,};

          let jsonDTSTAMP = {date:json.todaydate,
            time:json.todaytime,
            isUTC: true};

            let strEvent = jsonEvent;
            let strDTSTART = JSON.stringify(jsonDTSTART);
            let strDTSTAMP = JSON.stringify(jsonDTSTAMP);

            let errCode = lib.addEventToJS(filename,strEvent,strDTSTART,strDTSTAMP);

            if(errCode != "OK")
            {
              if(errCode == "INV_DT")
              {
                res.send({errorDT: 'The date and time is invalid!'});
              }
              else if(errCode == "INV_EVENT")
              {
                res.send({errorEVENT: 'The event is invalid!'});
              }
              else
              {
                res.send({error: 'Can not create the event!'});
              }
            }
            else {
              res.send("OK");
            }
          });

          app.get('/showalarms', function(req,res){
            console.log("Got In showalarms");
            let json = req.query;
            let filename = "./uploads/"+json.filename;
            let eventno = Number(json.eventno);

            let alarmList = lib.AlarmToJS(filename,eventno);
            let jsonAlarm = JSON.parse(alarmList);


            if(jsonAlarm.length == 0)
            {
              res.send({error: 'There is no alarm in this event!'});
            }
            else {
              res.send(jsonAlarm);
            }
          });

          app.get('/showprops', function(req,res){
            let json = req.query;
            let filename = "./uploads/"+json.filename;
            let eventno = Number(json.eventno);

            let propList = lib.PropToJS(filename,eventno);

            if(json.length == 0)
            {
              res.send({error: 'There is no optional properties in this event!'});
            }
            else {
              res.send(propList);
            }
          });
          //Sample endpoint

          app.listen(portNum);
          console.log('Running app at localhost: ' + portNum);
