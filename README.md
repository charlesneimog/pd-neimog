# pd-neimog

My owm pd-library with a lot of research objects.

## Objects

### Lua

#### orchidea

* `l.orchidea`: Read and play orchestral sounds samples from orchidea (check example inside [orchdieaSOL](http://www.orch-idea.org/examples/)).

#### upic 

* `l.readsvg`: Read and play svg score following some arbitary parameters (check example inside `Sources/lua/upic/score.svg`);
* `l.attrget`: l.readsvg will output a lua table, this object is able to get parameters from it (color, shape, stroke, fill, etc);
* `l.attrfilter`: It filter some (lua table) by parameters, for example just pass draw that have stroke == `blue`.
* `l.attrgetall`: It prints on Pd console all information about some object.
* `l.playpath`: l.readsvg will output a lua table, this object is able to get parameters from it (color, shape, stroke, fill, etc);

### Mir

* `onsetsds~`: Object that implementas the onset algorithm of Dan Stowell.
* `neimog.onset~`: Experiment with Kalman Filter for onset detection.
    

### Statistics

* `entropy` and `entropy~`: Calculates the Shannon entropy of a list of input numbers and outputs the normalized entropy value (between 0 and 1).
* `euclidean` and `euclidean~`:  Calculates the Euclidean distance between two lists of values and outputs the normalized distance, with additional support for real-time processing and signal-based operations.
* `kalman`:  Implements a simple 1D Kalman filter for real-time signal processing (it is the object [kalman](https://github.com/jwmatthys/kalman-pd)).
* `kl` and `kl~`: Calculates the Kullback-Leibler Divergence (KLD) between two probability distributions, supporting both real-time signal processing and offline analysis with optional normalization and exponential scaling.
* `renyi` and `renyi~`: Calculates the Renyi Entropy of a list of input numbers and outputs the normalized Renyi Entropy value.

### Array

* `a.rotate`: This shifts all the values in the array one index to the left, and then places the last value at the end of the array.



# Build

```
git clone https://github.com/charlesneimog/pd-neimog.git
git submodule update --init --recursive
cmake . -B build
cmake --build build
```
