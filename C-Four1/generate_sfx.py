import math
import struct
import wave
import random
import os

SAMPLE_RATE = 44100

def clamp(val, min_val=-1.0, max_val=1.0):
    return max(min_val, min(max_val, val))

def save_wav(filename, samples):
    os.makedirs(os.path.dirname(os.path.abspath(filename)), exist_ok=True)
    with wave.open(filename, 'w') as wf:
        wf.setnchannels(1)        # mono
        wf.setsampwidth(2)        # 16-bit
        wf.setframerate(SAMPLE_RATE)
        
        raw_bytes = bytearray()
        for s in samples:
            val = int(clamp(s) * 32767.0)
            raw_bytes.extend(struct.pack('<h', val))
        wf.writeframes(raw_bytes)
    print(f"Generated {filename} ({len(samples)} samples, {len(samples)/SAMPLE_RATE:.2f}s)")

def make_sine(freq, t):
    return math.sin(2.0 * math.pi * freq * t)

def make_square(freq, t, duty=0.5):
    cycle = (t * freq) % 1.0
    return 1.0 if cycle < duty else -1.0

def make_triangle(freq, t):
    cycle = (t * freq) % 1.0
    return 4.0 * abs(cycle - 0.5) - 1.0

def make_saw(freq, t):
    cycle = (t * freq) % 1.0
    return 2.0 * cycle - 1.0

def lerp(a, b, t):
    return a + (b - a) * t

def gen_boot(out_path):
    # Retro BIOS boot chime: dual pleasant computer chirps (587Hz -> 880Hz -> 1174Hz)
    dur = 0.5
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = 1.0
        if t < 0.15:
            # tone 1: D5
            env = min(t / 0.01, 1.0) * math.exp(-t * 8.0)
            s = make_square(587.33, t, 0.4) * 0.4 + make_sine(587.33, t) * 0.4
        elif t < 0.30:
            # tone 2: A5
            t2 = t - 0.15
            env = min(t2 / 0.01, 1.0) * math.exp(-t2 * 8.0)
            s = make_square(880.0, t2, 0.4) * 0.4 + make_sine(880.0, t2) * 0.4
        else:
            # tone 3: D6
            t3 = t - 0.30
            env = min(t3 / 0.01, 1.0) * math.exp(-t3 * 7.0)
            s = make_square(1174.66, t3, 0.5) * 0.45 + make_sine(1174.66, t3) * 0.45
        samples.append(s * env * 0.6)
    save_wav(out_path, samples)

def gen_keystroke(out_path):
    # Short snappy terminal key clack
    dur = 0.035
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 120.0)
        noise = (random.random() * 2.0 - 1.0) * 0.5
        click = make_square(1400.0, t) * 0.3 + make_sine(700.0, t) * 0.3
        samples.append((noise + click) * env * 0.5)
    save_wav(out_path, samples)

def gen_nav(out_path):
    # Menu cursor move blip (pleasant high blip)
    dur = 0.05
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 70.0)
        freq = 820.0 + t * 400.0
        s = make_square(freq, t, 0.25) * 0.35 + make_sine(freq, t) * 0.35
        samples.append(s * env * 0.4)
    save_wav(out_path, samples)

def gen_select(out_path):
    # Menu confirmation / select
    dur = 0.12
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 25.0)
        if t < 0.05:
            freq = 660.0
        else:
            freq = 990.0
        s = make_square(freq, t, 0.3) * 0.4 + make_sine(freq, t) * 0.4
        samples.append(s * env * 0.5)
    save_wav(out_path, samples)

def gen_back(out_path):
    # Cancel / back sound: descending short chirp
    dur = 0.10
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 30.0)
        freq = lerp(600.0, 320.0, t / dur)
        s = make_triangle(freq, t) * 0.4 + make_sine(freq, t) * 0.4
        samples.append(s * env * 0.4)
    save_wav(out_path, samples)

def gen_dtmf(digit, out_path):
    # Standard DTMF frequencies for keypad
    # Row freqs: 697, 770, 852, 941
    # Col freqs: 1209, 1336, 1477
    dtmf_table = {
        '1': (697, 1209), '2': (697, 1336), '3': (697, 1477),
        '4': (770, 1209), '5': (770, 1336), '6': (770, 1477),
        '7': (852, 1209), '8': (852, 1336), '9': (852, 1477),
        '0': (941, 1336)
    }
    f1, f2 = dtmf_table.get(str(digit), (800, 1300))
    dur = 0.075
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        # Envelope: fast 4ms attack, smooth release
        if t < 0.005:
            env = t / 0.005
        else:
            env = math.exp(-(t - 0.005) * 45.0)
        s1 = make_sine(f1, t)
        s2 = make_sine(f2, t)
        click = make_square(f2 * 2.0, t, 0.1) * 0.1
        sample = (s1 * 0.45 + s2 * 0.45 + click) * env * 0.45
        samples.append(sample)
    save_wav(out_path, samples)

