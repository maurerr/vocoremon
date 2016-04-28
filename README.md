### VoCore Monitor
---
Used to fix VoCore can not show AP when STA not working in AP+STA


  Recently I am trying to fix a issue that most people had already known it: if you setting up VoCore into AP+STA mode to connect to your home router(or any router else), once your changed your home router password or you bring VoCore to another place that it can not find your home router AP signal ,VoCore will failed to create its "VoCore-XXXXXX" AP hot point. 
  
(Is Chinglish the most popular language on internet? :D Hope you can understand what I am saying upper)
  
  The problem is you will not able to connect to VoCore through wireless if you bring your VoCore-in-AP+STA-mode to another place.

  Last time I introduce a script writing by LiZhuohuan, but it has limit, it is not clever enough, it only change your VoCore from AP+STA to AP when it detects VoCore can not connect to your home router, but once your VoCore back to your home, it can not recover it, you have to set AP+STA back manually, and the script keep running every 10~30 seconds, that takes some power. So I write this little tool to do all magically. 
  
  Every time VoCore boot up, it will run vocoremon, if AP+STA is normal, quit itself, else vocoremon makes VoCore run in AP mode. When next time boot, vocoremon find your STA back to normal, it will quit AP mode and make VoCore run back into AP+STA mode.
  So once you install vocoremon, do not have to take USB2TTL anymore, VoCore are able to create its AP hot point anywhere, any setting.

###Install:
  Copy vocoremon to /bin/apstamon
  Add one line into /etc/rc.local: /bin/apstamon 20 >> /tmp/vocore.log &
  
  That's ALL.


###Test:
  I tested it, works normal in my home, but I know the situation is complex, it has a low chance not working, I open the source code into https://github.com/Vonger/vocoremon.git, hope everybody could update it make it better.  
  

