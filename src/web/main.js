function throttle(func, delay) {
    let lastAgs;
    let lastCall = 0;
    let scheduled = false;

    return function (...args) {
        const now = Date.now();
        lastArgs = args;

        if ((now - lastCall) >= delay) {
            lastCall = now;
            func.apply(this, args);
        } else if (!scheduled) {
            const remaining = delay - (now - lastCall);
            scheduled = true;
            setTimeout(() => {
                scheduled = false;
                lastCall = Date.now();
                func.apply(this, lastArgs);
            }, remaining);
        }
    };
}

function base64toFloat32(base64) {
    const binary = atob(base64);
    const len = binary.length;
    const bytes = new Uint8Array(len);

    for (let i = 0; i < len; i++) {
        bytes[i] = binary.charCodeAt(i);
    }

    return new Float32Array(bytes.buffer);
}

function hexToRgb(hex) {
    hex = hex.replace(/^#/, '');
    if (hex.length === 3) {
        // Expand shorthand form (#f93 → #ff9933)
        hex = hex.split('').map(c => c + c).join('');
    }
    const int = parseInt(hex, 16);
    return [
        (int >> 16) & 255,
        (int >> 8) & 255,
        int & 255
    ];
}

async function main() {
    // Initialize QtWebChannel
    await setupWebChannel();
    const canvas = document.getElementById('glcanvas');
    const throttleUpdateMesh = throttle(bridge.updateMesh, 100);
    Renderer.init(canvas);
    UI.init(throttleUpdateMesh);

    bridge.meshUpdated.connect(function(id, verticesBase64, normalsBase64) {
        const vertices = base64toFloat32(verticesBase64);
        const normals = base64toFloat32(normalsBase64);
        if (id in Renderer.getMeshes()) {
            Renderer.updateMesh(id, vertices, normals);
        } else {
            Renderer.addMesh(id, vertices, normals);
        }
        Renderer.render();
    })
}

window.onload = main;