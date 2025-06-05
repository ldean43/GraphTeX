class Renderer {
    // general webgl properties
    static #gl;
    static #canvas;
    static #meshes;
    // properties for projection and view matrices
    static #matrices = {
        worldMatrix: undefined,
        cameraMatrix: undefined,
        projectionMatrix: undefined
    }
    static #varLocations = {
        linePositionLocation: undefined,
        lineProjectionMatrixLocation: undefined,
        phongPositionLocation: undefined,
        normalLocation: undefined,
        lightLocation: undefined,
        camLocation: undefined,
        rangeLocation: undefined,
        worldMatrixLocaiton: undefined,
        phongProjectionMatrixLocation: undefined,
        stencilPositionLocation: undefined,
        stencilRangeLocation: undefined
    }
    static #cameraConfig = {
        yaw: 0,
        pitch: 0,
        camPos: undefined,
        distance: 10
    }
    static #shaders = {
        linevs: `#version 300 es
        precision highp float;
        in vec3 a_position;

        uniform mat4 u_matrix;
         
        void main() {
            gl_Position = u_matrix * vec4(a_position, 1);
        }`,
        linefs: `#version 300 es
        precision highp float;

        out vec4 fragColor;
        
        void main() {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }`,
        phongvs: `#version 300 es
        precision highp float;
        in vec3 a_position;
        in vec3 a_normal;

        out vec3 v_normal;
        out vec3 v_to_light;
        out vec3 v_to_cam;
        out vec3 v_position;
        out vec3 v_raw;

        uniform mat4 u_matrix;
        uniform mat4 u_world;
        uniform vec3 u_light_pos;
        uniform vec3 u_cam_pos;

        void main() {
            v_normal = mat3(u_world) * a_normal;
            v_position = (u_world * vec4(a_position, 1.0)).xyz;
            v_raw = a_position;
            v_to_light = u_light_pos - v_position;
            v_to_cam = u_cam_pos - v_position;
            gl_Position = u_matrix * vec4(a_position, 1);
        }`,
        phongfs: `#version 300 es
        precision highp float;

        in vec3 v_normal;
        in vec3 v_to_light;
        in vec3 v_to_cam;
        in vec3 v_position;
        in vec3 v_raw;

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
            fragColor = vec4(color, 1);
        }`
    };
    static #stencilProgram;
    static #lineProgram;
    static #phongProgram;
    static #displayWidth;
    static #displayHeight;
    static #range = 10;
    static #isDragging = false;
    static #mouseX = 0;
    static #mouseY = 0;
        
    static init(canvas) {
        Renderer.#canvas = canvas;
        Renderer.#gl = canvas.getContext('webgl2');
        Renderer.#canvas.width = Renderer.#canvas.clientWidth;
        Renderer.#canvas.height = Renderer.#canvas.clientHeight;
        Renderer.#gl.enable(Renderer.#gl.DEPTH_TEST);
        Renderer.#gl.enable(Renderer.#gl.STENCIL_TEST);
        Renderer.#gl.enable(Renderer.#gl.SAMPLE_ALPHA_TO_COVERAGE);
        Renderer.#gl.clearStencil(0);

        if (!Renderer.#gl) {
            console.error('WebGL not supported, falling back on experimental-webgl');
            Renderer.#gl = canvas.getContext('experimental-webgl');
        }
        if (!Renderer.#gl) {
            qDebug('Unable to initialize WebGL. Qt may not support it.');
        }

        Renderer.#gl.clearDepth(1.0);
        Renderer.#meshes = {axes: {vertices: new Float32Array([
                                                -10, 0, 0,
                                                10, 0, 0,
                                                0, -10, 0, 
                                                0, 10, 0,
                                                0, 0, -10,
                                                0, 0, 10]),
                                indices: new Uint16Array([0, 1, 
                                                            2, 3, 
                                                            4, 5]),
                                buffers: [Renderer.#gl.createBuffer(), Renderer.#gl.createBuffer()],
                                vao: Renderer.#gl.createVertexArray()}};

        // set up line shaders
        Renderer.#lineProgram = webglUtils.createProgramFromSources(Renderer.#gl, [Renderer.#shaders.linevs, Renderer.#shaders.linefs]);
        Renderer.#gl.useProgram(Renderer.#lineProgram);
        Renderer.#varLocations.linePositionLocation = Renderer.#gl.getAttribLocation(Renderer.#lineProgram, "a_position");
        Renderer.#varLocations.lineProjectionMatrixLocation = Renderer.#gl.getUniformLocation(Renderer.#lineProgram, "u_matrix");

        // set up axes vao
        Renderer.#gl.bindVertexArray(Renderer.#meshes.axes.vao);
        Renderer.#gl.bindBuffer(Renderer.#gl.ARRAY_BUFFER, Renderer.#meshes.axes.buffers[0]);
        Renderer.#gl.bufferData(Renderer.#gl.ARRAY_BUFFER, Renderer.#meshes.axes.vertices, Renderer.#gl.STATIC_DRAW);
        Renderer.#gl.enableVertexAttribArray(Renderer.#varLocations.linePositionLocation);
        Renderer.#gl.vertexAttribPointer(Renderer.#varLocations.linePositionLocation, 3, Renderer.#gl.FLOAT, false, 0, 0);
        Renderer.#gl.bindBuffer(Renderer.#gl.ELEMENT_ARRAY_BUFFER, Renderer.#meshes.axes.buffers[1]);
        Renderer.#gl.bufferData(Renderer.#gl.ELEMENT_ARRAY_BUFFER, Renderer.#meshes.axes.indices, Renderer.#gl.STATIC_DRAW);
        Renderer.#gl.bindVertexArray(null);
        
        // set up phong shaders
        Renderer.#phongProgram = webglUtils.createProgramFromSources(Renderer.#gl, [Renderer.#shaders.phongvs, Renderer.#shaders.phongfs]);
        Renderer.#gl.useProgram(Renderer.#phongProgram);
        Renderer.#varLocations.phongPositionLocation = Renderer.#gl.getAttribLocation(Renderer.#phongProgram, "a_position");
        Renderer.#varLocations.normalLocation = Renderer.#gl.getAttribLocation(Renderer.#phongProgram, "a_normal");
        Renderer.#varLocations.phongProjectionMatrixLocation = Renderer.#gl.getUniformLocation(Renderer.#phongProgram, "u_matrix");
        Renderer.#varLocations.worldMatrixLocation = Renderer.#gl.getUniformLocation(Renderer.#phongProgram, "u_world");
        Renderer.#varLocations.lightLocation = Renderer.#gl.getUniformLocation(Renderer.#phongProgram, "u_light_pos");
        Renderer.#varLocations.camLocation = Renderer.#gl.getUniformLocation(Renderer.#phongProgram, "u_cam_pos");
        Renderer.#varLocations.rangeLocation = Renderer.#gl.getUniformLocation(Renderer.#phongProgram, "u_range");

        const fov = Math.PI / 4;
        const aspect = Renderer.#canvas.clientWidth / Renderer.#canvas.clientHeight;
        const zNear = 0.1;
        const zFar = 100;
        const camDistance = Renderer.#cameraConfig.distance / Math.tan(fov / 2);
        Renderer.#matrices.projectionMatrix = m4.perspective(fov, aspect, zNear, zFar);
        Renderer.#cameraConfig.camPos = m4.scaleVector(m4.normalize([0,0,1]), camDistance);
        Renderer.#matrices.cameraMatrix = m4.inverse(m4.lookAt(Renderer.#cameraConfig.camPos, [0, 0, 0], [0, 1, 0]));
        Renderer.#matrices.worldMatrix = m4.xRotation(-Math.PI/2);
        Renderer.#matrices.worldViewProjectionMatrix = 
            m4.multiply(Renderer.#matrices.projectionMatrix,
                m4.multiply(Renderer.#matrices.cameraMatrix, Renderer.#matrices.worldMatrix));

        Renderer.#canvas.addEventListener('pointerdown', (e) => {
            Renderer.#isDragging = true;
            Renderer.#mouseX = e.clientX;
            Renderer.#mouseY = e.clientY;
        });

        Renderer.#canvas.addEventListener('pointerup', () => {
            Renderer.#isDragging = false;
        });

        Renderer.#canvas.addEventListener('pointermove', (e) => {
            if (Renderer.#isDragging) {
                const deltaX = e.clientX - Renderer.#mouseX;
                const deltaY = e.clientY - Renderer.#mouseY;
                Renderer.#mouseX = e.clientX;
                Renderer.#mouseY = e.clientY;
                Renderer.#cameraConfig.yaw += deltaX * 0.01;
                Renderer.#cameraConfig.pitch += deltaY * 0.01;
                Renderer.#cameraConfig.yaw = (Renderer.#cameraConfig.yaw % (2 * Math.PI) + (2 * Math.PI)) % (2 * Math.PI)
                Renderer.#cameraConfig.pitch = Math.max(-Math.PI / 2 + .1, Math.min(Math.PI/2 - .1, Renderer.#cameraConfig.pitch % (2 * Math.PI) + (2 * Math.PI) % (2 * Math.PI)))
                Renderer.updateCameraMatrix();
            }
        });

        window.addEventListener('resize', () => {
            requestAnimationFrame(() => {
                console.log('Resizing canvas', Renderer.#canvas.clientWidth, Renderer.#canvas.clientHeight);
                Renderer.#canvas.width = Renderer.#canvas.clientWidth;
                Renderer.#canvas.height = Renderer.#canvas.clientHeight;
                Renderer.#gl.viewport(0, 0, Renderer.#canvas.width, Renderer.#canvas.height);
                Renderer.updateProjectionMatrix();
            });
        });

        Renderer.#canvas.addEventListener('wheel', (e) => {
            const delta = e.deltaY;
            Renderer.#cameraConfig.distance += delta * .01;
            Renderer.#cameraConfig.distance = Math.max(Renderer.#cameraConfig.distance, 1);
            Renderer.#cameraConfig.distance = Math.min(Renderer.#cameraConfig.distance, 20);
            Renderer.updateCameraMatrix();
        });
    }

    static updateCameraMatrix() {
        const camDistance = (Renderer.#cameraConfig.distance) / Math.tan(Math.PI / 4 / 2);
        // spherical coordinates
        const x = camDistance * Math.cos(Renderer.#cameraConfig.pitch) * Math.sin(Renderer.#cameraConfig.yaw);
        const y = camDistance * Math.sin(Renderer.#cameraConfig.pitch);
        const z = camDistance * Math.cos(Renderer.#cameraConfig.pitch) * Math.cos(Renderer.#cameraConfig.yaw);
        Renderer.#cameraConfig.camPos = [x, y, z];
        Renderer.#matrices.cameraMatrix = m4.inverse(m4.lookAt(Renderer.#cameraConfig.camPos, [0, 0, 0], [0, 1, 0]));
        Renderer.#matrices.worldViewProjectionMatrix = m4.multiply(Renderer.#matrices.projectionMatrix, m4.multiply(Renderer.#matrices.cameraMatrix, Renderer.#matrices.worldMatrix));
        Renderer.render();
    }

    static updateProjectionMatrix() {
        const fov = Math.PI / 4;
        const aspect = Renderer.#canvas.width / Renderer.#canvas.height;
        const zNear = 0.1;
        const zFar = 1000;
        Renderer.#matrices.projectionMatrix = m4.perspective(fov, aspect, zNear, zFar);
        Renderer.#matrices.worldViewProjectionMatrix = m4.multiply(Renderer.#matrices.projectionMatrix, m4.multiply(Renderer.#matrices.cameraMatrix, Renderer.#matrices.worldMatrix));
        Renderer.render();
    }

    static updateAxesDivs() {
        const xDiv = document.getElementById('xAxis');
        const yDiv = document.getElementById('yAxis');
        const zDIv = document.getElementById('zAxis');
        let x = [11, 0, 0, 1]
        let y = [0, 11, 0, 1]
        let z = [0, 0, 11, 1]

        x = m4.transformVector(Renderer.#matrices.worldViewProjectionMatrix, x);
        y = m4.transformVector(Renderer.#matrices.worldViewProjectionMatrix, y);
        z = m4.transformVector(Renderer.#matrices.worldViewProjectionMatrix, z);
        // Divide by the homogeneous coordinate
        x[0] /= x[3]; x[1] /= x[3];
        y[0] /= y[3]; y[1] /= y[3];
        z[0] /= z[3]; z[1] /= z[3];
        // Convert to pixels
        x[0] = (x[0] * .5 + .5) * Renderer.#canvas.width; 
        x[1] = (x[1] * -.5 + .5) * Renderer.#canvas.height;
        y[0] = (y[0] * .5 + .5) * Renderer.#canvas.width; 
        y[1] = (y[1] * -.5 + .5) * Renderer.#canvas.height;
        z[0] = (z[0] * .5 + .5) * Renderer.#canvas.width; 
        z[1] = (z[1] * -.5 + .5) * Renderer.#canvas.height;
        xAxis.style.left = Math.floor(x[0]) + 'px';
        xAxis.style.top = Math.floor(x[1]) + 'px';
        yAxis.style.left = Math.floor(y[0]) + 'px';
        yAxis.style.top = Math.floor(y[1]) + 'px';
        zAxis.style.left = Math.floor(z[0]) + 'px';
        zAxis.style.top = Math.floor(z[1]) + 'px';
    }

    static addMesh(name, vertices, indices, normals) {
        Renderer.#meshes[name] = {};
        Renderer.#meshes[name].vertices = vertices;
        Renderer.#meshes[name].indices = indices;
        Renderer.#meshes[name].normals = normals;

        Renderer.#meshes[name].vao = Renderer.#gl.createVertexArray();
        Renderer.#meshes[name].buffers = [Renderer.#gl.createBuffer(), Renderer.#gl.createBuffer(), Renderer.#gl.createBuffer()];
        Renderer.#gl.bindVertexArray(Renderer.#meshes[name].vao);
        // Binding position buffer
        Renderer.#gl.bindBuffer(Renderer.#gl.ARRAY_BUFFER, Renderer.#meshes[name].buffers[0]);
        Renderer.#gl.bufferData(Renderer.#gl.ARRAY_BUFFER, vertices, Renderer.#gl.STATIC_DRAW);
        Renderer.#gl.enableVertexAttribArray(Renderer.#varLocations.phongPositionLocation);
        Renderer.#gl.vertexAttribPointer(Renderer.#varLocations.phongPositionLocation, 3, Renderer.#gl.FLOAT, false, 0, 0);
        // Binding normal buffer
        Renderer.#gl.bindBuffer(Renderer.#gl.ARRAY_BUFFER, Renderer.#meshes[name].buffers[1]);
        Renderer.#gl.bufferData(Renderer.#gl.ARRAY_BUFFER, normals, Renderer.#gl.STATIC_DRAW);
        Renderer.#gl.enableVertexAttribArray(Renderer.#varLocations.normalLocation);
        Renderer.#gl.vertexAttribPointer(Renderer.#varLocations.normalLocation, 3, Renderer.#gl.FLOAT, false, 0, 0);
        // Binding index buffer
        Renderer.#gl.bindBuffer(Renderer.#gl.ELEMENT_ARRAY_BUFFER, Renderer.#meshes[name].buffers[2]);
        Renderer.#gl.bufferData(Renderer.#gl.ELEMENT_ARRAY_BUFFER, indices, Renderer.#gl.STATIC_DRAW);
        Renderer.#gl.bindVertexArray(null);
    }

    static removeMesh(name) {
        Renderer.#gl.deleteBuffer(Renderer.#meshes[name].buffers[0]);
        Renderer.#gl.deleteBuffer(Renderer.#meshes[name].buffers[1]);
        Renderer.#gl.deleteBuffer(Renderer.#meshes[name].buffers[2]);
        Renderer.#gl.deleteVertexArray(Renderer.#meshes[name].vao);
        delete Renderer.#meshes[name];
    }

    static render() {
        Renderer.#gl.viewport(0, 0, Renderer.#canvas.width, Renderer.#canvas.height);

        // Clear the canvas
        Renderer.#gl.clearColor(0.0, 0.0, 0.0, 0.0); // Make sure canvas is visible
        Renderer.#gl.clearDepth(1.0);
        Renderer.#gl.depthFunc(Renderer.#gl.LEQUAL);
        Renderer.#gl.clear(Renderer.#gl.COLOR_BUFFER_BIT | Renderer.#gl.DEPTH_BUFFER_BIT);
    
        const axesVAO = Renderer.#meshes.axes.vao;
    
        Renderer.#gl.useProgram(Renderer.#lineProgram);
        Renderer.#gl.bindVertexArray(axesVAO);
        Renderer.#gl.uniformMatrix4fv(Renderer.#varLocations.lineProjectionMatrixLocation, false, Renderer.#matrices.worldViewProjectionMatrix);
        Renderer.#gl.lineWidth(1);
        Renderer.#gl.drawElements(Renderer.#gl.LINES, 6, Renderer.#gl.UNSIGNED_SHORT, 0);
        Renderer.updateAxesDivs();
        for (const name in Renderer.#meshes) {
            if (name === 'axes') continue;
            const mesh = Renderer.#meshes[name];
            const lightPos = [20,20,20];

            Renderer.#gl
    
            Renderer.#gl.useProgram(Renderer.#phongProgram);
            Renderer.#gl.bindVertexArray(mesh.vao);
            Renderer.#gl.uniformMatrix4fv(Renderer.#varLocations.phongProjectionMatrixLocation, false, Renderer.#matrices.worldViewProjectionMatrix);
            Renderer.#gl.uniformMatrix4fv(Renderer.#varLocations.worldMatrixLocation, false, Renderer.#matrices.worldMatrix);
            Renderer.#gl.uniform3fv(Renderer.#varLocations.lightLocation, lightPos);
            Renderer.#gl.uniform3fv(Renderer.#varLocations.camLocation, Renderer.#cameraConfig.camPos);
            Renderer.#gl.uniform1f(Renderer.#varLocations.rangeLocation, Renderer.#range);
            console.log(`Drawing ${name}`);
            Renderer.#gl.drawElements(Renderer.#gl.TRIANGLE_STRIP, mesh.indices.length, Renderer.#gl.UNSIGNED_SHORT, 0);
        }
    }
}