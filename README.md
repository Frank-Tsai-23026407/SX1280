# SX1280 V1.0
Auther: Yueh-Feng Tsai  
Email: frank23026407@gmail.com  
Date: 2023/11/25

## Files
Inc  
|- SX128x.hpp  
|- SX128x_OBJ.hpp  
  
Src  
|- SX128x.cpp  
|- SX128x_driver.cpp  
|- SX128x_OBJ.cpp  

## Examples
**Naming Rule:** <span style="color:gray">[Code Type]</span>_<span style="color:gray">[Board Name]</span>  
**Code Type:** PLN(plain), OBJ(object)  
**Board Name:** SPHV2(sophia v2), F446(Nucleo-F446RE), L476(Nucleo-L476RE)

## TX only
1. Set up floor plan (checking spi frequency, spi bit rate, spi packet width, timer 1 frequency, gpio push up or down, gpio name, gpio speed)
EX: floor plane for L476 or F446
![image](pictures/L476-F446%20floor%20plane.png)
2. Change the board 
3. Declare `sx1280`
4. `sx1280.init()`
5. `sx1280.TxSetting`
6. `sx1280.PutPacket`

## RX only
refered to the TX only and "example/OBJ_F446" project