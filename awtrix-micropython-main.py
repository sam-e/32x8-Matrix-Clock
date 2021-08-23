import network
import utime
import time
import math
import machine
from credentials import CREDENTIALS
from pixelvals import PIXELVALS
from machine import Pin, I2C, Timer
from neopixel import NeoPixel


# The number of NeoPixels
num_pixels = 256

# The order of the pixel colors - RGB or GRB. Some NeoPixels have red and green reversed!
# For RGBW NeoPixels, simply change the ORDER to RGBW or GRBW.
#ORDER = neopixel.GRB

pin = Pin(14, Pin.OUT)   # set GPIO0 to output to drive NeoPixels
pixels = NeoPixel(pin, num_pixels)
    
# The LEDs values object
# Wifi credentials
pixelvals = PIXELVALS(PIXELVALS.FUCHSIA, 255)
    
matrixWidth = 32
matrixHeight = 8

nums = [[[0,1,1,1,0,0],
         [1,1,1,1,1,0],
         [1,1,0,1,1,0],
         [1,1,0,1,1,0],
         [1,1,0,1,1,0],
         [1,1,0,1,1,0],
         [1,1,1,1,1,0],
         [0,1,1,1,0,0]],

        # 1
        [[0,0,1,1,0,0],
         [0,1,1,1,0,0],
         [0,0,1,1,0,0],
         [0,0,1,1,0,0],
         [0,0,1,1,0,0],
         [0,0,1,1,0,0],
         [0,1,1,1,1,0],
         [0,1,1,1,1,0]],

        # 2
        [[1,1,1,1,0,0],
         [1,1,1,1,1,0],
         [0,0,0,1,1,0],
         [0,0,1,1,1,0],
         [1,1,1,1,0,0],
         [1,1,0,0,0,0],
         [1,1,1,1,1,0],
         [1,1,1,1,1,0]],

        # 3
        [[1,1,1,1,0,0],
         [1,1,1,1,1,0],
         [0,0,0,1,1,0],
         [0,1,1,1,1,0],
         [0,1,1,1,1,0],
         [0,0,0,1,1,0],
         [1,1,1,1,1,0],
         [1,1,1,1,0,0]],

        # 4
        [[0,0,1,1,1,0],
         [0,1,1,1,1,0],
         [1,1,1,1,1,0],
         [1,1,0,1,1,0],
         [1,1,1,1,1,1],
         [1,1,1,1,1,1],
         [0,0,0,1,1,0],
         [0,0,0,1,1,0]],

        # 5
        [[1,1,1,1,1,0],
         [1,1,1,1,1,0],
         [1,1,0,0,0,0],
         [1,1,1,1,0,0],
         [0,0,1,1,1,0],
         [0,0,0,1,1,0],
         [1,1,1,1,1,0],
         [1,1,1,1,0,0]],

        # 6
        [[0,1,1,1,0,0],
         [1,1,1,1,1,0],
         [1,1,0,0,0,0],
         [1,1,1,1,0,0],
         [1,1,1,1,1,0],
         [1,1,0,1,1,0],
         [1,1,1,1,1,0],
         [0,1,1,1,0,0]],

        # 7
        [[1,1,1,1,1,1],
         [1,1,1,1,1,1],
         [1,1,0,0,1,1],
         [0,0,0,1,1,0],
         [0,0,1,1,0,0],
         [0,1,1,0,0,0],
         [0,1,1,0,0,0],
         [1,1,0,0,0,0]],

        # 8
        [[0,1,1,1,0,0],
         [1,1,1,1,1,0],
         [1,1,0,1,1,0],
         [1,1,1,1,1,0],
         [1,1,1,1,1,0],
         [1,1,0,1,1,0],
         [1,1,1,1,1,0],
         [0,1,1,1,0,0]],

        #9
        [[0,1,1,1,0,0],
         [1,1,1,1,1,0],
         [1,1,0,1,1,0],
         [1,1,1,1,1,0],
         [1,1,1,1,1,0],
         [0,0,0,1,1,0],
         [1,1,1,1,1,0],
         [0,1,1,1,0,0]]]

dots = [[0,0],
        [1,1],
        [1,1],
        [0,0],
        [0,0],
        [1,1],
        [1,1],
        [0,0]]

# The network class
# Basically the network
class network_setup:
    creds = CREDENTIALS

    # Network setup
    sta_if = network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        print('connecting to network...')
        sta_if.active(True)
        sta_if.connect(creds.SSID, creds.PSWD)
        while not sta_if.isconnected():	
            print(".", end="")
                
    print('network config:', sta_if.ifconfig())
    
net = network_setup


def write_digit(index, digit):
    digits = pixelvals.DIGITS
    
    #Get the led array values from the led object
    r_val = pixelvals.get_rColorval()
    g_val = pixelvals.get_gColorval()
    b_val = pixelvals.get_bColorval()   

    if index is 0:
        x_offset = 26
    elif index is 1:
        x_offset = 19
    elif index is 2:
        x_offset = 9
    elif index is 3:
        x_offset = 2

    for y in range(8):
        for x in range(6):
            major = x + x_offset
            minor = y
            if(nums[digit][y][x]):
                majorScale = matrixHeight

                if (major & 1):
                    pixel_offset = (major + 1) * majorScale - 1 - minor
                else:
                    pixel_offset = major * majorScale + minor

                pixels[pixel_offset] = (r_val, g_val, b_val)


