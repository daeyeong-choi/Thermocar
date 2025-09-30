#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include <cmath>
#include "Serial.h"

DigitalOut myled_R(LED1);
I2C i2c(D4, D5);
I2C i2c_oled(D12, A6);
DigitalIn myswitch(D3);
Timer t1;
Serial pc(USBTX, USBRX, 9600);
Serial bt(D1, D0, 9600); // HC-06 tx, rx
Adafruit_SSD1306_I2c myOled(i2c_oled, A0, 0x78, 64, 128);


bool getTemp(float *temp_val)
{
    char ch1, ch2;
    char cmd[2];
    cmd[0] = 0x07;
    cmd[1] = 0;
    i2c.stop();
    wait_us(5);
    ch1 = i2c.write(0xb4, cmd, 1, true);
    if (ch1 != 0)
    {
        return false;
    }
    ch2 = i2c.read(0xb5, cmd, 2);
    if (ch2 != 0)
    {
        return false;
    }
    wait_us(5);
    *temp_val = ((((cmd[1] & 0x007f) << 8) + cmd[0]) * 0.02) - 273.15;
    return true;
}

bool getTempAmbient(float *temp_val)
{
    char ch1, ch2;
    char cmd[2];
    cmd[0] = 0x06;
    cmd[1] = 0;
    i2c.stop();
    wait_us(5);
    ch1 = i2c.write(0xb4, cmd, 1, true);
    if (ch1 != 0)
    {
        return false;
    }
    ch2 = i2c.read(0xb5, cmd, 2);
    if (ch2 != 0)
    {
        return false;
    }
    wait_us(5);
    *temp_val = ((((cmd[1] & 0x007f) << 8) + cmd[0]) * 0.02) - 273.15;
    return true;
}

bool getRawIR(int16_t *ir_val)
{
    char ch1, ch2;
    char cmd[2];
    uint16_t temp;
    cmd[0] = 0x04;
    cmd[1] = 0;
    i2c.stop();
    wait_us(5);
    ch1 = i2c.write(0xb4, cmd, 1, true);
    if (ch1 != 0)
    {
        return false;
    }
    ch2 = i2c.read(0xb5, cmd, 2);
    if (ch2 != 0)
    {
        return false;
    }
    wait_us(5);
    temp = (uint16_t)((cmd[1] << 8) | cmd[0]);
    if (temp & 0x8000)
        *ir_val = ~((int16_t)(temp & 0x7fff)) + 1;
    else
        *ir_val = (int16_t)temp;
    return true;
}

void myOled_config()
{
    myOled.begin();
    myOled.clearDisplay();
    myOled.setTextCursor(0, 0);
    myOled.printf("Press\n");
    myOled.printf("Button\n");
    myOled.display();
}

int main()
{
    float temp = 0.0;
    float amb_temp = 0.0;
    int16_t ir_data = 0;
    float final_temp = 0.0;
    float final_amb_temp = 0.0;
    float final = 0.0;
    float final_ir_data = 0.0;
    int num = 0;

    pc.printf("===============Hello world===============\n\r");
    myOled_config();
    myswitch.mode(PullDown);

    while (1)
    {
        myled_R = !myled_R;

        if (myswitch.read())
        {
            t1.start();
            num = 0;
            final_temp = 0.0;
            final_amb_temp = 0.0;
            final = 0.0;
            final_ir_data = 0;

            while (t1.read() <= 2)
            {
                myOled.setTextCursor(0, 54);
                myOled.clearDisplay();
                myOled.printf("Charging....\n", final_temp / num);
                myOled.display();

                if (getTemp(&temp) & getTempAmbient(&amb_temp) & getRawIR(&ir_data))
                {
                    final_temp = final_temp + temp;
                    final_amb_temp = final_amb_temp + amb_temp;
                    final_ir_data = final_ir_data + ir_data;
                    num = num + 1;
                }
            }

            float final = final_amb_temp + 0.070019728 * final_ir_data + 1.59059641;
            float final_rounded = round(final / num * 100) / 100;

            myOled.setTextCursor(10, 5);
            myOled.clearDisplay();
            myOled.drawRect(0, 0, 128, 64, WHITE);
            myOled.drawRect(1, 1, 126, 62, WHITE);
            myOled.printf("Surf Tem: %5.2f\n", final_temp / num);
            myOled.printf("Amb Temp: %5.2f\n", final_amb_temp / num);
            myOled.printf("IR_data: %5.2f\n", final_ir_data / num);
            myOled.printf("Final Temp: %5.1f\n", final_rounded + 1.6);
            myOled.display();
            t1.stop();
            t1.reset();
        }
    }
}