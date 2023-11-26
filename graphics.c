/* 3D Graphics using ASCII graphics
   -original NCurses code from "Game Programming in C with the Ncurses Library"
    https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
    and from "NCURSES Programming HOWTO"
    http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef NOGRAPHICS
#include <unistd.h>
#include <ncurses.h>
#endif

// OpenCL include header
#include <CL/cl.h>

#define DELAY 10000

// maximum screen size, both height and width
#define SCREENSIZE 100
// default number of iterations to run before exiting, only used
// when graphics are turned off
#define ITERATIONS 1000

#define PROGRAM_FILE "graphics.cl"
#define KERNEL_FUNC "move_points"

// number of points
int pointCount;
// array of points before transformation
float **pointArray;
// array of points after transformation
float **drawArray;

// transformation matrix
float transformArray[4][4];
// display buffers
char frameBuffer[SCREENSIZE][SCREENSIZE];
float depthBuffer[SCREENSIZE][SCREENSIZE];

void vectorMult(float a[4], float b[4], float c[4][4]);

#ifndef NOGRAPHICS
// maximum screen dimensions
int max_y = 0, max_x = 0;
#endif

#ifndef NOGRAPHICS
int drawPoints()
{
   int c, i, j;
   float multx, multy;
   float point[4];

   // update screen maximum size
   getmaxyx(stdscr, max_y, max_x);

   // used to scale position of points based on screen size
   multx = (float)max_x / SCREENSIZE;
   multy = (float)max_y / SCREENSIZE;

   clear();

   // display points
   for (i = 0; i < SCREENSIZE; i++)
   {
      for (j = 0; j < SCREENSIZE; j++)
      {
         if (frameBuffer[i][j] == 'X')
            mvprintw(i * multy, j * multx, "X");
         else if (frameBuffer[i][j] == 'o')
            mvprintw(i * multy, j * multx, "o");
         else if (frameBuffer[i][j] == '.')
            mvprintw(i * multy, j * multx, ".");
      }
   }

   refresh();

   usleep(DELAY);

   // read keyboard and exit if 'q' pressed
   c = getch();
   if (c == 'q')
      return (1);
   else
      return (0);
}
#endif

/* calculates the product of matrices b and c
        stores the result in matrix a */
void matrixMult(float a[4][4], float b[4][4], float c[4][4])
{
   int row, col, element;

   for (row = 0; row < 4; row++)
   {
      for (col = 0; col < 4; col++)
      {
         a[row][col] = 0.0;
         for (element = 0; element < 4; element++)
         {
            a[row][col] += b[row][element] * c[element][col];
         }
      }
   }
}

/* calculates the product of vector b and matrix c
        stores the result in vector a */
void vectorMult(float a[4], float b[4], float c[4][4])
{
   int col, element;

   for (col = 0; col < 4; col++)
   {
      a[col] = 0.0;
      for (element = 0; element < 4; element++)
      {
         a[col] += b[element] * c[element][col];
      }
   }
}

void allocateArrays()
{
   int i;

   pointArray = malloc(sizeof(float *) * pointCount);
   for (i = 0; i < pointCount; i++)
      pointArray[i] = malloc(sizeof(float) * 4);

   drawArray = malloc(sizeof(float *) * pointCount);
   for (i = 0; i < pointCount; i++)
      drawArray[i] = malloc(sizeof(float) * 4);
}

void cubePointArray()
{
   pointArray[0][0] = 0.5;
   pointArray[0][1] = 0.0;
   pointArray[0][2] = 0.5;
   pointArray[0][3] = 1.0;

   pointArray[1][0] = 0.5;
   pointArray[1][1] = 0.0;
   pointArray[1][2] = -0.5;
   pointArray[1][3] = 1.0;

   pointArray[2][0] = -0.5;
   pointArray[2][1] = 0.0;
   pointArray[2][2] = -0.5;
   pointArray[2][3] = 1.0;

   pointArray[3][0] = -0.5;
   pointArray[3][1] = 0.0;
   pointArray[3][2] = 0.5;
   pointArray[3][3] = 1.0;

   pointArray[4][0] = 0.5;
   pointArray[4][1] = 1.0;
   pointArray[4][2] = 0.5;
   pointArray[4][3] = 1.0;

   pointArray[5][0] = 0.5;
   pointArray[5][1] = 1.0;
   pointArray[5][2] = -0.5;
   pointArray[5][3] = 1.0;

   pointArray[6][0] = -0.5;
   pointArray[6][1] = 1.0;
   pointArray[6][2] = -0.5;
   pointArray[6][3] = 1.0;

   pointArray[7][0] = -0.5;
   pointArray[7][1] = 1.0;
   pointArray[7][2] = 0.5;
   pointArray[7][3] = 1.0;
}

