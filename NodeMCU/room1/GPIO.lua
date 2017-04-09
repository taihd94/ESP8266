local module = {}  
function module.start()
    PIN_IN = 5
    PIN_OUT= 6

    --gpio.mode(PIN_IN, gpio.INPUT)
    gpio.mode(PIN_OUT, gpio.OUTPUT)

      -- use pin 1 as the input pulse width counter
      
      gpio.mode(PIN_IN,gpio.INT)
      local function pin1cb()
        while gpio.read(PIN_IN)==gpio.HIGH do
        end
        tmr.delay(delayTime)
        gpio.write(PIN_OUT, gpio.HIGH)
        tmr.delay(100)
        gpio.write(PIN_OUT, gpio.LOW)
      end
      gpio.trig(PIN_IN, "up", pin1cb)
    

   
    print("hello")
    
end

return module
