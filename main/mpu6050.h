// #ifndef MPU6050_H
// #define MPU6050_H

// extern float final_data[7];

// static void i2c_init(void);
// static void mpu_task(void* arg);

// #endif

#ifndef MPU6050_H
#define MPU6050_H

extern float final_data[7];

void i2c_init(void);
void mpu_task(void *arg);

#endif