def gen_delete(out_path):
    # Delete / Backspace: downward short pitch drop
    dur = 0.07
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 50.0)
        freq = lerp(450.0, 180.0, t / dur)
        s = make_square(freq, t, 0.4) * 0.35 + make_saw(freq, t) * 0.2
        samples.append(s * env * 0.4)
    save_wav(out_path, samples)

def gen_clear(out_path):
    # Clear keypad buffer: two fast downward pips
    dur = 0.12
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        if t < 0.06:
            env = math.exp(-t * 60.0)
            f = lerp(500.0, 300.0, t / 0.06)
        else:
            t2 = t - 0.06
            env = math.exp(-t2 * 60.0)
            f = lerp(400.0, 200.0, t2 / 0.06)
        s = make_triangle(f, t) * 0.5 + make_sine(f, t) * 0.3
        samples.append(s * env * 0.45)
    save_wav(out_path, samples)

def gen_error(out_path):
    # Error buzzer: two harsh low buzz pulses (classic retro terminal reject)
    dur = 0.22
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        if t < 0.09:
            env = min(t / 0.005, 1.0) * math.exp(-t * 12.0)
            s = make_square(140.0, t, 0.5) * 0.5 + make_saw(140.0, t) * 0.3
        elif t < 0.11:
            env = 0.0
            s = 0.0
        else:
            t2 = t - 0.11
            env = min(t2 / 0.005, 1.0) * math.exp(-t2 * 12.0)
            s = make_square(125.0, t2, 0.5) * 0.5 + make_saw(125.0, t2) * 0.3
        samples.append(s * env * 0.55)
    save_wav(out_path, samples)

def gen_higher(out_path):
    # Scanner indicates HIGHER: rising sci-fi dual sweep
    dur = 0.32
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = min(t / 0.015, 1.0) * (1.0 - (t / dur) ** 0.8)
        # Pitch rises from 400Hz up to 1100Hz
        freq = 380.0 + (t / dur) ** 1.5 * 750.0
        s = make_sine(freq, t) * 0.5 + make_triangle(freq * 1.5, t) * 0.25 + make_square(freq, t, 0.3) * 0.15
        samples.append(s * env * 0.55)
    save_wav(out_path, samples)

def gen_lower(out_path):
    # Scanner indicates LOWER: descending sci-fi dual sweep
    dur = 0.32
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = min(t / 0.015, 1.0) * (1.0 - (t / dur) ** 0.8)
        # Pitch drops from 1050Hz down to 320Hz
        freq = 1050.0 - (t / dur) ** 1.3 * 730.0
        s = make_sine(freq, t) * 0.5 + make_triangle(freq * 0.75, t) * 0.25 + make_square(freq, t, 0.3) * 0.15
        samples.append(s * env * 0.55)
    save_wav(out_path, samples)

def gen_tick(out_path):
    # Subtle ticking fuse clock
    dur = 0.03
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 200.0)
        click = make_square(1600.0, t) * 0.3 + make_sine(500.0, t) * 0.5
        samples.append(click * env * 0.3)
    save_wav(out_path, samples)

def gen_alarm(out_path):
    # Urgent klaxon / danger siren pulse (used when timer < 15 or 2 tries left)
    dur = 0.35
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = min(t / 0.02, 1.0) * math.exp(-t * 3.5)
        # Siren warble
        freq = 780.0 + math.sin(2.0 * math.pi * 12.0 * t) * 180.0
        s = make_saw(freq, t) * 0.4 + make_square(freq, t, 0.4) * 0.3
        samples.append(s * env * 0.55)
    save_wav(out_path, samples)

def gen_theme(out_path):
    # Theme shift / CRT switch degauss
    dur = 0.20
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 15.0)
        freq = 250.0 + (t / dur) * 1200.0
        s = make_sine(freq, t) * 0.4 + make_triangle(freq * 2.0, t) * 0.3
        samples.append(s * env * 0.45)
    save_wav(out_path, samples)

def gen_pause(out_path):
    # Freeze / Pause sound
    dur = 0.16
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 20.0)
        if t < 0.08:
            freq = 740.0
        else:
            freq = 440.0
        s = make_square(freq, t, 0.25) * 0.35 + make_sine(freq, t) * 0.35
        samples.append(s * env * 0.4)
    save_wav(out_path, samples)

def gen_resume(out_path):
    # Resume sound
    dur = 0.14
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 22.0)
        if t < 0.07:
            freq = 440.0
        else:
            freq = 740.0
        s = make_square(freq, t, 0.25) * 0.35 + make_sine(freq, t) * 0.35
        samples.append(s * env * 0.4)
    save_wav(out_path, samples)

