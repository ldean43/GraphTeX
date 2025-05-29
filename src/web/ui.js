class UI {
    static table;

    static async init() {
        // Initialize event listeners for first 2 rows of the table
        UI.table = document.querySelector('table');
        document.querySelector('#display1 button').onclick = () => UI.deleteEquation(1);
        document.querySelector('#label1 button').onclick = () => UI.viewLatex(1);
        document.getElementById('equation1').addEventListener('input', () => UI.updateDisplay(1));
        document.getElementById('addEquation').onclick = () => UI.addEquation();

        //
        let start = performance.now(); 
        const res = await bridge.createEvaluator('\\sin(x)', 'equation1', {'x': 0, 'y': 0});
        console.log(`Awaited bridge.createEvaluator for ${performance.now() - start} milliseconds.`);
        if (res) {
            let start = performance.now();
            const vertices = base64toFloat32(await bridge.getVertices('equation1'));
            console.log(`Awaited bridge.getVertices for ${performance.now() - start} milliseconds.`);
            const indices = base64toInt16(await bridge.getIndices('equation1'));
            const normals = base64toFloat32(await bridge.getNormals('equation1'));
            Renderer.addMesh('equation1', vertices, indices, normals);
            Renderer.render();
        }
    }

    // Update the display with the current equation
    static async updateDisplay(num) {
        // Update MathJaX
        const equation = document.getElementById(`equation${num}`).value;
        const display = document.getElementById(`display${num}`).querySelector('div');
        display.textContent = '\\[\\displaystyle{' + equation + '}\\]';
        MathJax.typesetPromise();

        // Update C++ evaluator, generate mesh, and render
        const res = await bridge.updateEvaluator(equation, `equation${num}`, {'x': 0, 'y': 0}); // Update C++ evaluator
        if (res) {
            const vertices = base64toFloat32(await bridge.getVertices(`equation${num}`));
            const indices = base64toInt16(await bridge.getIndices(`equation${num}`));
            const normals = base64toFloat32(await bridge.getNormals(`equation${num}`));
            Renderer.addMesh(`equation${num}`, vertices, indices, normals);
            Renderer.render();
        }
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
    static async addEquation() {
        const num = this.table.rows.length / 2 + 1;

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
        document.querySelector(`#label${num} button`).onclick = () => UI.viewLatex(num);
        document.querySelector(`#display${num} button`).onclick = () => UI.deleteEquation(num);

        // Create C++ evaluator, generate mesh, and render
        const res = await bridge.createEvaluator('\\sin(x)', `equation${num}`, {'x': 0, 'y': 0});
        if (res) {
            const vertices = base64toFloat32(await bridge.getVertices(`equation${num}`));
            const indices = base64toInt16(await bridge.getIndices(`equation${num}`));
            const normals = base64toFloat32(await bridge.getNormals(`equation${num}`));
            Renderer.addMesh(`equation${num}`, vertices, indices, normals);
            Renderer.render();
        }
    }

    // Delete an equation
    static async deleteEquation(num) {
        const row1 = document.getElementById(`rowDisplay${num}`);
        const row2 = document.getElementById(`rowEquation${num}`);

        // Remove equation and meshes from C++ maps, delete mesh from renderer
        bridge.deleteEvaluator(`equation${num}`);
        Renderer.removeMesh(`equation${num}`);
        row1.remove();
        row2.remove();
    
        // Update IDs and event listeners for remaining equations
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
            newLabel.querySelector('button').onclick = () => this.viewLatex(i - 1);
            newDisplay.querySelector('button').onclick = () => this.deleteEquation(i - 1);
            newTextarea.addEventListener('input', () => this.updateDisplay(i - 1));

            // Update the C++ evaluator and generate new mesh
            await bridge.updateEvaluator(newTextarea.value, `equation${i - 1}`, {'x': 0, 'y': 0});
            const vertices = base64toFloat32(await bridge.getVertices(`equation${i - 1}`));
            const indices = base64toInt16(await bridge.getIndices(`equation${i - 1}`));
            const normals = base64toFloat32(await bridge.getNormals(`equation${i - 1}`));
            Renderer.addMesh(`equation${num}`, vertices, indices, normals);
        }
        bridge.deleteEvaluator(`equation${UI.table.rows.length / 2 + 1}`);
        Renderer.removeMesh(`equation${UI.table.rows.length / 2 + 1}`);
        MathJax.typesetPromise();
        Renderer.render();
    }
}
