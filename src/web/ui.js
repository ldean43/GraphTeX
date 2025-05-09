class UI {

    constructor() {
        this.table = document.querySelector('table');
        this.init();
        document.getElementById('eq1').addEventListener('input', () => this.updateDisplay(1));
        document.getElementById('addeq').onclick = () => this.addEquation();
        document.querySelector('#label1 button').onclick = () => this.viewLatex(1);
        document.querySelector('#display1 button').onclick = () => this.deleteEquation(1);
    }

    async init() {
        const res = await bridge.createEvaluator('\\sin(x)', 'eq1', {'x': 0, 'y': 0});
        if (res) {
            const vertices = new Float32Array(await bridge.getVertices(`eq1`));
            const indices = new Int16Array(await bridge.getIndices(`eq1`));
            const normals = new Float32Array(await bridge.getNormals(`eq1`));
            renderer.addMesh(`eq1`, vertices, indices, normals);
        }
    }

    // Update the display with the current equation
    async updateDisplay(num) {
        const equation = document.getElementById(`eq${num}`).value;
        const display = document.getElementById(`display${num}`).querySelector('div');
        display.textContent = '\\[\\displaystyle{' + equation + '}\\]';
        MathJax.typesetPromise(); // Update MathJax
        const res = await bridge.updateEvaluator(equation, `eq${num}`, {'x': 0, 'y': 0}); // Update C++ evaluator
        if (res) {
            const vertices = new Float32Array(await bridge.getVertices(`eq${num}`));
            const indices = new Int16Array(await bridge.getIndices(`eq${num}`));
            const normals = new Float32Array(await bridge.getNormals(`eq${num}`));
            renderer.addMesh(`eq${num}`, vertices, indices, normals);
            renderer.render();
        }
    }

    // Toggle the visibility of the LaTeX input area
    viewLatex(num) {
        const row = document.getElementById(`treq${num}`);
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

    // Add a new equation row
    async addEquation() {
        const num = this.table.rows.length / 2 + 1; // Keep count of equations

        // Create first row
        const row1 = document.createElement('tr');
        row1.id = `trd${num}`;
        row1.innerHTML = `
            <td id="label${num}" class="label" rowspan="1">
                <div>${num}</div>
                <button class="viewbutton">▼</button>
            </td>
            <td id="display${num}">
                <div>\\[\\displaystyle{\\displaylines{\\sin(x)}}\\]</div>
                <button class="deleq">✖</button>
            </td>`;

        // Create second row (hidden by default)
        const row2 = document.createElement('tr');
        row2.id = `treq${num}`;
        row2.style.display = 'none';
        row2.innerHTML = `
            <td colspan="2">
                <textarea id="eq${num}" autofocus>\\sin(x)</textarea>
            </td>`;

        // Append rows to table
        this.table.appendChild(row1);
        this.table.appendChild(row2);
        MathJax.typesetPromise();

        // Set event listeners and create C++ evaluator
        document.getElementById(`eq${num}`).addEventListener('input', () => this.updateDisplay(num));
        document.querySelector(`#label${num} button`).onclick = () => this.viewLatex(num);
        document.querySelector(`#display${num} button`).onclick = () => this.deleteEquation(num);
        const res = await bridge.createEvaluator('\\sin(x)', `eq${num}`, {'x': 0, 'y': 0});
        if (res) {
            const vertices = new Float32Array(await bridge.getVertices(`eq${num}`));
            const indices = new Int16Array(await bridge.getIndices(`eq${num}`));
            const normals = new Float32Array(await bridge.getNormals(`eq${num}`));
            renderer.addMesh(`eq${num}`, vertices, indices, normals);
            renderer.render();
        }
    }

    // Delete an equation row
    async deleteEquation(num) {
        const row1 = document.getElementById(`trd${num}`);
        const row2 = document.getElementById(`treq${num}`);

        bridge.deleteEvaluator(`eq${num}`);
        renderer.removeMesh(`eq${num}`);
        row1.remove();
        row2.remove();
    
        for (let i = num + 1; i <= this.table.rows.length / 2 + 1; i++) {
            const oldRow1 = document.getElementById(`trd${i}`);
            const oldRow2 = document.getElementById(`treq${i}`);
    
            // Clone and update the first row
            const newRow1 = oldRow1.cloneNode(true);
            newRow1.id = `trd${i - 1}`;
            const newLabel = newRow1.querySelector('.label');
            newLabel.id = `label${i - 1}`;
            newLabel.querySelector('div').textContent = i - 1;
            const newDisplay = newRow1.querySelector(`#display${i}`);
            newDisplay.id = `display${i - 1}`;
    
            // Clone and update the second row
            const newRow2 = oldRow2.cloneNode(true);
            newRow2.id = `treq${i - 1}`;
            const newTextarea = newRow2.querySelector('textarea');
            newTextarea.id = `eq${i - 1}`;
    
            // Replace old with new
            this.table.replaceChild(newRow1, oldRow1);
            this.table.replaceChild(newRow2, oldRow2);
    
            // Reattach clean event listeners and update C++ evaluator
            newLabel.querySelector('button').onclick = () => this.viewLatex(i - 1);
            newDisplay.querySelector('button').onclick = () => this.deleteEquation(i - 1);
            newTextarea.addEventListener('input', () => this.updateDisplay(i - 1));
            await bridge.updateEvaluator(newTextarea.value, `eq${i - 1}`, {'x': 0, 'y': 0});
            const vertices = new Float32Array(await bridge.getVertices(`eq${i - 1}`));
            const indices = new Int16Array(await bridge.getIndices(`eq${i - 1}`));
            const normals = new Float32Array(await bridge.getNormals(`eq${i - 1}`));
            renderer.addMesh(`eq${num}`, vertices, indices, normals);
            if (i == this.table.rows.length / 2 + 1) {
                bridge.deleteEvaluator(`eq${i}`);
                bridge.removeMesh(`eq${i}`);
            }
        }
        MathJax.typesetPromise();
        renderer.render();
    }
}
