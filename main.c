#include "bme280.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#define IIC_Dev "/dev/i2c-1" //Raspberry 3B+ platform's default I2C device file
#define IIC_Dev2 "/dev/i2c-3"	
  
int fd;
double ptsPerFile = 1048575;

void user_delay_ms(uint32_t period)
{
  usleep(period*1000);
}

int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  write(fd, &reg_addr,1);
  read(fd, data, len);
  return 0;
}

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
  int8_t *buf;
  buf = malloc(len +1);
  buf[0] = reg_addr;
  memcpy(buf +1, data, len);
  write(fd, buf, len +1);
  free(buf);
  return 0;
}

int8_t stream_one_sensor_forced_mode(struct bme280_dev *dev) {
  int8_t rslt;
  uint8_t settings_sel;
  struct bme280_data comp_data;
  time_t timer;
  char fileName[100];
  char buffer[26];
  struct tm* tm_info;
  FILE * fPtr = NULL;
  int dayCounter = 1;
  double counter = 1;
  char day;
    
  //set oversampling and filter settings
  dev->settings.osr_h = BME280_OVERSAMPLING_1X;
  dev->settings.osr_p = BME280_OVERSAMPLING_1X;
  dev->settings.osr_t = BME280_OVERSAMPLING_1X;
  dev->settings.filter = BME280_FILTER_COEFF_OFF;
  settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
  rslt = bme280_set_sensor_settings(settings_sel, dev);
  
  printf("Data Logger started successfully and is currently running...\n");
  
  for (dayCounter = 1; dayCounter < 10; dayCounter += 1) {
    // create output file
    day = dayCounter + '0';
    snprintf(fileName, sizeof fileName, "%s%c%s", "day", day, ".txt");
    fPtr = fopen(fileName, "w");
    
    if (fPtr){
      // Continuously stream sensor data
      for (counter = 1; counter < ptsPerFile; counter += 1) {
        if (counter == 1){
          // print file header
          fprintf(fPtr, "Date Time # T(C) P(hPa) RH(%)\r\n");
        }
        //get date and time
        time(&timer);
        tm_info = localtime(&timer);
    
        //print date and time
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S ", tm_info);
        fputs(buffer, fPtr);
    
        // get data for sensor 1
        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
        dev->delay_ms(80); //Wait for the measurement to complete and print data @11.36Hz
        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
    
        //print sensor 1 data
        fprintf(fPtr, "%0.0lf %0.2f %0.3f %0.2f\r\n", counter, comp_data.temperature, comp_data.pressure/100, comp_data.humidity);
      }
      fclose(fPtr);
    }
  }
  return rslt;
}