void randomPointArray()
{
   int i;
   float val;

   for (i = 0; i < pointCount; i++)
   {
      val = (float)random() / 10000.0;
      pointArray[i][0] = 2.5 * ((val - trunc(val)) - 0.5);
      val = (float)random() / 10000.0;
      pointArray[i][1] = 2.5 * ((val - trunc(val)) - 0.5);
      val = (float)random() / 10000.0;
      pointArray[i][2] = 2.5 * ((val - trunc(val)) - 0.5);
      val = (float)random() / 10000.0;
      pointArray[i][3] = 2.5 * ((val - trunc(val)) - 0.5);
   }
}

void initTransform()
{
   int i, j;

   for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
         if (i == j)
            transformArray[i][j] = 1.0;
         else
            transformArray[i][j] = 0.0;
}

void xRot(int rot)
{
   float oneDegree = 0.017453;
   float angle, sinAngle, cosAngle;
   float result[4][4];
   int i, j;
   float rotx[4][4] = {1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0};

   angle = (float)rot * oneDegree;
   sinAngle = sinf(angle);
   cosAngle = cosf(angle);

   rotx[1][1] = cosAngle;
   rotx[2][2] = cosAngle;
   rotx[1][2] = -sinAngle;
   rotx[2][1] = sinAngle;

   matrixMult(result, transformArray, rotx);

   // copy result to transformation matrix
   for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
         transformArray[i][j] = result[i][j];
}

void yRot(int rot)
{
   float oneDegree = 0.017453;
   float angle, sinAngle, cosAngle;
   float result[4][4];
   int i, j;
   float roty[4][4] = {1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0};

   angle = (float)rot * oneDegree;
   sinAngle = sinf(angle);
   cosAngle = cosf(angle);

   roty[0][0] = cosAngle;
   roty[2][2] = cosAngle;
   roty[0][2] = sinAngle;
   roty[2][0] = -sinAngle;

   matrixMult(result, transformArray, roty);

   // copy result to transformation matrix
   for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
         transformArray[i][j] = result[i][j];
}

void zRot(int rot)
{
   float oneDegree = 0.017453;
   float angle, sinAngle, cosAngle;
   float result[4][4];
   int i, j;
   float rotz[4][4] = {1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0};

   angle = (float)rot * oneDegree;
   sinAngle = sinf(angle);
   cosAngle = cosf(angle);

   rotz[0][0] = cosAngle;
   rotz[1][1] = cosAngle;
   rotz[0][1] = -sinAngle;
   rotz[1][0] = sinAngle;

   matrixMult(result, transformArray, rotz);

   // copy result to transformation matrix
   for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
         transformArray[i][j] = result[i][j];
}

void translate(float x, float y, float z)
{
   transformArray[3][0] = x;
   transformArray[3][1] = y;
   transformArray[3][2] = z;
}

void clearBuffers()
{
   int i, j;

   // empty the frame buffer
   // set the depth buffer to a large distance
   for (i = 0; i < SCREENSIZE; i++)
   {
      for (j = 0; j < SCREENSIZE; j++)
      {
         frameBuffer[i][j] = ' ';
         depthBuffer[i][j] = -1000.0;
      }
   }
}

// Flattens a 2D array into a 1D array
float *flatten_array(float **array, int i, int j)
{
   float *flattened_array = malloc(sizeof(float) * i * j);
   if (flattened_array == NULL)
   {
      perror("Failed to allocate memory for flattened_array");
      exit(1);
   }
   int k, l;
   for (k = 0; k < i; k++)
   {
      for (l = 0; l < j; l++)
      {
         flattened_array[k * j + l] = array[k][l];
      }
   }
   return flattened_array;
}