def clear_digit(index):
    global one

    if index is 0:
        x_offset = 26
    elif index is 1:
        x_offset = 19
    elif index is 2:
        x_offset = 9
    elif index is 3:
        x_offset = 2

    for y in range(8):
        for x in range(6):
            major = x + x_offset
            minor = y
            majorScale = matrixHeight

            if (major & 1):
                pixel_offset = (major + 1) * majorScale - 1 - minor
            else:
                pixel_offset = major * majorScale + minor

            #print(pixel_offset)
            pixels[pixel_offset] = (0, 0, 0)


def transition(index, digit):
    offscreen = 0
    #Get the led array values from the led object
    r_val = pixelvals.get_rColorval()
    g_val = pixelvals.get_gColorval()
    b_val = pixelvals.get_bColorval()   

    if index is 0:
        x_offset = 26
        if digit == 9:
            next_digit = 0
        else:
            next_digit = digit +1
    elif index is 1:
        x_offset = 19
        if digit == 5:
            next_digit = 0
        else:
            next_digit = digit +1
    elif index is 2:
        x_offset = 9
        if digit == 9:
            next_digit = 0
        else:
            next_digit = digit +1
    elif index is 3:
        x_offset = 2
        if digit == 2:
            next_digit = 0
        else:
            next_digit =digit +1
    
    for i in range(8):
        clear_digit(index)

        
        if offscreen < 0:
            offscreen = 0
        else:
            offscreen +=1        
    
        for y in range(offscreen):
            for x in range(6):
                major = x + x_offset
                minor = y 
                if(nums[next_digit][y-offscreen][x]):
                    majorScale = matrixHeight

                    if (major & 1):
                        pixel_offset = (major + 1) * majorScale - 1 - minor
                    else:
                        pixel_offset = major * majorScale + minor
                    #print(pixel_offset)
                    pixels[pixel_offset] = (r_val, g_val, b_val)
        pixels.write()
        time.sleep_ms(30)
        
                
                
def updatedots():
    global dots
    
    dotsOn = pixelvals.getDots()
    if dotsOn != True: pixelvals.setDots(True) 
    else: pixelvals.setDots(False)
    
    #Get the led array values from the led object
    r_val = pixelvals.get_rColorval()
    g_val = pixelvals.get_gColorval()
    b_val = pixelvals.get_bColorval()
    
    #  clear dot dots
    if not dotsOn:
        isDots = True
        for y in range(8): 
            for x in range(2):     
                major = x + 16
                minor = y
                majorScale = matrixHeight

                if (major & 1):
                    pixel_offset = (major + 1) * majorScale - 1 - minor
                else:
                    pixel_offset = major * majorScale + minor

                #print(pixel_offset)
                pixels[pixel_offset] = (0, 0, 0)
    
    if dotsOn:
        #  dots
        isDots = False
        for y in range(8): 
            for x in range(2):    
                if(dots[y][x]):
                    major = x + 16 
                    minor = y
                    majorScale = matrixHeight

                    if (major & 1):
                        pixel_offset = (major + 1) * majorScale - 1 - minor
                    else:
                        pixel_offset = major * majorScale + minor

                    pixels[pixel_offset] = (r_val, g_val, b_val)


def updateRTC():
    # Synchronise system time and the DS3231 RTC
    import ntptime
    utc_shift = 10

    print("Local time before synchronization：%s" %str(utime.localtime()))
    ntptime.settime()
    print("Local time after synchronization：%s" %str(utime.localtime()))

    #board.DS3231.save_time()
    
updateRTC()


def update_time():
    utc_shift = 10

    #Get the time from the RTC, get_time(True) updates system time
    #board.DS3231.get_time(True)
    pixelvals.updateBrightness(1)
    clockTime = utime.localtime(utime.mktime(utime.localtime()) + utc_shift*3600)
    
    digit = clockTime[4]
    steve = clockTime[3]
    secs = clockTime[5]
    
    #Offsets for individual numbers
    DIG_1 = 0
    DIG_2 = 7
    DIG_3 = 16
    DIG_4 = 23

    fMdig = int(digit / 10)
    sMdig = int(digit % 10)

    fHdig = int(steve  / 10)
    sHdig = int(steve % 10)

    
    #print (clockTime)
    if (secs==59):
        transition(0, sMdig)
    else:
        clear_digit(0)
        write_digit(0, sMdig)
    if (fMdig==9 and secs==59):
        transition(1, fMdig)
    else:
        clear_digit(1)
        write_digit(1, fMdig)
    if (steve==59 and secs==59):
        transition(2, sHdig)
    else:
        clear_digit(2)
        write_digit(2, sHdig)
    if (sHdig==9 and steve==59 and secs==59):
        transition(3, fHdig)
    else:
        clear_digit(3)
        write_digit(3, fHdig)

    pixels.write()



                
# Global for the millisecond and second timer loop
milliSec = 0

while True:
    # 1000 millisecond or second function
    millis = int(utime.ticks_ms())
    
    # 1 second loop
    if millis - milliSec >= 1000:
        milliSec = millis
        updatedots()
        update_time()
