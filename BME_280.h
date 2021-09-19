#include <stdint.h>
#include <stm32f1xx_hal_conf.h>
#include <stm32f1xx_it.h>
#include <string.h>

/*      PINOUT:
 *      VCC -> 3.3V
 *      GND -> GND
 *      SCL -> B6
 *      SDA -> B7
 *      CSB -> 3.3V     Povezivanjem na 3.3V omogucavamo I2C protokol, u slucaju da je pulled down -> SPI
 *      SDO -> GND      Povezivanjem na GND Slave adresa postaje 0x76, u slucaju povezivanja na 3.3V -> 0x77
 */

I2C_HandleTypeDef hi2c1;
uint8_t SensorAddressWrite = 0x76 << 1 | 0x00;

//Vrednosti za trimovanje T
unsigned short digt1;
signed short digt2;
signed short digt3;

//Vrednosti za trimovanje P
unsigned short digp1;
signed short digp2;
signed short digp3;
signed short digp4;
signed short digp5;
signed short digp6;
signed short digp7;
signed short digp8;
signed short digp9;

void initialisation(uint8_t T_oversampling, uint8_t P_oversampling, char operationalMode, uint8_t IIRfilter, uint8_t SPIEnable)
{
    /*      Initialisation funkcija služi podešavanju parametra senzora:
     *      - T oversampling (vrednosti moraju biti 2^n, gde n može biti 0 - 4)
     *      - P oversampling (vrednosti moraju biti 2^n, gde n može biti 0 - 4)
     *      - Operational mode ('n'- normal mode (ciklično merenje), 'f' - forced mode (jedno merenje potom sleep), 's' - sleep mode (nema merenja))
     *      - Koeficijent IIRfilter-a (vrednosti mogu biti: [0,2,4,8,16])
     *      - SPI enable (Prelaz na SPI interfejs u slučaju setovanja bita na 1)
     */

        uint8_t ctrlMeas7_5, ctrMeas4_2, ctrlMeas1_0, ctrlMeas;

        switch(T_oversampling)
        {
            case 0: ctrlMeas7_5 = 0; break;         // Oversampling disabled
            case 1: ctrlMeas7_5 = 32; break;        // x1 oversampling
            case 2: ctrlMeas7_5 = 64; break;        // x2 oversampling
            case 4: ctrlMeas7_5 = 96; break;        // x4 oversampling
            case 8: ctrlMeas7_5 = 128; break;       // x8 oversampling
            case 16: ctrlMeas7_5 = 160; break;      // x16 oversampling
                default: ctrlMeas7_5 = 160;         // x16 oversampling
        }

        switch(P_oversampling)
        {
            case 0: ctrMeas4_2 = 0; break;          // Oversampling disabled
            case 1: ctrMeas4_2 = 4; break;          // x1 oversampling
            case 2: ctrMeas4_2 = 8; break;          // x2 oversampling
            case 4: ctrMeas4_2 = 12; break;         // x4 oversampling
            case 8: ctrMeas4_2 = 16; break;         // x8 oversampling
            case 16: ctrMeas4_2 = 20; break;        // x16 oversampling
                default: ctrMeas4_2 = 20;           // x16 oversampling
        }

        switch(operationalMode)
        {
            case 'n': ctrlMeas1_0 = 3; break;       // Normal mode
            case 'f': ctrlMeas1_0 = 1; break;       // Forced mode
            case 's': ctrlMeas1_0 = 0; break;       // Sleep mode
            default: ctrlMeas1_0 = 3;               //Normal mode
        }

        ctrlMeas = ctrlMeas7_5 | ctrMeas4_2 | ctrlMeas1_0;      // Kombinovanje pređašnjih vrednosti u vrednost koju je potrebno zapisati u registru

        uint8_t ctrlMeasSet[2] = {0xF4, ctrlMeas};              // 0xF4 - adresa registra, ctrlMeas - vrednost koju želimo upisati
        HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, ctrlMeasSet, 2, HAL_MAX_DELAY);     // Upis u registar

        uint8_t config4_2, config0, config;
        switch(IIRfilter)
        {
            case 0: config4_2 = 0; break;       // Filter off
            case 2: config4_2 = 4; break;       // Koef = 2
            case 4: config4_2 = 8; break;       // Koef = 4
            case 8: config4_2 = 12; break;     // Koef = 8
            case 16: config4_2 = 16; break;    // Koef = 16
                default: config4_2 = 16;       // Koef = 16
        }

        switch(SPIEnable)
        {
            case 1: config0 = 1; break;         // SPI interface on
                default: config0 = 0;           // SPI interface off
        }

        config = config4_2 | config0;       // Kombinovanje pređašnjih vrednosti u vrednost koju je potrebno zapisati u registru

        uint8_t configSet[2] = {0xF5, config};      // 0xF5 - adresa registra, config - vrednost koju želimo upisati
        HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, configSet, 2, HAL_MAX_DELAY);       // Upis u registar
}

void write1(unsigned short *dig, uint8_t dig_Address[])      // Funkcija za čitanje unsigned short vrednosti za trimovanje
{
    uint8_t niz[5], dataBuffer[5];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &dig_Address[0], 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite  , dataBuffer, 1, HAL_MAX_DELAY);
    niz[0] = dataBuffer[0];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &dig_Address[1], 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite  , dataBuffer, 1, HAL_MAX_DELAY);  
    niz[1] = dataBuffer[0];
    memcpy(dig, &niz, sizeof(*dig));
}

void write2(signed short *dig, uint8_t dig_Address[])      // Funkcija za čitanje signed short vrednosti za trimovanje
{
    uint8_t niz[5], dataBuffer[5];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &dig_Address[0], 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite  , dataBuffer, 1, HAL_MAX_DELAY);
    niz[0] = dataBuffer[0];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &dig_Address[1], 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite  , dataBuffer, 1, HAL_MAX_DELAY);  
    niz[1] = dataBuffer[0];
    memcpy(dig, &niz, sizeof(*dig));
}

