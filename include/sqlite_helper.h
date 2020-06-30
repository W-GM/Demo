#ifndef __SQLITE_HELPER_H__
#define __SQLITE_HELPER_H__
#include <iostream>
#include "library/sqlite3.h"
#include <vector>
#include <sstream>
#include <stdlib.h>
using namespace std;

class sqlControl {
    char *_errmsg = NULL;
    sqlite3 *_db = NULL;

    string _err;                     // 出错信息

    static int _rowSign;             // 行数标志位
    static int _row;                 // callback返回的数据行数
    static int _col;                 // callback返回的数据列数

    static vector<string>_recvData;  // 存储选择的数据
    static vector<string>_parameter; // 存储参数（包含字段名）

public:

    int           getRowSign();
    int           getRow();
    int           getCol();
    vector<string>getRecvData();
    vector<string>getParameter();
    void          setRowSign(int rowSign);
    void          setCol(int col);
    void          setRecvData(vector<string>recvData);
    void          setParameter(vector<string>parameter);

    /* 高速，写同步 */
    int writeSync();

    /**
     * @brief sqlite3_exec的回调函数，在选择数据时使用
     *
     * @param arg 由sqlite3_exec第四个参数传递
     * @param col 列数
     * @param value 接收到的数据
     * @param field 字段名
     * @return int
     */
    static int    callback(void  *arg,
                           int    col,
                           char **value,
                           char **field);

    /**
     * @brief 打印选择的数据
     *
     */
    void printfData();

    /**
     * @brief 打开数据库
     *
     * @param path 数据库名及路径
     * @return int 成功：0，失败：-1
     */
    int  sqlOpen(const char *path);

    /**
     * @brief 通用的操作sqlite的函数（不含查询功能），查询功能见：
     *
     * @param command 具体要操作的命令语句
     * @return int 成功：0，失败：-1
     */
    int  sqlcurrent(string command);

    /**
     * @brief 创建表
     *
     * @param tableName 表名
     * @param field 标签语句（格式：<field> <type>,<field> <type>,...）
     * @return int 成功：0，失败：-1
     */
    int  sqlCreateTable(char *tableName,
                        char *field);

    /**
     * @brief 删除表
     *
     * @param tableName 表名
     * @return int 成功：0，失败：-1
     */
    int sqlDeleteTable(char *tableName);

    /**
     * @brief 修改表中的数据
     *
     * @param tableName 表名
     * @param updateValue 要修改/更新的值(格式：<field>=<value>)
     * @param updateOption 要修改/更新的选项(格式：同上)，若为NULL则将field整列设置为value
     * @return int 成功：0，失败：-1
     */
    int sqlUpdateData(char *tableName,
                      char *updateValue,
                      char *updateOption);

    /**
     * @brief 向表中添加一行数据
     *
     * @param tableName 表名
     * @param value 数据行语句
     * @return int 成功：0，失败：-1
     */
    int sqlInsertData(char       *tableName,
                      const char *value);

    /**
     * @brief 删除表中的一行数据/表中的所有数据
     *
     * @param tableName 表名
     * @param deleteOption 删除选项语句（格式：<field>=<value>），若为NULL则删除表中所有数据
     * @return int 成功：0，失败：-1
     */
    int sqlDeleteData(char *tableName,
                      const char *deleteOption);

    /**
     * @brief 添加字段
     *
     * @param tableName 表名
     * @param fieldName 字段名
     * @param fieldType 字段类型
     * @param defaultValue 默认值，若为NULL则该字段默认为0
     * @return int 成功：0，失败：-1
     */
    int sqlInsertField(char *tableName,
                       char *fieldName,
                       char *fieldType,
                       char *defaultValue);

    /**
     * @brief 删除字段
     *
     * @param tableName 表名
     * @param fieldName 字段名
     * @return int 成功：0，失败：-1
     */
    int sqlDeleteField(char *tableName,
                       char *fieldName);

    /**
     * @brief 查询数据
     *
     * @param tableName 表名
     * @param selectOption 查询字段选项（格式："*" / <field>,<field>,...）
     * @param selectCondition 查询条件语句
     * @return int 成功：0，失败：-1
     */
    int sqlSelectData(char *tableName,
                      char *selectOption,
                      char *selectCondition);

    /**
     * @brief 重命名表名
     *
     * @param tableName 原表名
     * @param newTableName 新表名
     * @return int 成功：0，失败：-1
     */
    int sqlRenameTable(const char *tableName,
                       char       *newTableName);

    /**
     * @brief 关闭数据库
     *
     */
    void sqlClose();
};

#endif // ifndef __SQLITE_CTL_H__
