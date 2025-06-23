class UI {
    static table;
    static range;
    static step;
    static latest = 0;
    static clipZ = true;
    static throttleUpdateMesh;

    static init(throttle) {
        // Initialize event listeners for first 2 rows of the table
        UI.table = document.querySelector('table');
        UI.step = document.getElementById('meshResolution').value;
        UI.range = document.getElementById('range').value;
        UI.throttleUpdateMesh = throttle;
        document.querySelector('#display1 button').onclick = () => UI.deleteEquation(1);
        document.querySelector('#label1 button').onclick = () => UI.viewLatex(1);
        document.querySelector('#display1 input').onchange = (e) => Renderer.updateColor('equation1', e.target.value);
        document.getElementById('equation1').addEventListener('input', () => UI.updateDisplay(1));
        document.getElementById('addEquation').onclick = () => UI.addEquation();
        document.getElementById('range').oninput = (e) => UI.updateRange(e.target.value);
        document.getElementById('meshResolution').oninput = (e) => UI.updateMeshResolution(e.target.value);
        document.getElementById('shaderType').onchange = (e) => { Renderer.activeShader = e.target.value; Renderer.render(); }
        document.getElementById('lightXRotation').oninput = (e) => { Renderer.lightXRotation = e.target.value; Renderer.updateLightPos(); }
        document.getElementById('lightYRotation').oninput = (e) => { Renderer.lightYRotation = e.target.value; Renderer.updateLightPos(); }
        document.getElementById('clipZ').onchange = (e) => { UI.clipZ = e.target.checked; UI.updateDisplay(1) }

        bridge.createEvaluator('\\sin(x)', 'equation1', {'x': 0, 'y': 0}, 150, 10, true).then(res => {
            if (!res) Renderer.clear();
        })
    }

    // Update the display with the current equation
    static updateDisplay(num) {
        const equation = document.getElementById(`equation${num}`).value;
        const display = document.getElementById(`display${num}`).querySelector('div');
        display.textContent = '\\[\\displaystyle{' + equation + '}\\]';
        MathJax.typesetPromise();

        // Update C++ evaluator, generate mesh, and render
        bridge.updateEvaluator(equation, `equation${num}`, {'x': 0, 'y': 0}, UI.step, UI.range, UI.clipZ).then(res => {
            if (!res) {
               Renderer.clearMesh(`equation${num}`);
               Renderer.render();
            }
        });
    }

    // Toggle the visibility of the LaTeX input area
    static viewLatex(num) {
        const row = document.getElementById(`rowEquation${num}`);
        const cell = document.getElementById(`label${num}`);
        const button = cell.querySelector('button');

        if (window.getComputedStyle(row).display === 'none') {
            row.style.display = 'table-row';
            cell.setAttribute('rowspan', '2');
            button.textContent = '▲';
        } else {
            row.style.display = 'none';
            cell.setAttribute('rowspan', '1');
            button.textContent = '▼';
        }
    }

    // Initialize a new equation
    static addEquation() {
        const num = UI.table.rows.length / 2 + 1;

        // Create first row
        const row1 = document.createElement('tr');
        row1.id = `rowDisplay${num}`;
        row1.innerHTML = `
            <td id='label${num}' class='label' rowspan='1'>
                <div>${num}</div>
                <button class='viewButton'>▼</button>
            </td>
            <td id='display${num}'>
                <div>\\[\\displaystyle{\\displaylines{\\sin(x)}}\\]</div>
                <input type='color' class='color' value='#ff0000'>
                <button class='deleteEquation'>✖</button>
            </td>`;

        // Create second row (hidden by default)
        const row2 = document.createElement('tr');
        row2.id = `rowEquation${num}`;
        row2.style.display = 'none';
        row2.innerHTML = `
            <td colspan='2'>
                <textarea id='equation${num}' autofocus>\\sin(x)</textarea>
            </td>`;

        // Append rows, update MathJaX
        UI.table.appendChild(row1);
        UI.table.appendChild(row2);
        MathJax.typesetPromise();

        // Set event listeners
        document.getElementById(`equation${num}`).addEventListener('input', () => UI.updateDisplay(num));
        document.getElementById('clipZ').addEventListener('change', (e) => {UI.clipZ = e.target.checked; UI.updateDisplay(num) });
        document.querySelector(`#label${num} button`).onclick = () => UI.viewLatex(num);
        document.querySelector(`#display${num} input`).onchange = (e) => Renderer.updateColor(`equation${num}`, e.target.value);
        document.querySelector(`#display${num} button`).onclick = () => UI.deleteEquation(num);

        // Create C++ evaluator, generate mesh, and render
        bridge.createEvaluator('\\sin(x)', `equation${num}`, {'x': 0, 'y': 0}, UI.step, UI.range, UI.clipZ).then(res => {
            if (!res) Renderer.clear();
        });
    }

    // Delete an equation
    static deleteEquation(num) {
        const row1 = document.getElementById(`rowDisplay${num}`);
        const row2 = document.getElementById(`rowEquation${num}`);
        const promises = [];

        // Remove equation and meshes from C++ maps, delete mesh from renderer
        bridge.deleteEvaluator(`equation${num}`);
        Renderer.removeMesh(`equation${num}`);
        row1.remove();
        row2.remove();
    
        // Update IDs and event listeners for remaining equations
        if (num !== this.table.rows.length / 2 + 1) {
            for (let i = num + 1; i <= this.table.rows.length / 2 + 1; i++) {
                const oldRow1 = document.getElementById(`rowDisplay${i}`);
                const oldRow2 = document.getElementById(`rowEquation${i}`);

                // Clone and update the first row with decremented IDs
                const newRow1 = oldRow1.cloneNode(true);
                const newLabel = newRow1.querySelector('.label');
                const newDisplay = newRow1.querySelector(`#display${i}`);
                newRow1.id = `rowDisplay${i - 1}`;
                newLabel.id = `label${i - 1}`;
                newLabel.querySelector('div').textContent = i - 1;
                newDisplay.id = `display${i - 1}`;

                // Clone and update the second row
                const newRow2 = oldRow2.cloneNode(true);
                const newTextarea = newRow2.querySelector('textarea');
                newRow2.id = `rowEquation${i - 1}`;
                newTextarea.id = `equation${i - 1}`;

                // Replace old with new
                UI.table.replaceChild(newRow1, oldRow1);
                UI.table.replaceChild(newRow2, oldRow2);

                // Reattach clean event listeners
                newLabel.querySelector('button').onclick = () => UI.viewLatex(i - 1);
                newDisplay.querySelector('input').onchange = (e) => Renderer.updateColor(`equation${i - 1}`, e.target.value);
                newDisplay.querySelector('button').onclick = () => UI.deleteEquation(i - 1);
                newTextarea.addEventListener('input', () => UI.updateDisplay(i - 1));
                document.getElementById('clipZ').addEventListener('change', (e) => { UI.clipZ = e.target.checked; UI.updateDisplay(i - 1) });

                // Update the C++ evaluator and generate new mesh
                bridge.updateEvaluator(newTextarea.value, `equation${i - 1}`, {'x': 0, 'y': 0}, UI.step, UI.range, UI.clipZ).then(res => {
                    if (!res) Renderer.clear();
                });
            }
            bridge.deleteEvaluator(`equation${UI.table.rows.length / 2 + 1}`);
            Renderer.removeMesh(`equation${UI.table.rows.length / 2 + 1}`);
        } 
        MathJax.typesetPromise();
    }

    static updateRange(value) {
        if (value > 100) {
            document.getElementById('range').value = 100;
            value = 100;
        } else if (value < 1) {
            document.getElementById('range').value = 1;
            value = 1;
        }

        UI.range = value;
        UI.step = document.getElementById('meshResolution').value;
        const x = document.getElementById('xAxis');
        const y = document.getElementById('yAxis');
        const z = document.getElementById('zAxis');
        x.innerHTML = `<strong>X = ${value}`;
        y.innerHTML = `<strong>Y = ${value}`;
        z.innerHTML = `<strong>Z = ${value}`;
        UI.throttleUpdateMesh(value, UI.step, UI.clipZ);
    }

    static updateMeshResolution(value) {
        if (value > 200) {
            document.getElementById('meshResolution').value = 200;
            value = 200;
        } else if (value < 3) {
            if (value < 0) {
                document.getElementById('meshResolution').value = 1;
            }
            for (const mesh in Renderer.getMeshes) {
                Renderer.clearMesh(mesh);
            }
        }

        UI.step = value;
        UI.range = document.getElementById('range').value;
        UI.throttleUpdateMesh(UI.range, value, UI.clipZ);
    }
}