void trimming(void)
{
    /*      Trimming funkcija vraća vrednosti registra za trimovanje podataka
     *      digp - vrednosti za trimmovanje Raw vrednosti pritiska
     *      digt - vrednosti za trimovanje Raw vrednosti temperature
     */

    uint8_t dig_T1_Address[2] = {0x88 , 0x89};
    uint8_t dig_T2_Address[2] = {0x8A , 0x8B};
    uint8_t dig_T3_Address[2] = {0x8C , 0x8D};

    uint8_t dig_P1_Address[2] = {0x8E , 0x8F};
    uint8_t dig_P2_Address[2] = {0x90 , 0x91};
    uint8_t dig_P3_Address[2] = {0x92 , 0x93};
    uint8_t dig_P4_Address[2] = {0x94 , 0x95};
    uint8_t dig_P5_Address[2] = {0x96 , 0x97};
    uint8_t dig_P6_Address[2] = {0x98 , 0x99};
    uint8_t dig_P7_Address[2] = {0x9A , 0x9B};
    uint8_t dig_P8_Address[2] = {0x9C , 0x9D};
    uint8_t dig_P9_Address[2] = {0x9E , 0x9F};

    write1(&digp1, dig_P1_Address);
    write2(&digp2, dig_P2_Address);
    write2(&digp3, dig_P3_Address);
    write2(&digp4, dig_P4_Address);
    write2(&digp5, dig_P5_Address);
    write2(&digp6, dig_P6_Address);
    write2(&digp7, dig_P7_Address);
    write2(&digp8, dig_P8_Address);
    write2(&digp9, dig_P9_Address);
    write1(&digt1, dig_T1_Address);
    write2(&digt2, dig_T2_Address);
    write2(&digt3, dig_T3_Address);
}

void readValue(float *Tconv, float *Pconv)
{
    /*      readValue funkcija cita Raw vrednosti pritiska i temperature
     *      potom ih trimmuje i vraca kao konvertovne vrednosti Temperature[C] i Pritiska[Pa]
     */
    
    long signed int adc_T = 0;
    long signed int adc_P = 0;
    signed long T;
    unsigned long int P;
    uint8_t dataBuffer[5];
    uint8_t niz[5];

    //Adrese registra raw vrednosti P i T
    uint8_t P1 = 0xF7;
    uint8_t P2 = 0xF8;
    uint8_t P3 = 0xF9;
    uint8_t T1 = 0xFA;
    uint8_t T2 = 0xFB;
    uint8_t T3 = 0xFC;

    //Citanje raw temperature
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &T3, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite , dataBuffer, 1, HAL_MAX_DELAY);
    niz[0] = dataBuffer[0];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &T2, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite , dataBuffer, 1, HAL_MAX_DELAY);
    niz[1] = dataBuffer[0];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &T1, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite  , dataBuffer, 1, HAL_MAX_DELAY);
    niz[2] = dataBuffer[0]; 

    //Racunanje stvarne temperature, izrazene sa preciznoscu na 2 decimale, vrednost 2711 predstavlja temperaturu od 27,11 stepena
    adc_T = 0 | ((long signed int)niz[0] >> 4 | (long signed int)niz[1] << 4 | (long signed int)niz[2] << 12);
    signed long var1, var2,t_fine;
    var1 = ((((adc_T>>3) - ((signed long)digt1<<1))) *((signed long)digt2)) >> 11;
    var2 = (((((adc_T>>4) - ((signed long)digt1)) * ((adc_T>>4) - ((signed long)digt1))) >> 12) * ((signed long)digt3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    *Tconv = T / 100.0;

    //Citanje raw pritiska
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &P3, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite , dataBuffer, 1, HAL_MAX_DELAY);
    niz[0] = dataBuffer[0];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &P2, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite , dataBuffer, 1, HAL_MAX_DELAY);
    niz[1] = dataBuffer[0];
    HAL_I2C_Master_Transmit(&hi2c1, SensorAddressWrite, &P1, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, SensorAddressWrite  , dataBuffer, 1, HAL_MAX_DELAY);
    niz[2] = dataBuffer[0]; 

    //Racunanje stvarnog pritiska, izrazen u Q24.8 formatu -> dobijenu vrednost potrebno podeliti sa 256, time dobijamo PConv
    adc_P = 0 | ((long signed int)niz[0] >> 4 | (long signed int)niz[1] << 4 | (long signed int)niz[2] << 12);

    signed long long int var1p, var2p, p;
    var1p = ((signed long long int)t_fine) - 128000;
    var2p = var1p * var1p * (signed long long int)digp6;
    var2p = var2p + ((var1p*(signed long long int)digp5)<<17);
    var2p = var2p + (((signed long long int)digp4)<<35);
    var1p = ((var1p * var1p * (signed long long int)digp3)>>8) + ((var1p * (signed long long int)digp2)<<12);
    var1p = (((((signed long long int)1)<<47)+var1p))*((signed long long int)digp1)>>33;
    if(var1p == 0)
    {
    //kako ne bi doslo do slucaja deljenjem sa nulom
    }
    else
    {
      p = 1048576 - adc_P;
      p = (((p<<31)-var2p)*3125)/var1p;
      var1p = (((signed long long int)digp9) * (p>>13) * (p>>13)) >> 25;
      var2p = (((signed long long int)digp8) * p) >> 19;
      p = ((p + var1p + var2p) >> 8) + (((signed long long int)digp7)<<4);
    }
    P = (unsigned long int)p;
    *Pconv = P/256.0;
    }