
import ConfigParser
import os, sys, re
  
conf = sys.argv[1]
act = sys.argv[2]
#print "conf = " + conf + ", act=" + act

vsConfs = {}

item = {}
item["section"] = "VSConf"
item["default"] = "1G"
item["regex"] = "(\d+[p|P|t|T|g|G|m|M|k|K])"
item["description"] = "The total size of file to trigger backup to tape"
item["format"] =  "num+unit"
vsConfs["WriteToTapeWaitSize"] = item;

item1 = {}
item1["section"] = "VSConf"
item1["default"] = "180"
item1["regex"] = "(\d+)"
item1["description"] = "The time past since last backup to trigger a new backup"
item1["format"] = "seconds"
vsConfs["WriteToTapeWaitTime"] = item1;

item2 = {}
item2["section"] = "VSConf"
item2["default"] = "4G"
item2["regex"] = "(\d+[p|P|t|T|g|G|m|M|k|K])"
item2["description"] = "The size a backup thread will backup"
item2["format"] = "num+unit"
vsConfs["ThreadWriteToTapeSize"] = item2;

item3 = {}
item3["section"] = "VSConf"
item3["default"] = "01:05"
item3["regex"] = "([0|\d]\d\:[0|\d]\d)"
item3["description"] = "Time for tape auditor to start at"
item3["format"] = "HH:MM"
vsConfs["TapeAuditorRun"] = item3;

item4 = {}
item4["section"] = "VSConf"
item4["default"] = "2"
item4["regex"] = "(\d+)"
item4["description"] = "Maximum tape auditor task at a time"
item4["format"] = "integer"
vsConfs["TapeAuditorMaxnum"] = item4;

item5 = {}
item5["section"] = "VSConf"
item5["default"] = "3"
item5["regex"] = "(\d+)"
item5["description"] = "Number of days tape auditor can be started again since last start"
item5["format"] = "integer"
vsConfs["TapeAuditorInterval"] = item5;

item6 = {}
item6["section"] = "VSConf"
item6["default"] = "true"
item6["regex"] = "(true|false)"
item6["description"] = "Tape auditor is enabled or not"
item6["format"] = "true/false"
vsConfs["TapeAuditorEnable"] = item6;

content = ""
with open(conf, 'r') as fp:
    content = fp.read()
fp.close()
    
if act != "uninstall":
    #print "Please setup the configuration for swift VS:" 
    newConf = "\n[VSConf]"
    for key in vsConfs.keys():
        #print "key=" + key
        done = False
        while done == False:
            msg = vsConfs[key]["description"] + "(format:" + vsConfs[key]["format"] \
            + ", default:" + vsConfs[key]["default"] + ". Press enter for default):"
            line = ""#raw_input(msg) 
            reRet = None#re.match("^\s*" + vsConfs[key]["regex"] + "\s*$", line)
            value = None
            if line == "":
                value = vsConfs[key]["default"]
            elif reRet is not None:
                value = reRet.group(1)
            else:
                continue
            newConf += "\n" + key + " = " + value
            done = True
    newConf += "\n[VSConfend]\n"
    m = re.match("^(.*)\n*\[VSConf\].*\[VSConfend\]\n*(.*)$", content, re.DOTALL) 
    if m is not None:
        content = m.group(1) + newConf + m.group(2)
    else:
        content += newConf
else:
     m = re.match("^(.*)\n*\[VSConf\].*\[VSConfend\]\n*(.*)$", content, re.DOTALL) 
     if m is not None:
        content = m.group(1) + m.group(2)
 
with open(conf, 'w') as fp:
    fp.write(content)
            
            