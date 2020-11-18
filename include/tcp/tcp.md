# Tcp Notes

## Server
### tcp acceptor
- `TcpAcceptor` 调用 `startAccept` 方法之后，会从自己的 `mIoContextThreadPool` 中选择一个 `io_context`，构造出新的 `socket`
- `startAccept` 方法会要求传入一个 `callback` ，即对于这个新的`socket`要怎么处理，所有的网络数据收发都在该`callback`中，耦合性低，这样`TcpAcceptor`只需要做接受网络连接即可

### tcp service
- `TcpService` 构造的时候会传入一下参数来控制网络线程的数量。
- `Start` 方法会调用 `TcpAcceptor` 的 `startAccept` 方法，传入的 `callback` 会调用 `TcpService` 的 `onRawSocket` 接口方法，以供使用者自己做扩展。
- `callback` 中调用了 `onRawSocket` 之后，会根据 `asio::socket` 构造出 `TcpSession`, 调用 `TcpService` 的 `onConnected` 接口方法
- 上一步构造完成`TcpSession`的时候会调用 `asio::socket` 的 `async_receive`, 开始接受数据，并且会回调 `TcpService` 的 `dataHandler` 接口方法
- 并且 `TcpSession` 在断开连接的时候调用 `TcpService` 的 `onClosed` 接口方法
- 这样在一个新连接（socket）的各个时期（创建，收到数据，断开连接）都会调用对应的方法，使用者只要重写这些方法即可，减轻使用者的负担。