int8_t stream_two_sensors_forced_mode(struct bme280_dev *dev1, struct bme280_dev *dev2)
{
  int8_t rslt1;
  int8_t rslt2;
  uint8_t settings_sel1;
  uint8_t settings_sel2;
  struct bme280_data comp_data1;
  struct bme280_data comp_data2;
  time_t timer;
  char buffer[26];
  struct tm* tm_info;
  FILE * fPtr = NULL;
  double counter = 0;
    
  // create output file for day1
  fPtr = fopen("day1.txt", "w");
  if (!fPtr){
    printf("Unable to open/create file 'day1'.\n");
    exit(1);
  }
    
  //set oversampling and filter settings for dev1
  dev1->settings.osr_h = BME280_OVERSAMPLING_1X;
  dev1->settings.osr_p = BME280_OVERSAMPLING_1X;
  dev1->settings.osr_t = BME280_OVERSAMPLING_1X;
  dev1->settings.filter = BME280_FILTER_COEFF_OFF;
  settings_sel1 = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
  rslt1 = bme280_set_sensor_settings(settings_sel1, dev1);
  
  //set oversampling and filter settings for dev2
  dev2->settings.osr_h = BME280_OVERSAMPLING_1X;
  dev2->settings.osr_p = BME280_OVERSAMPLING_1X;
  dev2->settings.osr_t = BME280_OVERSAMPLING_1X;
  dev2->settings.filter = BME280_FILTER_COEFF_OFF;
  settings_sel2 = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
  rslt2 = bme280_set_sensor_settings(settings_sel2, dev2);
  
  // print file header
  fprintf(fPtr, "Date Time # T1(C) P1(hPa) RH1(%) T2(C) P2(hPa) RH2(%)\r\n");
    
  /* Continuously stream sensor data */
  while (1) {
    //get date and time
    time(&timer);
    tm_info = localtime(&timer);
    
    //print date and time
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S ", tm_info);
    fputs(buffer, fPtr);
    
    // get data for sensor 1
    rslt1 = bme280_set_sensor_mode(BME280_FORCED_MODE, dev1);
    dev1->delay_ms(40); // wait for the measurement to complete and print data @25Hz
    rslt1 = bme280_get_sensor_data(BME280_ALL, &comp_data1, dev1);
    
    //print sensor 1 data
    fprintf(fPtr, "%0.0lf %0.2f %0.3f %0.2f", counter += 1, comp_data1.temperature, comp_data1.pressure/100, comp_data1.humidity);

    //get data for sensor 2
    rslt2 = bme280_set_sensor_mode(BME280_FORCED_MODE, dev2);
    dev2->delay_ms(40); // wait for the measurement to complete and print data @25Hz
    rslt2 = bme280_get_sensor_data(BME280_ALL, &comp_data2, dev2);
    
    //print sensor 2 data
    fprintf(fPtr, " %0.2f %0.3f %0.2f\r\n", comp_data2.temperature, comp_data2.pressure/100, comp_data2.humidity);
  }
  fclose(fPtr);
  return rslt1;
}

int main(int argc, char* argv[])
{
  struct bme280_dev dev1;
  struct bme280_dev dev2;
  int8_t rslt1 = BME280_OK;
  int8_t rslt2 = BME280_OK;
  
  //Initialize sensor on 0x77 (mandatory sensor)
  if ((fd = open(IIC_Dev, O_RDWR)) < 0){
    printf("Failed to open the i2c bus 1 %s", argv[1]);
    exit(1);
  } else {
    //printf("Opened I2C bus 1\n");
  }
  if (ioctl(fd, I2C_SLAVE, 0x77) < 0){
    printf("Failed to acquire bus access 0x77.\n");
    exit(1);
  } else {
  //printf("Acquired 0x77\n");  }
  dev1.dev_id = BME280_I2C_ADDR_SEC;
  dev1.intf = BME280_I2C_INTF;
  dev1.read = user_i2c_read;
  dev1.write = user_i2c_write;
  dev1.delay_ms = user_delay_ms;
  rslt1 = bme280_init(&dev1);
  }
/*  
  //initialize sensor on 0x76 (optional sensor)
  if ((fd = open(IIC_Dev2, O_RDWR)) < 0) {
    printf("Failed to open the i2c bus 3 %s", argv[1]);
    stream_one_sensor_forced_mode(&dev1);
  } else {
    printf("Opened I2C bus 3\n");
  }
  if (ioctl(fd, I2C_SLAVE, 0x76) < 0){
    printf("Unable to acquire 0x76, running single sensor mode.\n");
    stream_one_sensor_forced_mode(&dev1);
  } else {
    printf("Acquired 0x76");
    dev2.dev_id = BME280_I2C_ADDR_PRIM;
    dev2.intf = BME280_I2C_INTF;
    dev2.read = user_i2c_read;
    dev2.write = user_i2c_write;
    dev2.delay_ms = user_delay_ms;
    rslt2 = bme280_init(&dev2);
    stream_two_sensors_forced_mode(&dev1, &dev2);
  */
  stream_one_sensor_forced_mode(&dev1);
}
