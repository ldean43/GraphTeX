// util
function base64toFloat32(base64) {
    const binary = atob(base64);
    const len = binary.length;
    const bytes = new Uint8Array(len);

    for (let i = 0; i < len; i++) {
        bytes[i] = binary.charCodeAt(i);
    }

    return new Float32Array(bytes.buffer);
}

function base64toInt16(base64) {
    const binary = atob(base64);
    const len = binary.length;
    const bytes = new Uint8Array(len);

    for (let i = 0; i < len; i++) {
        bytes[i] = binary.charCodeAt(i);
    }

    return new Int16Array(bytes.buffer);
}

async function main() {
    // Initialize QtWebChannel
    await setupWebChannel();
    const canvas = document.getElementById('glcanvas');
    Renderer.init(canvas);
    UI.init();
}

window.onload = main;