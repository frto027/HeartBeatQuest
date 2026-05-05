# Thread Model

The mod has at least 2 threads:

- Unity thread
- Background thread
- IXWebSocket threads

Use the following functions for cross-thread communication:

```cpp
runBackground([](){
    ...
});

runInUnityThread([](){
    ...
});
```

There is a C++ event loop that runs all background tasks in a single thread.

There is a Unity GameObject that processes all queued Unity thread functions.

IXWebSocket creates threads for each websocket/http client.

Singleton patterns are used extensively, so you can safely capture the `this` pointer in lambda expressions.
