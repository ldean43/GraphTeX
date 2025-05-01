class Renderer {
    #vs = `
        #version 300 es
        precision highp float;
        in vec3 a_position;
        in vec3 a_normal;

        out vec3 v_normal;
        out vec3 v_to_light;
        out vec3 v_to_cam;
        out vec3 v_position;

        uniform mat4 u_matrix;
        uniform mat4 u_world;
        uniform vec3 light_pos;
        uniform vec3 cam_pos;

        void main() {
            v_normal = mat3(u_world) * a_normal;
            v_position = (u_world * vec4(a_position, 1.0)).xyz;
            v_to_light = light_pos - v_position;
            v_to_cam = cam_pos - v_position;
            gl_Position = u_matrix * vec4(a_position, 1);
        }`;
    #fs = ` 
        #version 300 es
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
            vec3 base_color = vec3(min(abs(v_position.z)/u_range, 1.0), 0.0, 1.0);

            float diffuse = max(dot(norm, light_dir), 0.0);
            float specular = pow(max(dot(cam_dir, reflect_dir), 0.0), 32.0);
            vec3 ambient = vec3(0.1);
            vec3 color = ambient + diffuse * base_color + specular * base_color;

            fragColor = vec4(color, 1.0);
        }
    `;

    constructor(canvas) {
        this.canvas = canvas;
        this.gl = canvas.getContext('webgl');

        // initlize meshes and indices for axes
        this.meshes = {'x': new Float32Array(-1, 0, 0, 1, 0, 0),
                        'y': new Float32Array(0, -1, 0, 0, 1, 0),
                        'z': new Float32Array(0, 0, -1, 0, 0, 1)};
        this.indices = {'x': new Uint16Array([0, 1]),
                        'y': new Uint16Array([0, 1]),
                        'z': new Uint16Array([0, 1])};

        if (!this.gl) {
        console.error('WebGL not supported, falling back on experimental-webgl');
        this.gl = canvas.getContext('experimental-webgl');
        }
        if (!this.gl) {
            qDebug('Unable to initialize WebGL. Qt may not support it.');
        }
    }
    
    clear() {
        this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
    }
    
    setViewport(x, y, width, height) {
        this.gl.viewport(x, y, width, height);
    }

    render() {

    }

}