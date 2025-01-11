#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <unistd.h>

#define DHT_PIN 4 // GPIO pin where DHT11 is connected

// Function to read from the DHT11 sensor
int read_dht11_data(int *temperature, int *humidity) {
    unsigned char data[5] = {0}; // Store 5 bytes of data
    int bit_index = 7;
    int byte_index = 0;
    int timeout = 1000;

    // Initialize GPIO pin as input
    gpioSetMode(DHT_PIN, PI_INPUT);

    // Start signal
    gpioWrite(DHT_PIN, PI_OUTPUT);
    gpioWrite(DHT_PIN, 0); // Low for 18 ms
    usleep(18000); 
    gpioWrite(DHT_PIN, 1); // High for 20-40 us
    usleep(20);
    gpioSetMode(DHT_PIN, PI_INPUT); // Set the pin back to input

    // Read the response from DHT11
    for (int i = 0; i < 40; i++) {
        timeout = 1000;
        while (gpioRead(DHT_PIN) == 0 && timeout-- > 0); // Wait for low signal
        usleep(30);
        
        if (gpioRead(DHT_PIN) == 1) {
            data[byte_index] |= (1 << bit_index); // Set the corresponding bit to 1
        }

        while (gpioRead(DHT_PIN) == 1 && timeout-- > 0); // Wait for high signal

        if (--bit_index < 0) {
            bit_index = 7;
            byte_index++;
        }

        if (timeout <= 0) {
            return -1; // Timeout error
        }
    }

    // Check for valid data by comparing checksum
    int checksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] != (checksum & 0xFF)) {
        return -1; // Checksum error
    }

    // Extract temperature and humidity data
    *humidity = data[0];
    *temperature = data[2];

    return 0; // Success
}

int main() {
    if (gpioInitialise() < 0) {
        printf("pigpio initialization failed.\n");
        return -1;
    }

    int temperature, humidity;
    
    while (1) {
        if (read_dht11_data(&temperature, &humidity) == 0) {
            printf("Temperature: %d C, Humidity: %d %%\n", temperature, humidity);
        } else {
            printf("Error reading from DHT11 sensor.\n");
        }
        sleep(5); // Wait 5 seconds before reading again
    }

    gpioTerminate(); // Clean up pigpio
    return 0;
}
