/**
 *  
 * javascript client for online ASR server
 * It is preferred to test or show, rather than industry
 * 
 */
 
// pcm transformer worker
let recorderWorker = new Worker('./transformpcm.worker.js')
// buffer for streaming input
let buffer = []
let AudioContext = window.AudioContext || window.webkitAudioContext
let notSupportTip = 'try chrome brower and use localhost or 127.0.0.1 '
navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia

recorderWorker.onmessage = function (e) {
  buffer.push(...e.data.buffer)
}

class IatRecorder {
  constructor (config) {
    this.config = config
    this.state = 'ing'
  }

  start () {
    this.stop()
    if (navigator.getUserMedia && AudioContext) {
      this.state = 'ing'
      if (!this.recorder) {
        var context = new AudioContext()
        this.context = context
        this.recorder =context.createScriptProcessor(0, 1, 1)

        var getMediaSuccess = (stream) => {
          var mediaStream = this.context.createMediaStreamSource(stream)
          this.mediaStream = mediaStream
          this.recorder.onaudioprocess = (e) => {
            this.sendData(e.inputBuffer.getChannelData(0))
          }
          this.connectWebsocket()
        }
        var getMediaFail = (e) => {
          this.recorder = null
          this.mediaStream = null
          this.context = null
          console.log('request microphone failed')
        }
        if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
          navigator.mediaDevices.getUserMedia({
            audio: true,
            video: false
          }).then((stream) => {
            getMediaSuccess(stream)
          }).catch((e) => {
            getMediaFail(e)
          })
        } else {
          navigator.getUserMedia({
            audio: true,
            video: false
          }, (stream) => {
            getMediaSuccess(stream)
          }, function (e) {
            getMediaFail(e)
          })
        }
      } else {
        this.connectWebsocket()
      }
    } else {
      var isChrome = navigator.userAgent.toLowerCase().match(/chrome/)
      alert(notSupportTip)
    }
  }
    
  stop () {
    this.state = 'end'
    try {      
      this.mediaStream.disconnect(this.recorder)
      this.recorder.disconnect()
    } catch (e) {}
  }
  
  sendData (buffer) {
    recorderWorker.postMessage({
      command: 'transform',
      buffer: buffer
    })
  }

  connectWebsocket () {
    // ASR backend server ip
    var url = 'ws://10.186.1.174:8080'
    console.log("connect to :"+url);

    if ('WebSocket' in window) {
      this.ws = new WebSocket(url)
    } else if ('MozWebSocket' in window) {
      this.ws = new MozWebSocket(url)
    } else {
      alert(notSupportTip)
      return null
    }
    this.ws.onopen = (e) => {
      this.mediaStream.connect(this.recorder)
      this.recorder.connect(this.context.destination)
      setTimeout(() => {
        this.wsOpened(e)
      }, 500) // timer to wait the connection is established
      this.config.onStart && this.config.onStart(e)
    }
    this.ws.onmessage = (e) => {// set on message callback
      // this.config.onMessage && this.config.onMessage(e)
      this.wsOnMessage(e)
    }
    this.ws.onerror = (e) => {
      this.stop()
      console.log("ws.onerror");
      this.config.onError && this.config.onError(e)
    }
    this.ws.onclose = (e) => {
      this.stop()
      console.log("ws.onclose");
      $('.start-button').attr('disabled', false);
      this.config.onClose && this.config.onClose(e)
    }
  }
  
  wsOpened () { // on open callback
    if (this.ws.readyState !== 1) { // detect whether the connection is established
      return
    }else{
      var params = {"opt":"START"};// start ASR parameter
      var jsonObject = JSON.stringify(params);
      this.ws.send(jsonObject);
      console.log("send ASR START parameter");    
    }
    var audioData = buffer.splice(0, 2560) // speech buffer size
    this.ws.send(new Int8Array(audioData))
    this.handlerInterval = setInterval(() => {
      if (this.ws.readyState !== 1) {
        clearInterval(this.handlerInterval)
        return
      }
      if (buffer.length === 0) {
        if (this.state === 'end') {        
          var params = {"opt":"STOP"};// stop ASR parameter
          var jsonObject = JSON.stringify(params);
          this.ws.send(jsonObject);
          console.log("send ASR STOP parameter"); 
          this.ws.close()
          clearInterval(this.handlerInterval)
        }
        return false
      }
      //var audioData = buffer.splice(0, 1280)
      var audioData = buffer.splice(0, 2560) // speech buffer size
      if(audioData.length > 0){
        this.ws.send(new Int8Array(audioData))
      }
    }, 40)
  }

  wsOnMessage(e){// on message callback
    let jsonData = e
    //console.log(jsonData);
    if(this.config.onMessage && typeof this.config.onMessage == 'function'){
        this.config.onMessage(jsonData.data)
      }
  }
  

  ArrayBufferToBase64 (buffer) {
    var binary = ''
    var bytes = new Uint8Array(buffer)
    var len = bytes.byteLength
    for (var i = 0; i < len; i++) {
      binary += String.fromCharCode(bytes[i])
    }
    return window.btoa(binary)
  }
}

