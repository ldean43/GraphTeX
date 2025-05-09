function setupWebChannel() {
    return new Promise((resolve) => {
        new QWebChannel(qt.webChannelTransport, function(channel) {
            window.bridge = channel.objects.bridge;
            console.log("Bridge is ready!");
            resolve(bridge);
        });
    });
}
