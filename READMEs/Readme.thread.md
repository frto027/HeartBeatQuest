# Thread Model

The mod has at least 2 thread.

- unity thread.
- background thread.
- ixwebsocket threads.

use the following function to do things between threads:
```cpp
runBackground([](){
    ...
});

runInUnityThread([](){
    ...
});
```

we have a cpp event loop, that run all background in one single thread.

we have an Unity GameObject, runs every unity thread functions in queue.

ixwebsocket create threads for every websockets/httpclients.

We use the singleton pattern extensively, so we can freely capture the `this` pointer in lambda expressions.
