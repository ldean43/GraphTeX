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
    Renderer.init(canvas);
    UI.init();
}

window.onload = main;