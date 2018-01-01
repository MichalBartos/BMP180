// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// check_BMP180
//Implementation of BMP180 sensor into nagios/icinga monitoring system

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>

void show_help(void) {
    puts("Usage:");
    puts("Chip configuration");
    puts("--address        -a       BMP085/BMP180 address (default 0x77)");
    puts("--bus            -b       I2C bus (default 1)");
    puts("Check configuration");
    puts("--critical_t     -c       Critical level for temperature -> high,low (default 100,-100)");
    puts("--warning_t      -w       Warning level for temperature -> high,low (default 50,-50)");
    puts("--critical_p     -t       Critical level for pressure -> high,low(default 1600,500)");
    puts("--warning_p      -p       Warning level for pressure -> high,low(default 1100,800)");
    puts("Output configuration");
    puts("--fahrenheit     -f       Show values in Fahrenheit");
    puts("--help           -h       This information");
}

int main (int argc, char **argv)
{
    int c;

    int device_address = 0x77;
    char bus_interface[11];
    int critical_level_t_high = 100;
    int warning_level_t_high = 50;
    int critical_level_t_low = -100;
    int warning_level_t_low = -50;
    int critical_level_p_high = 1600;
    int warning_level_p_high = 1100;
    int critical_level_p_low = 500;
    int warning_level_p_low = 800;
    int show_fahrenheit = 0;

    char* high;
    char* low;
    char dest[10] = "";
    char dest2[10] = "";


    while (1)
    {
        static struct option long_options[] =
        {
            {"address"    ,required_argument, 0, 'a'},
            {"bus"        ,required_argument, 0, 'b'},
            {"critical_t" ,required_argument, 0, 'c'},
            {"warning_t"  ,required_argument, 0, 'w'},
            {"fahrenheit" ,      no_argument, 0, 'f'},
            {"help"       ,      no_argument, 0, 'h'},
            {"critical_p" ,required_argument, 0, 't'},
            {"warning_p"  ,required_argument, 0, 'p'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long (argc, argv, "fht:p:a:b:c:w:",
                         long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0) {
                break;
            }
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;

        case 'a':
            device_address = strtol(optarg, NULL, 16);
            break;

        case 'b':
            if ((atoi (optarg) < 10)&&(atoi (optarg) > 0)){
                sprintf(bus_interface, "/dev/i2c-%d", atoi (optarg));
            }
            break;

        case 'c':
            strcpy(dest,optarg);
            high = strtok (dest,",");
            critical_level_t_high = atoi(high);
            low = strtok (NULL, ",");
            critical_level_t_low = atoi(low);
            break;

        case 'w':
            strcpy(dest2,optarg);
            high = strtok (dest2,",");
            warning_level_t_high = atoi(high);
            low = strtok (NULL, ",");
            warning_level_t_low = atoi(low);
            break;
        
        case 't':
            strcpy(dest,optarg);
            high = strtok (dest,",");
            critical_level_p_high = atoi(high);
            low = strtok (NULL, ",");
            critical_level_p_low = atoi(low);
            break;

        case 'p':
            strcpy(dest2,optarg);
            high = strtok (dest2,",");
            warning_level_p_high = atoi(high);
            low = strtok (NULL, ",");
            warning_level_p_low = atoi(low);
            break;

        case 'f':
            show_fahrenheit = 1;
            break;

        case '?':
            show_help();
            break;

        default:
            abort();
        }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

      	// Create I2C bus
	int file;
	//char *bus = interface;
	if((file = open(bus_interface, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(3);
	}
	// Get I2C device, VEML6070 I2C address is 0x77(119)
	ioctl(file, I2C_SLAVE, device_address);

	// Calibration Cofficients stored in EEPROM of the device
	// Read 22 bytes of data from address 0xAA(170)

        
        char data[22] = {0};
        char reg_data[1] = {0xAA};      
	write(file, reg_data, 1);
	read(file, data, 22);

        // Convert the data
        long AC1 = data[0] * 256 + data[1];
        if (AC1 > 32767) { AC1 -= 65535; }
        long AC2 = data[2] * 256 + data[3];
        if (AC2 > 32767) { AC2 -= 65535; }
        long AC3 = data[4] * 256 + data[5];
        if (AC3 > 32767) { AC3 -= 65535; }
        long AC4 = data[6] * 256 + data[7];
        long AC5 = data[8] * 256 + data[9];
        long AC6 = data[10] * 256 + data[11];
        long B1 = data[12] * 256 + data[13];
        if (B1 > 32767) { B1 -= 65535; }
        long B2 = data[14] * 256 + data[15];
        if (B2 > 32767) { B2 -= 65535; }
        long MB = data[16] * 256 + data[17];
        if (MB > 32767) { MB -= 65535; }
        long MC = data[18] * 256 + data[19];
        if (MC > 32767) { MC -= 65535; }
        long MD = data[20] * 256 + data[21];
        if (MD > 32767) { MD -= 65535; }
        sleep(1);

	// Select measurement control register(0xF4)
	// Enable temperature measurement(0x2E)
	char config[2]= {0};
	config[0] = 0xF4;
	config[1] = 0x2E;
	write(file, config, 2);
	sleep(1);
	
	// Read 2 bytes of data from register(0xF6)
	// temp msb, temp lsb
	char reg[1] = {0xF6};
	write(file, reg, 1);
	if(read(file, data, 2) != 2)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(3);
	}

	// Convert the data
	int temp = (data[0] * 256 + data[1]);

	// Select measurement control register(0xf4)
	// Enable pressure measurement, OSS = 1(0x74)
	config[0] = 0xF4;
	config[1] = 0x74;
	write(file, config, 2);
	sleep(1);
	
	// Read 3 bytes of data from register(0xF6)
	// pres msb1, pres msb, pres lsb
	reg[0] = 0xF6;
	write(file, reg, 1);
	read(file, data, 3);

	// Convert the data
	double pres = (data[0] * 65536 + (data[1] * 256) + data[2]) / 128;

	// Callibration for Temperature
	double X1 = (temp - AC6) * AC5 / 32768.0;
	double X2 = (MC * 2048.0) / (X1 + MD);
	double B5 = X1 + X2;
	double cTemp = ((B5 + 8.0) / 16.0) / 10.0;
        double fTemp = cTemp * 1.8 + 32;
        
        // Calibration for Pressure
        double B6 = B5 - 4000;
        X1 = (B2 * (B6 * B6 / 4096.0)) / 2048.0;
        X2 = AC2 * B6 / 2048.0;
        double X3 = X1 + X2;
        double B3 = (((AC1 * 4 + X3) * 2) + 2) / 4.0;
        X1 = AC3 * B6 / 8192.0;
        X2 = (B1 * (B6 * B6 / 2048.0)) / 65536.0;
        X3 = ((X1 + X2) + 2) / 4.0;
        double B4 = AC4 * (X3 + 32768) / 32768.0;
        double B7 = ((pres - B3) * (25000.0));
        double pressure = 0.0;
        if(B7 < 2147483648LU)
        {
                pressure = (B7 * 2) / B4;
        }
        else
        {
                pressure = (B7 / B4) * 2;
        }
        X1 = (pressure / 256.0) * (pressure / 256.0);
        X1 = (X1 * 3038.0) / 65536.0;
        X2 = ((-7357) * pressure) / 65536.0;
        double pressure1 = (pressure + (X1 + X2 + 3791) / 16.0) / 100;

        // Calculate Altitude
        double altitude = 44330 * (1 - pow(pressure1/1013.25, 0.1903));
        
        if (show_fahrenheit == 0)
        {
            printf("Temperature = %0.2f °C Pressure = %0.2f hPa Altitude = %0.2f m", cTemp, pressure1, altitude);
            printf(" |Temp=%0.2f|Pressure=%0.2f|Altitude=%0.2f", cTemp, pressure1, altitude);
        }
        else
        {
            printf("Temperature = %0.2f F Pressure = %0.2f hPa Altitude = %0.2f m", fTemp, pressure1, altitude);
            printf(" |Temp=%0.2f|Pressure=%0.2f|Altitude=%0.2f", fTemp, pressure1, altitude);
        }
        
        if((critical_level_t_high < cTemp)||(critical_level_t_low > cTemp))
        {
            exit(2);
        }
        
        if((warning_level_t_high < cTemp)||(warning_level_t_low > cTemp))
        {
            exit(1);
        }
        
        if((critical_level_p_high < pressure1)||(critical_level_p_low > pressure1))
        {
            exit(2);
        }
        
        if((warning_level_p_high < pressure1)||(warning_level_p_low > pressure1))
        {
            exit(1);
        }
        
    exit (0);
}