// Sends the pointArray, drawArray, and transformArray to OpenCL Kernel and applies the transformations to the drawArray
void transformPointArray(cl_context context, cl_command_queue queue, cl_kernel kernel, float *flattened_pointArray, float *flattened_drawArray)
{
   // Set local and global work size dimensions for OpenCL
   size_t local_size = 64;
   size_t global_size = ((pointCount + local_size - 1) / local_size) * local_size;
   cl_int err;

   // Create buffers
   cl_mem point_buffer, transform_buffer, draw_buffer;

   // Flatten transformArray into a 1D array (it is not a float ** array like the others)
   float *flattened_transformArray = malloc(sizeof(float) * 4 * 4);
   for (int i = 0; i < 4; i++)
   {
      flattened_transformArray[i * 4] = transformArray[i][0];
      flattened_transformArray[i * 4 + 1] = transformArray[i][1];
      flattened_transformArray[i * 4 + 2] = transformArray[i][2];
      flattened_transformArray[i * 4 + 3] = transformArray[i][3];
   }

   // Create a buffer for points in OpenCL
   point_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 4 * pointCount, flattened_pointArray, &err);
   if (err < 0)
   {
      perror("Failed to create buffer for pointArray");
      exit(1);
   }

   // READ_WRITE is used because we need to write to the draw buffer (WRITE_ONLY is really slow)
   draw_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * 4 * pointCount, flattened_drawArray, &err);
   if (err < 0)
   {
      perror("Failed to create buffer for drawArray");
      exit(1);
   }

   // We do not need to write to the transform buffer, so we can use CL_MEM_READ_ONLY
   transform_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 4 * 4, flattened_transformArray, &err);
   if (err < 0)
   {
      perror("Failed to create buffer for transformArray");
      exit(1);
   }

   // Set the four kernel arguments: point_buffer, draw_buffer, transform_buffer, and pointCount
   err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &point_buffer);
   if (err < 0)
   {
      perror("Failed to set point_buffer kernel argument");
      exit(1);
   }

   err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &draw_buffer);
   if (err < 0)
   {
      perror("Failed to set draw_buffer kernel argument");
      exit(1);
   }

   err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &transform_buffer);
   if (err < 0)
   {
      perror("Failed to set transform_buffer kernel argument");
      exit(1);
   }

   err = clSetKernelArg(kernel, 3, sizeof(cl_int), &pointCount);
   if (err < 0)
   {
      perror("Failed to set pointCount kernel argument");
      exit(1);
   }

   // Enqueue kernel
   err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
   if (err < 0)
   {
      printf("Error: %d\n", err);
      perror("Failed to enqueue kernel");
      exit(1);
   }

   //Read the kernel's output
   err = clEnqueueReadBuffer(queue, draw_buffer, CL_TRUE, 0, sizeof(float) * 4 * pointCount, flattened_drawArray, 0, NULL, NULL);
   if (err < 0)
   {
      printf("Error: %d\n", err);
      perror("Failed to read draw_buffer");
      exit(1);
   }

   // Copy transformedPoints back into drawArray
   for (int i = 0; i < pointCount; i++)
   {
      drawArray[i][0] = flattened_drawArray[i * 4];
      drawArray[i][1] = flattened_drawArray[i * 4 + 1];
      drawArray[i][2] = flattened_drawArray[i * 4 + 2];
      drawArray[i][3] = flattened_drawArray[i * 4 + 3];
   }

   // Free allocated memory
   free(flattened_transformArray);

   // Free OpenCL resources
   clReleaseMemObject(point_buffer);
   clReleaseMemObject(transform_buffer);
   clReleaseMemObject(draw_buffer);
}

void movePoints(cl_context context, cl_command_queue queue, cl_kernel kernel, float *flattened_pointArray, float *flattened_drawArray)
{
   static int counter = 1;
   int i;
   int x, y;

   // initialize transformation matrix
   // this needs to be done before the transformation is performed
   initTransform();

   // add some rotations to the transformation matrix
   // this needs to be done before the transformation is performed
   xRot(counter);
   yRot(counter);
   counter++;

   // apply the transformation to the points using OpenCL
   // OpenCL kernel setup and execution is done in transformPointArray() just so that this function is not too long
   transformPointArray(context, queue, kernel, flattened_pointArray, flattened_drawArray);

   // clears buffers before drawing screen
   clearBuffers();

   // draw the screen
   // adds points to the frame buffer, use depth buffer to
   // sort points based upon distance from the viewer
   for (i = 0; i < pointCount; i++)
   {
      x = (int)drawArray[i][0];
      y = (int)drawArray[i][1];
      if (depthBuffer[x][y] < drawArray[i][2])
      {
         if (drawArray[i][2] > 60.0)
            frameBuffer[x][y] = 'X';
         else if (drawArray[i][2] < 40.0)
            frameBuffer[x][y] = '.';
         else
            frameBuffer[x][y] = 'o';
         depthBuffer[x][y] = drawArray[i][2];
      }
   }
}

