<h1>MailBox Chardev with LCD display<h1>
Mathieu 
Uday
Matt 
Vladym

<h2>To Run Module</h2>

      git clone https://github.com/Mathieugril/DeviceDriver.git
      cd DeviceDriver/chardev
      make
      sudo insmod mailbox.ko
      sudo chmod 666 /dev/mailbox*
      gcc mb_userspace.c -o mb_userspace
      sudo ./mb_userspace
      cat /proc/mailbox
      
      #Must have I2C LCD on /dev/i2c-1
      gcc mb_lcd.c -o mb_lcd
      sudo ./mb_lcd
      
      sudo rmmod mailbox
