import queue, sys, json
import sounddevice as sd
from vosk import Model, KaldiRecognizer

MODEL_PATH = "/usr/share/vosk/models/en-us"

# Detect input device & its default samplerate
inp = sd.query_devices(kind='input')
SAMPLE_RATE = int(inp["default_samplerate"])  # e.g., 48000 on many boards

model = Model(MODEL_PATH)
rec = KaldiRecognizer(model, SAMPLE_RATE)

q = queue.Queue()

def callback(indata, frames, time, status):
    if status:
        print(status, file=sys.stderr)
    q.put(bytes(indata))

# Try 'default' first; if you know your ALSA card use device="plughw:0,0"
with sd.RawInputStream(samplerate=SAMPLE_RATE,
                       blocksize=8000,
                       dtype='int16',
                       channels=1,
                       device=None,
                       callback=callback):
    print(f"Listening at {SAMPLE_RATE} Hz… (Ctrl+C to stop)")
    try:
        while True:
            data = q.get()
            if rec.AcceptWaveform(data):
                print(json.loads(rec.Result())["text"])
    except KeyboardInterrupt:
        print("\nFinal:", json.loads(rec.FinalResult())["text"])

