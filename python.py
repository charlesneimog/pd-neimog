import librosa
import librosa.display
import matplotlib.pyplot as plt
import numpy as np

# Load an audio file
filename = "Fl-tng-ram-B3-mf.aif"
y, sr = librosa.load(filename, sr=None)

# Compute RMS with a window size of 64 samples
rms = librosa.feature.rms(y=y, frame_length=64, hop_length=32)[0]
times = librosa.frames_to_time(np.arange(len(rms)), sr=sr, hop_length=32)
rms_diff = np.diff(rms, prepend=rms[0])  # Derivada discreta

window_size = 32
rms_diff_smoothed = np.convolve(rms_diff, np.ones(window_size)/window_size, mode='same')

threshold_attack = 0.0005
threshold_decay = -0.0005
sustain_threshold = 0.00040

# Identifica regiões de ataque, decay e sustain com histórico maior
attack_mask = rms_diff_smoothed > threshold_attack
decay_mask = rms_diff_smoothed < threshold_decay
sustain_mask = (~attack_mask) & (~decay_mask)

# Plotando os resultados
plt.figure(figsize=(15, 5))
plt.plot(times, rms, label="RMS Energy", color="b")
plt.fill_between(times, rms, where=attack_mask, color="red", alpha=0.3, label="Ataque")
plt.fill_between(times, rms, where=decay_mask, color="green", alpha=0.3, label="Decaimento")
plt.fill_between(times, rms, where=sustain_mask, color="yellow", alpha=0.3, label="Sustain")

plt.xlabel("Tempo ")
plt.ylabel("RMS Energia")
plt.title("RMS Energy with Attack, Decay, and Sustain Detection (Larger Window)")
plt.legend()
plt.show()

