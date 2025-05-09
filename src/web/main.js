async function main() {
    // Initialize QtWebChannel
    await setupWebChannel();
    const canvas = document.getElementById('glcanvas');
    window.renderer = new Renderer(canvas);
    const ui = new UI();
}

window.onload = main;