-- file : application.lua
local module = {}  
mqtt = nil
local function mqtt_start()  


        mqtt = mqtt.Client("client_id")
              
        mqtt:on("connect", function(conn) print ("connected") end)
        mqtt:on("offline", function(conn) print ("offline mqtt") end)
        
        -- on receive message
        mqtt:on("message", function(conn, topic, data)
            --print(topic .. ": " .. data)
            delayTime = tonumber(data)
            --print(delayTime)
          
        end)
        
        mqtt:connect("192.168.1.135", 1883, 0, function(conn) 
            print("connected")
          
            mqtt:subscribe("presence",0, function(conn)
            end)
        end)
end

function module.publish(topic, message)
    mqtt:publish(topic,message,0,0, function(conn) 
       print("sent") 
    end)
end


function module.start()
    print("mqtt client has started") 
    mqtt_start()
end

return module
