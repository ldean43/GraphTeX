class Renderer {
    // general webgl properties
    #gl;
    #canvas;
    #meshes;
    // properties for projection and view matrices
    #worldMatrix;
    #cameraMatrix;
    #projectionMatrix;
    #worldViewProjectionMatrix;
    #displayWidth;
    #displayHeight;
    #camPos;
    #range = 10;
    #yaw = 0;
    #pitch = 0;
    #isDragging = false;
    #mouseX = 0;
    #mouseY = 0;
    // properties for shaders
    #lineProgram;
    #linePositionLocation;
    #lineProjectionMatrixLocation;

    #phongProgram;
    #phongPositionLocation;
    #normalLocation;
    #lightLocation;
    #camLocation;
    #rangeLocation;
    #worldMatrixLocation;
    #phongProjectionMatrixLocation;
    

    #linevs = `#version 300 es
        precision highp float;
        in vec3 a_position;

        uniform mat4 u_matrix;
         
        void main() {
            gl_Position = u_matrix * vec4(a_position, 1);
        }`;

    #linefs = `#version 300 es
        precision highp float;

        out vec4 fragColor;
        
        void main() {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }`;

    #phongvs = `#version 300 es
        precision highp float;
        in vec3 a_position;
        in vec3 a_normal;

        out vec3 v_normal;
        out vec3 v_to_light;
        out vec3 v_to_cam;
        out vec3 v_position;

        uniform mat4 u_matrix;
        uniform mat4 u_world;
        uniform vec3 u_light_pos;
        uniform vec3 u_cam_pos;

        void main() {
            v_normal = mat3(u_world) * a_normal;
            v_position = (u_world * vec4(a_position, 1.0)).xyz;
            v_to_light = u_light_pos - v_position;
            v_to_cam = u_cam_pos - v_position;
            gl_Position = u_matrix * vec4(a_position, 1);
        }`;

    #phongfs = `#version 300 es
        precision highp float;

        in vec3 v_normal;
        in vec3 v_to_light;
        in vec3 v_to_cam;
        in vec3 v_position;

        out vec4 fragColor;
        
        uniform float u_range;

        void main() {
            vec3 norm = normalize(v_normal);
            vec3 light_dir = normalize(v_to_light);
            vec3 cam_dir = normalize(v_to_cam);
            vec3 reflect_dir = reflect(-light_dir, norm);
            vec3 base_color = vec3(max(abs(v_position.z)/u_range, 0.0), 0.0, .5);

            float diffuse = max(dot(norm, light_dir), 0.0);
            float specular = pow(max(dot(cam_dir, reflect_dir), 0.0), 32.0);
            vec3 ambient = vec3(0.1);
            vec3 color = ambient + diffuse * base_color + specular * vec3(1.0, 1.0, 1.0);
            color = clamp(color, 0.0, 1.0);

            fragColor = vec4(color, 1.0);
        }`;

    constructor(canvas) {
        this.#canvas = canvas;
        this.#gl = canvas.getContext('webgl2');

        if (!this.#gl) {
            console.error('WebGL not supported, falling back on experimental-webgl');
            this.#gl = canvas.getContext('experimental-webgl');
        }
        if (!this.#gl) {
            qDebug('Unable to initialize WebGL. Qt may not support it.');
        }

        this.#gl.clearDepth(1.0);
        this.#meshes = {"axes": {"vertices": new Float32Array([
                                                -10, 0, 0,
                                                10, 0, 0,
                                                0, -10, 0, 
                                                0, 10, 0,
                                                0, 0, -10,
                                                0, 0, 10]),
                                "indices": new Uint16Array([0, 1, 
                                                            2, 3, 
                                                            4, 5]),
                                "buffers": [this.#gl.createBuffer(), this.#gl.createBuffer()],
                                "vao": this.#gl.createVertexArray()}};

        // set up line shaders
        this.#lineProgram = webglUtils.createProgramFromSources(this.#gl, [this.#linevs, this.#linefs]);
        this.#gl.useProgram(this.#lineProgram);
        this.#linePositionLocation = this.#gl.getAttribLocation(this.#lineProgram, "a_position");
        this.#lineProjectionMatrixLocation = this.#gl.getUniformLocation(this.#lineProgram, "u_matrix");

        // set up axes vao
        this.#gl.bindVertexArray(this.#meshes['axes']['vao']);
        this.#gl.bindBuffer(this.#gl.ARRAY_BUFFER, this.#meshes['axes']['buffers'][0]);
        this.#gl.bufferData(this.#gl.ARRAY_BUFFER, this.#meshes['axes']['vertices'], this.#gl.STATIC_DRAW);
        this.#gl.enableVertexAttribArray(this.#linePositionLocation);
        this.#gl.vertexAttribPointer(this.#linePositionLocation, 3, this.#gl.FLOAT, false, 0, 0);
        this.#gl.bindBuffer(this.#gl.ELEMENT_ARRAY_BUFFER, this.#meshes['axes']['buffers'][1]);
        this.#gl.bufferData(this.#gl.ELEMENT_ARRAY_BUFFER, this.#meshes['axes']['indices'], this.#gl.STATIC_DRAW);
        this.#gl.bindVertexArray(null);
        
        // set up phong shaders
        this.#phongProgram = webglUtils.createProgramFromSources(this.#gl, [this.#phongvs, this.#phongfs]);
        this.#gl.useProgram(this.#phongProgram);
        this.#phongPositionLocation = this.#gl.getAttribLocation(this.#phongProgram, "a_position");
        this.#normalLocation = this.#gl.getAttribLocation(this.#phongProgram, "a_normal");
        this.#phongProjectionMatrixLocation = this.#gl.getUniformLocation(this.#phongProgram, "u_matrix");
        this.#worldMatrixLocation = this.#gl.getUniformLocation(this.#phongProgram, "u_world");
        this.#lightLocation = this.#gl.getUniformLocation(this.#phongProgram, "u_light_pos");
        this.#camLocation = this.#gl.getUniformLocation(this.#phongProgram, "u_cam_pos");
        this.#rangeLocation = this.#gl.getUniformLocation(this.#phongProgram, "u_range");

        const fov = Math.PI / 4;
        const aspect = this.#canvas.clientWidth / this.#canvas.clientHeight;
        console.log('width and height: ', this.#canvas.clientWidth, this.#canvas.clientHeight);
        const zNear = 0.1;
        const zFar = 1000;
        const distance = (this.#range) / Math.tan(fov / 2);
        this.#projectionMatrix = m4.perspective(fov, aspect, zNear, zFar);
        this.#camPos = m4.scaleVector(m4.normalize([0,0,1]), distance);
        this.#cameraMatrix = m4.inverse(m4.lookAt(this.#camPos, [0, 0, 0], [0, 1, 0]));
        this.#worldMatrix = m4.identity();
        this.#worldViewProjectionMatrix = m4.multiply(this.#projectionMatrix, m4.multiply(this.#cameraMatrix, this.#worldMatrix));

        this.#canvas.addEventListener('pointerdown', (e) => {
            this.#isDragging = true;
            this.#mouseX = e.clientX;
            this.#mouseY = e.clientY;
        });
        this.#canvas.addEventListener('pointerup', () => {
            this.#isDragging = false;
        });
        this.#canvas.addEventListener('pointermove', (e) => {
            if (this.#isDragging) {
                const deltaX = e.clientX - this.#mouseX;
                const deltaY = e.clientY - this.#mouseY;
                this.#mouseX = e.clientX;
                this.#mouseY = e.clientY;
                this.#yaw += deltaX * 0.01;
                this.#pitch += deltaY * 0.01;
                this.#yaw = this.#yaw % (2 * Math.PI);
                this.#pitch = this.#pitch % (2 * Math.PI);
                this.updateCameraMatrix();
            }
        });

        window.addEventListener('resize', () => {
            requestAnimationFrame(() => {
                console.log('Resizing canvas', this.#canvas.clientWidth, this.#canvas.clientHeight);
                this.#canvas.width = this.#canvas.clientWidth;
                this.#canvas.height = this.#canvas.clientHeight;
                this.#gl.viewport(0, 0, this.#canvas.width, this.#canvas.height);
                renderer.updateProjectionMatrix();
            });
        });

        this.#canvas.addEventListener('wheel', (e) => {
            const delta = e.deltaY;
            this.#range += delta * .01;
            this.#range = Math.max(this.#range, 0.1);
            this.#range = Math.min(this.#range, 100);
            this.updateCameraMatrix();
        });
    }

    updateCameraMatrix() {
        const camDistance = (this.#range) / Math.tan(Math.PI / 4 / 2);
        // spherical coordinates
        const x = camDistance * Math.cos(this.#pitch) * Math.sin(this.#yaw);
        const y = camDistance * Math.sin(this.#pitch);
        const z = camDistance * Math.cos(this.#pitch) * Math.cos(this.#yaw);
        this.#camPos = [x, y, z];
        this.#cameraMatrix = m4.inverse(m4.lookAt(this.#camPos, [0, 0, 0], [0, 1, 0]));
        this.#worldViewProjectionMatrix = m4.multiply(this.#projectionMatrix, m4.multiply(this.#cameraMatrix, this.#worldMatrix));
        this.render();
    }

    updateProjectionMatrix() {
        const fov = Math.PI / 4;
        const aspect = this.#canvas.width / this.#canvas.height;
        const zNear = 0.1;
        const zFar = 1000;
        this.#projectionMatrix = m4.perspective(fov, aspect, zNear, zFar);
        this.#worldViewProjectionMatrix = m4.multiply(this.#projectionMatrix, m4.multiply(this.#cameraMatrix, this.#worldMatrix));
        this.render();
    }

    addMesh(name, vertices, indices, normals) {
        this.#meshes[name] = {};
        this.#meshes[name]['vertices'] = vertices;
        this.#meshes[name]['indices'] = indices;
        this.#meshes[name]['normals'] = normals;

        this.#meshes[name]['vao'] = this.#gl.createVertexArray();
        this.#meshes[name]['buffers'] = [this.#gl.createBuffer(), this.#gl.createBuffer(), this.#gl.createBuffer()];
        this.#gl.bindVertexArray(this.#meshes[name]['vao']);
        // binding position buffer
        this.#gl.bindBuffer(this.#gl.ARRAY_BUFFER, this.#meshes[name]['buffers'][0]);
        this.#gl.bufferData(this.#gl.ARRAY_BUFFER, vertices, this.#gl.STATIC_DRAW);
        this.#gl.enableVertexAttribArray(this.#phongPositionLocation);
        this.#gl.vertexAttribPointer(this.#phongPositionLocation, 3, this.#gl.FLOAT, false, 0, 0);
        // binding normal buffer
        this.#gl.bindBuffer(this.#gl.ARRAY_BUFFER, this.#meshes[name]['buffers'][1]);
        this.#gl.bufferData(this.#gl.ARRAY_BUFFER, normals, this.#gl.STATIC_DRAW);
        this.#gl.enableVertexAttribArray(this.#normalLocation);
        this.#gl.vertexAttribPointer(this.#normalLocation, 3, this.#gl.FLOAT, false, 0, 0);
        // binding index buffer
        this.#gl.bindBuffer(this.#gl.ELEMENT_ARRAY_BUFFER, this.#meshes[name]['buffers'][2]);
        this.#gl.bufferData(this.#gl.ELEMENT_ARRAY_BUFFER, indices, this.#gl.STATIC_DRAW);
        this.#gl.bindVertexArray(null);
    }

    removeMesh(name) {
        this.#gl.deleteBuffer(this.#meshes[name]['buffers'][0]);
        this.#gl.deleteBuffer(this.#meshes[name]['buffers'][1]);
        this.#gl.deleteBuffer(this.#meshes[name]['buffers'][2]);
        this.#gl.deleteVertexArray(this.#meshes[name]['vao']);
        delete this.#meshes[name];
    }

    render() {
        this.#canvas.width = this.#canvas.clientWidth;
        this.#canvas.height = this.#canvas.clientHeight;
        this.#gl.viewport(0, 0, this.#canvas.width, this.#canvas.height);

        // Clear the canvas
        this.#gl.clearColor(0.0, 0.0, 0.0, 0.0); // Make sure canvas is visible
        this.#gl.clearDepth(1.0);
        this.#gl.enable(this.#gl.DEPTH_TEST);
        this.#gl.depthFunc(this.#gl.LEQUAL);
        this.#gl.clear(this.#gl.COLOR_BUFFER_BIT | this.#gl.DEPTH_BUFFER_BIT);
    
        const axesVAO = this.#meshes['axes']['vao'];
        const axesIndices = this.#meshes['axes']['indices'];
    
        this.#gl.useProgram(this.#lineProgram);
        this.#gl.bindVertexArray(axesVAO);
        this.#gl.uniformMatrix4fv(this.#lineProjectionMatrixLocation, false, this.#worldViewProjectionMatrix);
        this.#gl.lineWidth(1);
        this.#gl.drawElements(this.#gl.LINES, 6, this.#gl.UNSIGNED_SHORT, 0);
    
        for (const name in this.#meshes) {
            if (name === 'axes') continue;
            const mesh = this.#meshes[name];
            const lightPos = [20,20,20];
    
            this.#gl.useProgram(this.#phongProgram);
            this.#gl.bindVertexArray(mesh.vao);
            this.#gl.uniformMatrix4fv(this.#phongProjectionMatrixLocation, false, this.#worldViewProjectionMatrix);
            this.#gl.uniformMatrix4fv(this.#worldMatrixLocation, false, this.#worldMatrix);
            this.#gl.uniform3fv(this.#lightLocation, lightPos);
            this.#gl.uniform3fv(this.#camLocation, this.#camPos);
            this.#gl.uniform1f(this.#rangeLocation, this.#range);
            console.log(`Drawing ${name}`);
            this.#gl.drawElements(this.#gl.TRIANGLE_STRIP, mesh.indices.length, this.#gl.UNSIGNED_SHORT, 0);
        }
    }
}