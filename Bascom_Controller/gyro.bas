$regfile = "M88def.dat"
$crystal = 8000000
$baud = 19200
'$hwstack = 288
'$swstack = 288
'$framesize = 80
$hwstack = 512                                              ' default use 32 for the hardware stack
$swstack = 10                                               'default use 10 for the SW stack
$framesize = 40                                             'default use 40 for the frame space


Config Com1 = Dummy , Synchrone = 0 , Parity = None , Stopbits = 1 , Databits = 8 , Clockpol = 0
Open "com1:" For Binary As #1                               'Setup serial connection

Config Adc = Single , Prescaler = Auto , Reference = Avcc
Config Portc = Input


Config Portb.1 = Output
Pinb.1 = 1



Dim Value1 As Integer
Dim Value2 As Integer
Dim Value3 As Integer
Dim Startbyte As Byte
Dim Highbyte0 As Byte
Dim Lowbyte0 As Byte
Dim Highbyte1 As Byte
Dim Lowbyte1 As Byte
Dim Highbyte2 As Byte
Dim Lowbyte2 As Byte

Start Adc

Declare Sub Send_data



Do


Get #1 , Startbyte                                          'Receive Byte
     If Startbyte = 1 Then

      Send_data
     End If


Loop

End




Sub Send_data

      Value1 = Getadc(0)
      Value2 = Getadc(1)
      Value3 = Getadc(2)
                                                  'Split Integers

       Highbyte0 = High(value1)
       Lowbyte0 = Low(value1)
       Highbyte1 = High(value2)
       Lowbyte1 = Low(value2)
       Highbyte2 = High(value3)
       Lowbyte2 = Low(value3)


       Put #1 , Highbyte0                                   'Send Bytes
       Put #1 , Lowbyte0
       Put #1 , Highbyte1
       Put #1 , Lowbyte1
       Put #1 , Highbyte2
       Put #1 , Lowbyte2


End Sub