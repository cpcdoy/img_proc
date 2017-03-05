# img_proc
Image processing project

## Building the project

### In-source build
``` 
  cmake .
  make
```

And to clean everything :

```
make deep_clean
```

### Out-of-source build

``` 
  mkdir build
  cd build
  cmake ..
  make
```

### Dependencies

You need to compile the latest OpenCV 3 from source

### Articles

The blur detection is based on this [paper](https://www.cs.cmu.edu/~htong/pdf/ICME04_tong.pdf)

To understand the Haar Wavelet Transform, read [this](http://www.whydomath.org/node/wavlets/hwt.html)
