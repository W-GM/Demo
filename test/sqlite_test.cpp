#include "sqlite_helper.h"

#define CREATE_TABLE

int main()
{
    sqlControl sqlCtrl;
    char path[] = "./wellsiteRTUData.db";
    char basicinfotable[] = "basicinfo";
    char basicinfoField[] =
        "_saveTime text,_oilName text,_oilTypeH int,_oilTypeL int,_oilExtName text,_oilVendorInfo int,\
    _faultFlag int,_faultPositionCode int,_faultTime1 int,_faultTime2 int,_faultTime3 int,_accuRunTimeH int,\
    _accuRunTimeL int,_pipePressure int,_reStStCtl int,_oilWellStStStatus int,_APhaseCurrent int,_BPhaseCurrent int,\
    _CPhaseCurrent int,_APhaseVoltage int,_BPhaseVoltage int ,_CPhaseVoltage int,_totalActivePower int,_totalReactivePower int,\
    _totalApparentPower int,_runFreq int,_lossPhaseStatus int,_powerFactor int,_ctlObtaentTemp int,_OPMFreqCtlMode int,\
    _OPMFreqCtlStatus int,_OPMManFreqValue int,_OPMEqualCtlMode int,_OPMEqualCtlStatus int,_OPMManEqualAdjustControl int,\
    _OPMBalance int,_OPMCtlMode int";

    char WellSiteTable[] = "wellsite";
    char wellSiteField[]="_saveTime text,_wellSiteInfo text,_rtuVendorInfo text,_rtuDeviceMode text,_wellPipePress int,\
    _injectWaterCountPress int,_wellSiteFaultInfo text, _wellSiteremoteAlarm text,_1_injectWaterSetRead int,\
    _1_injectWaterSetValue int,_1_partWaterPress int,_1_partWaterInsFlow  int,_1_partWaterCntFlowH int,_1_partWaterCntFlowL  int,\
    _2_injectWaterSetRead int,_2_injectWaterSetValue int,_2_partWaterPress int,_2_partWaterInsFlow  int,_2_partWaterCntFlowH int,\
    _2_partWaterCntFlowL  int";

    char engerPowerGraphTable[]="engerpowergraph";
    char engerPowerGraphField[]="_saveTime text,_stroke int,_jigFrequency int,_dotCount int,_maxLoad int,_minLoad int,\
    _maxDistance int,_minDistance int,_PwrEngGraphid int,_minCurrent int,_maxCurrent int ,_minActivePower int,\
    _maxActivePower int,_upStrokeMaxCurrent int,_downStrokeMaxCurrent int,_upStrokeMaxActivePower int,_downStrokeMaxActivePower int";

    char engerPowerDataTable[]="engerpowerdata";
    char engerPowerDataField[]="id int, register int ,energydata int,powerdata int,atime text";
    // 打开数据库
    if (sqlCtrl.sqlOpen(path) == -1)
    {
        return -1;
    }
    printf("open ok\n");

#ifdef CREATE_TABLE

    // 创建表
    if (sqlCtrl.sqlCreateTable(basicinfotable, basicinfoField) == -1)
    {
        return -1;
    }
     // 创建表
    if (sqlCtrl.sqlCreateTable(WellSiteTable, wellSiteField) == -1)
    {
        return -1;
    }

     // 创建表
    if (sqlCtrl.sqlCreateTable(engerPowerGraphTable, engerPowerGraphField) == -1)
    {
        return -1;
    }

    if (sqlCtrl.sqlCreateTable(engerPowerDataTable, engerPowerDataField) == -1)
    {
        return -1;
    }
    printf("create table ok\n");
#endif // ifdef CREATE_TABLE
#ifdef DELETE_TABLE

    // 删除表
    if (sqlCtrl.sqlDeleteTable(tableName) == -1)
    {
        return -1;
    }
    printf("delete table ok\n");
#endif // ifdef DELETE_TABLE
#ifdef UPDATE_VALUE

    // 修改表中的数据
    if (sqlCtrl.sqlUpdateData(tableName, "test=3", NULL) == -1)
    {
        return -1;
    }
    printf("update data ok\n");
#endif // ifdef UPDATE_VALUE
#ifdef INSERT_DATA

    // 向表中添加一行数据
    if (sqlCtrl.sqlInsertData(tableName, "3,1,2,4") == -1)
    {
        return -1;
    }
    printf("insert data ok\n");
#endif // ifdef INSERT_DATA
#ifdef DELETE_DATA

    // 删除表中符合选项的一行(多行)数据/表中的所有数据
    if (sqlCtrl.sqlDeleteData(tableName, "name=2") == -1)
    {
        return -1;
    }
    printf("delete data ok\n");
#endif // ifdef DELETE_DATA
#ifdef INSERT_FIELD

    // 添加字段
    if (sqlCtrl.sqlInsertField(tableName, "ooo", "TEXT", "1") == -1)
    {
        return -1;
    }
    printf("insert field ok\n");
#endif // ifdef INSERT_FIELD
#ifdef DELETE_FIELD

    // 删除字段
    if (sqlCtrl.sqlDeleteField(tableName, "other") == -1)
    {
        return -1;
    }
    printf("delete field ok\n");
#endif // ifdef DELETE_FIELD
#ifdef SELECT_DATA

    // 查询数据
    if (sqlCtrl.sqlSelectData(tableName, "*", "saveTime GLOB '2020*'") == -1)
    {
        return -1;
    }
    printf("select data ok\n");

    // 打印数据
    sqlCtrl.printfData();
#endif // ifdef SELECT_DATA

    // 关闭数据库
    sqlCtrl.sqlClose();
    return 0;
}
