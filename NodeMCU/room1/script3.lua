local module = {}  
function module.start()
    PIN_IN = 5
    PIN_OUT= 6

    gpio.mode(PIN_IN, gpio.INPUT)
    gpio.mode(PIN_OUT, gpio.OUTPUT)

    if not tmr.create():alarm(1, tmr.ALARM_AUTO, function()
      if gpio.read(PIN_IN)==gpio.HIGH then
            while gpio.read(PIN_IN)==gpio.HIGH do
            end
            tmr.delay(7000)
            gpio.write(PIN_OUT, gpio.HIGH)
            tmr.delay(100)
            gpio.write(PIN_OUT, gpio.LOW)
        end
    end)
    then
      print("whoopsie")
    end

 
    print("hello")
    
end

return module