// Based on the example from the add_numbers.c file
cl_device_id create_device()
{
   cl_platform_id platform;
   cl_device_id dev;
   int err;

   // Identify a platform
   err = clGetPlatformIDs(1, &platform, NULL);
   if (err < 0)
   {
      perror("Couldn't identify a platform");
      exit(1);
   }

   // Access a device
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if (err == CL_DEVICE_NOT_FOUND)
   {
      printf("GPU not found, trying CPU...\n");
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if (err < 0)
   {
      perror("Couldn't access any devices");
      exit(1);
   }


   // Check if device is GPU
   cl_device_type type;
   err = clGetDeviceInfo(dev, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
   if (err < 0)
   {
      perror("Couldn't get device type");
      exit(1);
   }
   if (type == CL_DEVICE_TYPE_GPU)
      printf("Device is GPU\n");
   else if (type == CL_DEVICE_TYPE_CPU)
      printf("Device is CPU\n");
   else
      printf("Device is not GPU or CPU\n");

   // Check device name
   char name[128];
   err = clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(name), &name, NULL);
   if (err < 0)
   {
      perror("Couldn't get device name");
      exit(1);
   }
   printf("Device name: %s\n", name);

   return dev;
}

// Based on the example from the add_numbers.c file
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename)
{
   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
   int err;

   // Read program file and place content into buffer
   program_handle = fopen(filename, "r");
   if (program_handle == NULL)
   {
      perror("Couldn't find the program file");
      exit(1);
   }

   // Get program file size
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);

   // Read program file into buffer allocated on the stack
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   // Create program from file
   program = clCreateProgramWithSource(ctx, 1, (const char**)&program_buffer, &program_size, &err);
   if (err < 0)
   {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   // Build program
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if (err < 0)
   {
      // Find size of log and print to std output
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

      // Allocate log space
      program_log = (char*)malloc(log_size + 1);
      program_log[log_size] = '\0';

      // Get log and print to std output
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
      printf("%s\n", program_log);

      free(program_log);
      exit(1);
   }

   return program;
}

int main(int argc, char *argv[])
{
   int i, count;
   int argPtr;
   int drawCube, drawRandom;

   // OpenCL variables
   cl_device_id device;
   cl_context context;
   cl_command_queue queue;
   cl_program program;
   cl_kernel kernel;
   cl_int err;

   // set number of iterations, only used for timing tests
   // not used in curses version
   count = ITERATIONS;

   // initialize drawing objects
   drawCube = 0;
   drawRandom = 0;

   // read command line arguments for number of iterations
   if (argc > 1)
   {
      argPtr = 1;
      while (argPtr < argc)
      {
         if (strcmp(argv[argPtr], "-i") == 0)
         {
            sscanf(argv[argPtr + 1], "%d", &count);
            argPtr += 2;
         }
         else if (strcmp(argv[argPtr], "-cube") == 0)
         {
            drawCube = 1;
            pointCount = 8;
            argPtr += 1;
         }
         else if (strcmp(argv[argPtr], "-points") == 0)
         {
            drawRandom = 1;
            sscanf(argv[argPtr + 1], "%d", &pointCount);
            argPtr += 2;
         }
         else
         {
            printf("USAGE: %s <-i iterations> <-cube | -points #>\n", argv[0]);
            printf(" iterations -the number of times the population will be updated\n");
            printf("    the number of iterations only affects the non-curses program\n");
            printf(" the curses program exits when q is pressed\n");
            printf(" choose either -cube to draw the cube shape or -points # to\n");
            printf("    draw random points where # is an integer number of points to draw\n");
            exit(1);
         }
      }
   }

   // allocate space for arrays to store point position
   allocateArrays();
   if (drawCube == 1)
      cubePointArray();
   else if (drawRandom == 1)
      randomPointArray();
   else
   {
      printf("You must choose either <-cube> or <-points #> on the command line.\n");
      exit(1);
   }

   // Create OpenCL device and context
   device = create_device();
   context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
   if (err < 0)
   {
      perror("Couldn't create a context");
      exit(1);
   }

   // Build the program
   program = build_program(context, device, PROGRAM_FILE);

   // Create a command queue
   queue = clCreateCommandQueue(context, device, 0, &err);
   if (err < 0)
   {
      perror("Couldn't create a command queue");
      exit(1);
   }

   // Create a kernel
   kernel = clCreateKernel(program, KERNEL_FUNC, &err);
   if (err < 0)
   {
      perror("Couldn't create a kernel");
      exit(1);
   };

   // Flatten pointArray into a 1D array
   float *flattened_pointArray = flatten_array(pointArray, pointCount, 4);

   // Flatten drawArray into a 1D array
   float *flattened_drawArray = flatten_array(drawArray, pointCount, 4);

#ifndef NOGRAPHICS
   // initialize ncurses
   initscr();
   noecho();
   cbreak();
   timeout(0);
   curs_set(FALSE);
   // Global var `stdscr` is created by the call to `initscr()`
   getmaxyx(stdscr, max_y, max_x);
#endif

   // draw and move points using ncurses
   // do not calculate timing in this loop, ncurses will reduce performance
#ifndef NOGRAPHICS
   while (1)
   {
      if (drawPoints() == 1)
         break;
      movePoints(context, queue, kernel, flattened_pointArray, flattened_drawArray);
   }
#endif

   // calculate movement of points but do not use ncurses to draw
#ifdef NOGRAPHICS
   printf("Number of iterations %d\n", count);

   for (i = 0; i < count; i++)
   {
      movePoints(context, queue, kernel, flattened_pointArray, flattened_drawArray);
   }
#endif

#ifndef NOGRAPHICS
   // shut down ncurses
   endwin();
#endif

   // Deallocate OpenCL resources
   clReleaseKernel(kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
}
