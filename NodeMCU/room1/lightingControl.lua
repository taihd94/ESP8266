local module = {}  
function main()
    pin = 13
    gpio.mode(pin, gpio.OUTPUT)
    gpio.write(pin, gpio.HIGH)
end

return module
