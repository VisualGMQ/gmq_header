#version 430

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer InputBufferA {
    float A[];
};

layout(std430, binding = 1) buffer InputBufferB {
    float B[];
};

layout(std430, binding = 2) buffer OutputBuffer {
    float C[];
};

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index < 256) {
        C[index] = A[index] + B[index];
    }
}