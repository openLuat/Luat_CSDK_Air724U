
#include <string.h>

#include "lplatform.h"
#include "platform_conf.h"
#include "am_openat.h"

static const E_AMOPENAT_I2C_PORT i2cIdMap[OPENAT_I2C_QTY] =
{
    OPENAT_I2C_1, //id = 0
    OPENAT_I2C_2, //id = 1
    OPENAT_I2C_3,
/*+\BUG3555\zhuwangbin\2020.11.11\修改I2C1和I2C3不能用的问题,id 1,2,3对应I2C1,2,3*/
	OPENAT_I2C_QTY
/*-\BUG3555\zhuwangbin\2020.11.11\修改I2C1和I2C3不能用的问题,id 1,2,3对应I2C1,2,3*/
};

static u16 i2cSlaveAddr[OPENAT_I2C_QTY];

static u16 gpioi2cSlaveAddr[OPENAT_I2C_QTY];

/*-\NEW\zhuwangbin\2016.4.6\兼容不同版本的g_sensor, 添加获取设备id和设备地址接口*/
/*
A13	00111101b	01000100b（SLI3108）
A14/A15	00111011b	01000100b（SLI3108）
A14/A15	00111011b	00111101b（PA22401001)
*/


#define PLATFORM_G_SENSOR_SLI3108_ID 0X21
#define PLATFORM_G_SENSOR_SLI3108_REG 0X00
#define PLATFORM_G_SENSOR_LIS2DS12_ID 0X43
#define PLATFORM_G_SENSOR_LIS2DS12_ID_REG 0X0F
#define PLATFORM_A13_SLAVE_ADDR 0X1E
#define PLATFORM_A14_SLAVE_ADDR 0X1D
#define PLATFORM_A15_SLAVE_ADDR PLATFORM_A14_SLAVE_ADDR

