/**
 * Created by lycheng Yang Han
 */
self.onmessage = function(e){
  switch(e.data.command){
    case "transform":
      transform.transaction(e.data.buffer);
      break;
  }
}

var transform = {
  transaction(buffer) {
    let bufToxkHz = transform.to8kHz(buffer)
    let bufTo16BitPCM = transform.to16BitPCM(bufToxkHz)
    // let bufToBase64 = transform.toBase64(bufTo16BitPCM)
    self.postMessage({'buffer': bufTo16BitPCM})
  },
  to8kHz (buffer) {
    var data = new Float32Array(buffer)
    var fitCount = Math.round(data.length * (8000 / 44100))
    var newData = new Float32Array(fitCount)
    var springFactor = (data.length - 1) / (fitCount - 1)
    newData[0] = data[0]
    for (let i = 1; i < fitCount - 1; i++) {
      var tmp = i * springFactor
      var before = Math.floor(tmp).toFixed()
      var after = Math.ceil(tmp).toFixed()
      var atPoint = tmp - before
      newData[i] = data[before] + (data[after] - data[before]) * atPoint
    }
    newData[fitCount - 1] = data[data.length - 1]
    return newData
  },
  to16kHz (buffer) {
    var data = new Float32Array(buffer)
    var fitCount = Math.round(data.length * (16000 / 44100))
    var newData = new Float32Array(fitCount)
    var springFactor = (data.length - 1) / (fitCount - 1)
    newData[0] = data[0]
    for (let i = 1; i < fitCount - 1; i++) {
      var tmp = i * springFactor
      var before = Math.floor(tmp).toFixed()
      var after = Math.ceil(tmp).toFixed()
      var atPoint = tmp - before
      newData[i] = data[before] + (data[after] - data[before]) * atPoint
    }
    newData[fitCount - 1] = data[data.length - 1]
    return newData
  },

  to16BitPCM (input) {
    var dataLength = input.length * (16 / 8)
    var dataBuffer = new ArrayBuffer(dataLength)
    var dataView = new DataView(dataBuffer)
    var offset = 0
    for (var i = 0; i < input.length; i++, offset += 2) {
      var s = Math.max(-1, Math.min(1, input[i]))
      dataView.setInt16(offset, s < 0 ? s * 0x8000 : s * 0x7FFF, true)
    }
    return Array.from(new Int8Array(dataView.buffer))
  },
  toBase64 (buffer) {
    var binary = ''
    var bytes = new Uint8Array(buffer)
    var len = bytes.byteLength
    for (var i = 0; i < len; i++) {
      binary += String.fromCharCode(bytes[i])
    }
    return window.btoa(binary)
  }
}