class IatTaste {
  constructor () {
    var iatRecorder = new IatRecorder({
      onClose: () => {
        this.stop()
        this.reset()
      },
      onError: (data) => {
        this.stop()
        this.reset()
        alert('WebSocket connection failed')
      },
      onMessage: (message) =>{
        this.setResult(JSON.parse(message))
      },
      onStart: () => {
        $('hr').addClass('hr')
        var dialect = $('.dialect-select').find('option:selected').text()
        $('.taste-content').css('display', 'none')
        $('.start-taste').addClass('flex-display-1')
        $('.dialect-select').css('display', 'none')
        $('.start-button').text('stop')
        $('.time-box').addClass('flex-display-1')
        $('.dialect').text(dialect).css('display', 'inline-block')
        this.counterDown($('.used-time'))
      }
    })
    this.iatRecorder = iatRecorder
    this.counterDownDOM = $('.used-time')
    this.counterDownTime = 0

    this.text = {
      start: 'start',
      stop: 'stop'
    }
    this.resultText = ''
  }

  start () {
    this.iatRecorder.start()
  }

  stop () {
    $('hr').removeClass('hr')
    this.iatRecorder.stop()
  }

  reset () {
    this.counterDownTime = 0
    clearTimeout(this.counterDownTimeout)
    buffer = []
    $('.time-box').removeClass('flex-display-1').css('display', 'none')
    $('.start-button').text(this.text.start)
    $('.dialect').css('display', 'none')
    $('.dialect-select').css('display', 'inline-block')
    $('.taste-button').css('background', '#0b99ff')
  }

  init () {
    let self = this
    //start
    $('#taste_button').click(function () {
      if (navigator.getUserMedia && AudioContext && recorderWorker) {
        self.start()
      } else {
        alert(notSupportTip)
      }
    })
    //stop
    $('.start-button').click(function () {
      if ($(this).text() === self.text.start && !$(this).prop('disabled')) {
        $('#result_output').text('')
        self.resultText = ''
        self.start()
        //console.log("按钮非禁用状态，正常启动" + $(this).prop('disabled'))
      } else {
        //$('.taste-content').css('display', 'none')
        $('.start-button').attr('disabled', true);
        self.stop()
        //reset
        this.counterDownTime = 0
        clearTimeout(this.counterDownTimeout)
        buffer = []
        $('.time-box').removeClass('flex-display-1').css('display', 'none')
        $('.start-button').text('stoping listening...')
        $('.dialect').css('display', 'none')
        $('.taste-button').css('background', '#8E8E8E')
        $('.dialect-select').css('display', 'inline-block')
        
        //console.log("按钮非禁用状态，正常停止" + $(this).prop('disabled'))
      }
    })
  }
  setResult (data) {
    //console.log(data);
    var currentText = $('#result_output').html()
    var certain;
    var uncertain;
    uncertain = data["result"]
    $('#result_output').html(uncertain);

    var ele = document.getElementById('result_output');
    ele.scrollTop = ele.scrollHeight;
  }

  counterDown () {
    /*//timer 5 minutes
    if (this.counterDownTime === 300) {
      this.counterDownDOM.text('05: 00')
      this.stop()
    } else if (this.counterDownTime > 300) {
      this.reset()
      return false
    } else */ 
    if (this.counterDownTime >= 0 && this.counterDownTime < 10) {
      this.counterDownDOM.text('00: 0' + this.counterDownTime)
    } else if (this.counterDownTime >= 10 && this.counterDownTime < 60) {
      this.counterDownDOM.text('00: ' + this.counterDownTime)
    } else if (this.counterDownTime%60 >=0 && this.counterDownTime%60 < 10) {
      this.counterDownDOM.text('0' + parseInt(this.counterDownTime/60) + ': 0' + this.counterDownTime%60)
    } else {
      this.counterDownDOM.text('0' + parseInt(this.counterDownTime/60) + ': ' + this.counterDownTime%60)
    }
    this.counterDownTime++
    this.counterDownTimeout = setTimeout(() => {
      this.counterDown()
    }, 1000)
  }
}
var iatTaste = new IatTaste()
iatTaste.init()
