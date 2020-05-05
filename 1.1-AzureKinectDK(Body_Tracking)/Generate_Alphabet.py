import tts

D = {'U': 21, 'V': 22, 'W': 23, 'X': 24, 'Y': 25, 'Z': 26}

for key in D.keys():
    app = tts.TextToSpeech(key)
    app.get_token()
    app.save_audio()
