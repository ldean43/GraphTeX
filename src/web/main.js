async function main() {
    // Initialize QtWebChannel
    await setupWebChannel();
    const ui = new UI();
    const canvas = document.getElementById('canvas');
    const renderer = new Renderer(canvas);
}

window.onload = main;