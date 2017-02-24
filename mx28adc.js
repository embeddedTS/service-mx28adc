"use strict"

var mx28adc = require("bindings")("/mx28adc.node")

console.log()

function Server(req,res,next) {
    console.log("MX28ADC:"+req.path)
    res.setHeader("Content-Type","text/plain")
    var values = mx28adc.get()
    res.send(values.join("\n"))
}

var endpoint = "/adc/"

module.exports = function(app,exports,options) {
    if (options && typeof options == "string") {
	endpoint = options
    }
    Log("service mma8451 ",endpoint)
    app.use(endpoint, Server)
}
