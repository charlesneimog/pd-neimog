# loris spectral sustain loop experiment
#
# objective:
# - analyze a sound with loris
# - extract a stable sustain region
# - create a spectral loop transition
# - resynthesize continuously
#
# IMPORTANT:
# this is an experimental SMS-style loop prototype.
# the exact Loris API may vary depending on your build.

import math
import numpy as np
import librosa
import soundfile as sf
import loris

# ------------------------------------------------------------
# CONFIG
# ------------------------------------------------------------

SR = 48000

INPUT = "/home/neimog/Nextcloud/MusicData/Samples/Orchidea/Winds/Flute/ordinario/Fl-ord-C4-ff-N-N.wav"

LOOP_SECONDS = 120

# stable sustain region
SUSTAIN_START = 1.0
SUSTAIN_END = 2.5

# interpolation transition
CROSSFADE_TIME = 0.25

# ------------------------------------------------------------
# LOAD AUDIO
# ------------------------------------------------------------

x, sr = librosa.load(
    INPUT,
    sr=SR,
)

# ------------------------------------------------------------
# ANALYZE
# ------------------------------------------------------------

analyzer = loris.Analyzer(50, 100)

partials = analyzer.analyze(
    x.astype(np.float64).tolist(),
    SR,
)

print("analysis complete")

# ------------------------------------------------------------
# HELPERS
# ------------------------------------------------------------


def lerp(a, b, t):
    return a + ((b - a) * t)


def collect_breakpoints(partial):

    points = []

    for bp in partial:

        points.append(
            {
                "time": bp.time(),
                "freq": bp.frequency(),
                "amp": bp.amplitude(),
                "phase": bp.phase(),
                "bw": bp.bandwidth(),
            }
        )

    return points


# ------------------------------------------------------------
# EXTRACT SUSTAIN REGION
# ------------------------------------------------------------

partial_data = []

for partial in partials:

    pts = collect_breakpoints(partial)

    sustain = []

    for p in pts:

        if p["time"] >= SUSTAIN_START and p["time"] <= SUSTAIN_END:
            sustain.append(p)

    if len(sustain) < 2:
        continue

    partial_data.append(sustain)

print("partials in sustain:", len(partial_data))

# ------------------------------------------------------------
# BUILD LOOP TRANSITION
# ------------------------------------------------------------

loop_partials = []

for sustain in partial_data:

    first = sustain[0]
    last = sustain[-1]

    transition = []

    num_steps = 32

    dt = CROSSFADE_TIME / num_steps

    phase = last["phase"]

    for i in range(num_steps):

        t = i / (num_steps - 1)

        freq = lerp(
            last["freq"],
            first["freq"],
            t,
        )

        amp = lerp(
            last["amp"],
            first["amp"],
            t,
        )

        bw = lerp(
            last["bw"],
            first["bw"],
            t,
        )

        # preserve phase continuity
        phase += 2.0 * math.pi * freq * dt

        transition.append(
            {
                "time": (last["time"] + ((i + 1) * dt)),
                "freq": freq,
                "amp": amp,
                "phase": phase,
                "bw": bw,
            }
        )

    full = sustain + transition

    loop_partials.append(full)

# ------------------------------------------------------------
# REPEAT LOOP
# ------------------------------------------------------------

duration = SUSTAIN_END - SUSTAIN_START + CROSSFADE_TIME

repeats = int(LOOP_SECONDS / duration)

# ------------------------------------------------------------
# REBUILD LORIS PARTIALS
# ------------------------------------------------------------

new_partials = loris.PartialList()

for pdata in loop_partials:

    p = loris.Partial()

    current_time_offset = 0.0

    for r in range(repeats):

        for pt in pdata:

            bp = loris.Breakpoint(
                pt["freq"],
                pt["amp"],
                pt["phase"],
                pt["bw"],
            )

            p.insert(
                pt["time"] + current_time_offset,
                bp,
            )

        current_time_offset += duration

    new_partials.append(p)

print("loop partials created")

# ------------------------------------------------------------
# SYNTHESIS
# ------------------------------------------------------------

samples = loris.synthesize(new_partials, SR)

y = np.array(samples)

# # normalize
# y /= np.max(np.abs(y) + 1e-12)
#
# # ------------------------------------------------------------
# # WRITE
# # ------------------------------------------------------------
#
# sf.write(
#     "spectral_loop.wav",
#     y,
#     SR,
# )
#
# print("done")