void platform_i2c_gSensorParam_get(u8 id, u8 *slave_addr, u8 *slave_id)
{
  u8 pRegAddr;

  /*版本A13  型号LIS2DS12*/
  pRegAddr = PLATFORM_G_SENSOR_LIS2DS12_ID_REG;
  *slave_addr = PLATFORM_A13_SLAVE_ADDR;
  IVTBL(read_i2c)(i2cIdMap[id], *slave_addr, &pRegAddr, slave_id, 1);

  if (*slave_id == PLATFORM_G_SENSOR_LIS2DS12_ID)
  {
    OPENAT_print("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
    return;
  }
  
  /*版本A14  型号LIS2DS12*/
  pRegAddr = PLATFORM_G_SENSOR_LIS2DS12_ID_REG;
  *slave_addr = PLATFORM_A14_SLAVE_ADDR;
  IVTBL(read_i2c)(i2cIdMap[id], *slave_addr, &pRegAddr, slave_id, 1);

  if (*slave_id == PLATFORM_G_SENSOR_LIS2DS12_ID)
  {
    OPENAT_print("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
    return;
  }

  /*版本A15 型号SLI3108*/
  
  pRegAddr = PLATFORM_G_SENSOR_SLI3108_REG;
  *slave_addr = PLATFORM_A15_SLAVE_ADDR;
  IVTBL(read_i2c)(i2cIdMap[id], *slave_addr, &pRegAddr, slave_id, 1);

  if (*slave_id == PLATFORM_G_SENSOR_SLI3108_ID)
  {
    OPENAT_print("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
    return;
  }

  /*版本A15 型号其他*/
  *slave_id = 0;
  *slave_addr = PLATFORM_A15_SLAVE_ADDR;
    
  OPENAT_print("platform_i2c_gSensorParam_get %x, %x", *slave_id, *slave_addr);
   
  return;
}
/*-\NEW\zhuwangbin\2016.4.6\兼容不同版本的g_sensor, 添加获取设备id和设备地址接口*/

int platform_i2c_exists( unsigned id ) 
{
/*+\BUG3555\zhuwangbin\2020.11.11\修改I2C1和I2C3不能用的问题,id 1,2,3对应I2C1,2,3*/
    if((id > OPENAT_I2C_QTY) || id == 0) // 仅支持I2C 2
        return PLATFORM_ERR;
/*-\BUG3555\zhuwangbin\2020.11.11\修改I2C1和I2C3不能用的问题,id 1,2,3对应I2C1,2,3*/
    return PLATFORM_OK;
}

int platform_i2c_setup( unsigned id, PlatformI2CParam *pParam ) 
{
    T_AMOPENAT_I2C_PARAM openatI2CParam;
    BOOL ret;

    memset(&openatI2CParam, 0, sizeof(openatI2CParam));

    openatI2CParam.freq = pParam->speed;
    i2cSlaveAddr[id] = pParam->slaveAddr;
    
    ret = IVTBL(open_i2c)(i2cIdMap[id], &openatI2CParam);
    
    return ret ? pParam->speed : 0;
}
/*+\NEW\WANGJIAN\2019.4.10\封装i2c.close接口*/
int platform_i2c_close( unsigned id )
{
    return IVTBL(close_i2c)(i2cIdMap[id]);
}
/*-\NEW\WANGJIAN\2019.4.10\封装i2c.close接口*/

int platform_i2c_send_data( unsigned id, u16 slave_addr, const u8 *pRegAddr, const u8 *buf, u32 len )
{
    // 如果传输的从地址为空 则使用预设的从地址
    if(slave_addr == I2C_NULL_SLAVE_ADDR) {
        slave_addr = i2cSlaveAddr[id];
    }

    // 如果从地址为空,则返回传输失败
    if(slave_addr == I2C_NULL_SLAVE_ADDR){
        return 0;
    }
    
    return IVTBL(write_i2c)(i2cIdMap[id], slave_addr, pRegAddr, buf, len);
}

int platform_i2c_recv_data( unsigned id, u16 slave_addr, const u8 *pRegAddr, u8 *buf, u32 len  )
{
    // 如果传输的从地址为空 则使用预设的从地址
    if(slave_addr == I2C_NULL_SLAVE_ADDR) {
        slave_addr = i2cSlaveAddr[id];
    }

    // 如果从地址为空,则返回传输失败
    if(slave_addr == I2C_NULL_SLAVE_ADDR){
        return 0;
    }
    
    return IVTBL(read_i2c)(i2cIdMap[id], slave_addr, pRegAddr, buf, len);
}


#ifdef LUA_GPIO_I2C
int platform_gpio_i2c_send_data( unsigned id, u16 slave_addr, const u8 *pRegAddr, const u8 *buf, u32 len )
{
    // 如果传输的从地址为空 则使用预设的从地址
    if(slave_addr == I2C_NULL_SLAVE_ADDR) {
        slave_addr = gpioi2cSlaveAddr[id];
    }

    // 如果从地址为空,则返回传输失败
    if(slave_addr == I2C_NULL_SLAVE_ADDR){
        return 0;
    }
    
    return IVTBL(write_gpio_i2c)(i2cIdMap[id], slave_addr, pRegAddr, buf, len);
}

int platform_gpio_i2c_recv_data( unsigned id, u16 slave_addr, const u8 *pRegAddr, u8 *buf, u32 len  )
{
    // 如果传输的从地址为空 则使用预设的从地址
    if(slave_addr == I2C_NULL_SLAVE_ADDR) {
        slave_addr = gpioi2cSlaveAddr[id];
    }

    // 如果从地址为空,则返回传输失败
    if(slave_addr == I2C_NULL_SLAVE_ADDR){
        return 0;
    }
    
    return IVTBL(read_gpio_i2c)(i2cIdMap[id], slave_addr, pRegAddr, buf, len);
}


int platform_gpio_i2c_setup( unsigned id, T_AMOPENAT_GPIO_I2C_PARAM *pParam ) 
{
    BOOL ret;
    gpioi2cSlaveAddr[id] = pParam->slaveAddr;
    
    ret = IVTBL(open_gpio_i2c)(i2cIdMap[id], pParam->sda_port, pParam->scl_port);
    
    return ret ;
}
#endif

