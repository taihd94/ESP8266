local module = {}

module.SSID = {}   
module.SSID["BKHome"] = "bkhomebkhome"

module.HOST = "10.42.0.1"  
module.PORT = 3000
module.ID = node.chipid()

return module  
