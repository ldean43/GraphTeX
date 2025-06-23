# GraphTeX
A 3D graphing calculator for LaTeX math expressions

![GraphTeXDemo-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/58acff1e-ee1f-4c41-960a-065d9e2b02ee)

## Download
Download for Windows is in releases. The application is located in the build folder, named GraphTeX.exe. You need to allow Windows to run the app by clicking "More Info..." when prompted. The app relies on JS files located in src/web/ and their paths are calculated relative to the executable's path. So if you plan to alter the folder structure, ensure that src/web/ is acessible via ../src/web/ relative to GraphTeX.exe.

## How to Use
### Basic input
The left side of the window lists your equations. Each box represents one equation to be graphed. Press the downward arrow on the left side of the equation to view the input box.

![Capture](https://github.com/user-attachments/assets/80e03ebd-80ad-47be-8aae-d991c3b0a5c8)

![Capture](https://github.com/user-attachments/assets/0fb5d863-5b9f-41c8-8680-8a5c32db2fca)

Pressing the 'X' in the top right corner of the box will delete the equation from your list and remove the graph from the canvas.

Pressing the color swatch will open a window to change the color of the graph.

![Capture](https://github.com/user-attachments/assets/d576d124-b65b-43c4-a9da-e5eaf8462104)

### Advanced Settings
The bottom left of the window features 6 options to alter the graph.

![Capture](https://github.com/user-attachments/assets/2e4d9847-3b64-43df-b7f5-d1a74e6064f8)

* Range defines the sample space for x and y values. For example, with a range of 1, the graph generated has x and y values in the range [-1, 1].

* Mesh resolution defines the amount of samples taken per row and column. It also determines the number of triangles per row and column. For example, a mesh resolution of 200 means that 200x200 = 40,000 points will be generated.

* Light X and Y rotation are sliders for the point light's location. Each slider is in terms of spherical coordinates. The pitch, Light X, ranges from [0, 180] and the yaw, Light Y, ranges from [0, 360]

* The shader selection allows you to choose from one of 4 shaders. The currently supported shaders are Diffuse, Phong, Wireframe, and Points.

* Clip Z is a true/false value that determines whether you want to clip z values outside of the bounding box. With Clip Z unchecked, you will see triangles drawn above and below the veiwable z-axis.

## Supported LaTeX

* \sin(), \cos(), \tan()
* \arcsin(), \arccos(), \arctan()
* \csc(), \sec(), \cot()
* \sinh(), \cosh(), \tanh()
* \log(), \log_{}(), \ln(), \lg()
* \sqrt(), \sqrt{}()
* \frac{}{}
* ^, ^{}
* +, -, *, /