def gen_explosion(out_path):
    # Massive bomb explosion: deep punch + noise blast + long decaying rumble
    dur = 1.5
    total = int(dur * SAMPLE_RATE)
    samples = []
    # low pass filter state
    lp = 0.0
    for i in range(total):
        t = i / SAMPLE_RATE
        
        # Sub-bass punch at start (60Hz -> 25Hz)
        bass_env = math.exp(-t * 4.0)
        bass_freq = 65.0 - min(t * 30.0, 40.0)
        bass = make_sine(bass_freq, t) * 0.7 * bass_env
        
        # Distortion noise blast
        noise_env = min(t / 0.005, 1.0) * math.exp(-t * 3.2)
        raw_noise = (random.random() * 2.0 - 1.0)
        # One-pole low-pass filter opening and closing
        cutoff = lerp(0.85, 0.15, min(t / 0.8, 1.0))
        lp = lp + cutoff * (raw_noise - lp)
        noise = lp * noise_env * 0.85
        
        # Shockwave distortion saturation
        combined = bass + noise
        if combined > 0.8:
            combined = 0.8 + (combined - 0.8) * 0.2
        elif combined < -0.8:
            combined = -0.8 + (combined + 0.8) * 0.2
            
        samples.append(combined * 0.85)
    save_wav(out_path, samples)

def gen_win(out_path):
    # Mission complete victory fanfare: uplifting 8-bit arpeggio chord
    notes = [523.25, 659.25, 783.99, 1046.50, 1318.51] # C5, E5, G5, C6, E6
    step_dur = 0.11
    sustain_dur = 0.70
    dur = len(notes) * step_dur + sustain_dur
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        step = int(t / step_dur)
        if step < len(notes):
            # Arpeggiating note
            freq = notes[step]
            t_note = t - step * step_dur
            env = min(t_note / 0.01, 1.0) * (0.8 + 0.2 * math.exp(-t_note * 10.0))
            s = make_square(freq, t, 0.4) * 0.35 + make_sine(freq, t) * 0.35
        else:
            # Final sustained chord
            t_chord = t - len(notes) * step_dur
            env = math.exp(-t_chord * 3.0)
            s = 0.0
            for idx, f in enumerate([523.25, 659.25, 783.99, 1046.50]):
                s += (make_square(f, t, 0.35) * 0.2 + make_sine(f, t) * 0.25)
        samples.append(s * env * 0.6)
    save_wav(out_path, samples)

def gen_star(out_path):
    # Rating star chime: bright crystal ding (G6 -> C7)
    dur = 0.3
    total = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(total):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 12.0)
        s1 = make_sine(1567.98, t) * 0.5 # G6
        s2 = make_sine(2093.00, t) * 0.4 # C7
        sparkle = make_square(3135.96, t, 0.2) * 0.15 # G7
        samples.append((s1 + s2 + sparkle) * env * 0.5)
    save_wav(out_path, samples)

if __name__ == "__main__":
    assets_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets")
    os.makedirs(assets_dir, exist_ok=True)
    
    print(f"Generating SFX in {assets_dir}...")
    gen_boot(os.path.join(assets_dir, "sfx_boot.wav"))
    gen_keystroke(os.path.join(assets_dir, "sfx_keystroke.wav"))
    gen_nav(os.path.join(assets_dir, "sfx_nav.wav"))
    gen_select(os.path.join(assets_dir, "sfx_select.wav"))
    gen_back(os.path.join(assets_dir, "sfx_back.wav"))
    
    for d in range(10):
        gen_dtmf(d, os.path.join(assets_dir, f"sfx_keypad_{d}.wav"))
        
    gen_delete(os.path.join(assets_dir, "sfx_delete.wav"))
    gen_clear(os.path.join(assets_dir, "sfx_clear.wav"))
    gen_error(os.path.join(assets_dir, "sfx_error.wav"))
    gen_higher(os.path.join(assets_dir, "sfx_higher.wav"))
    gen_lower(os.path.join(assets_dir, "sfx_lower.wav"))
    gen_tick(os.path.join(assets_dir, "sfx_tick.wav"))
    gen_alarm(os.path.join(assets_dir, "sfx_alarm.wav"))
    gen_theme(os.path.join(assets_dir, "sfx_theme.wav"))
    gen_pause(os.path.join(assets_dir, "sfx_pause.wav"))
    gen_resume(os.path.join(assets_dir, "sfx_resume.wav"))
    gen_explosion(os.path.join(assets_dir, "sfx_explosion.wav"))
    gen_win(os.path.join(assets_dir, "sfx_win.wav"))
    gen_star(os.path.join(assets_dir, "sfx_star.wav"))
    print("All SFX generated successfully!")
