// OpenCL kernel for transforming points
__kernel void move_points(
        __global float* points,
        __global float* draw_array,
        __global float* transforms,
        const int numPoints)
{
    int index = get_global_id(0);

    // Safety check
    if (index < numPoints)
    {
        int offset = index * 4;

        // Based on code from graphics.c - vectorMult()
        for (int i = 0; i < 4; i++)
        {
            float sum = 0.0f;
            for (int j = 0; j < 4; j++)
            {
                sum += points[offset + j] * transforms[j * 4 + i];
            }
            draw_array[offset + i] = sum;
        }

        // Apply ncurses scaling
        draw_array[offset + 0] *= 20.0f;
        draw_array[offset + 1] *= 20.0f;
        draw_array[offset + 2] *= 20.0f;
        
        draw_array[offset + 0] += 50.0f;
        draw_array[offset + 1] += 50.0f;
        draw_array[offset + 2] += 50.0f;
    }
} 