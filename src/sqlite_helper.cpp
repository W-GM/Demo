#include "sqlite_helper.h"

int sqlControl::_rowSign = 0;          // 行数标志位
int sqlControl::_row = 0;              // callback返回的数据行数
int sqlControl::_col = 0;              // callback返回的数据列数

vector<string> sqlControl::_recvData;  // 存储选择的数据
vector<string> sqlControl::_parameter; // 存储参数（包含字段名）

int sqlControl::getRowSign()
{
    return _rowSign;
}

int sqlControl::getRow()
{
    return _row;
}

int sqlControl::getCol()
{
    return _col;
}

vector<string>sqlControl::getRecvData()
{
    return _recvData;
}

vector<string>sqlControl::getParameter()
{
    return _parameter;
}

void sqlControl::setRowSign(int rowSign)
{
    _rowSign = rowSign;
}

void sqlControl::setCol(int col)
{
    _col = col;
}

void sqlControl::setRecvData(vector<string>recvData)
{
    _recvData = recvData;
}

void sqlControl::setParameter(vector<string>parameter)
{
    _parameter = parameter;
}

/* 高速，写同步 */
int sqlControl::writeSync()
{
    string errmsg;
    if (sqlite3_exec(_db, "PRAGMA synchronous = OFF; ", 0, 0, 0))
    {
        errmsg = _errmsg;
        _err = "open write sync faild: " + errmsg;
        cout << _err << endl;
        return -1;
    }
    return 0;
}

/**
 * @brief sqlite3_exec的回调函数，在选择数据时使用
 *
 * @param arg 由sqlite3_exec第四个参数传递
 * @param col 列数
 * @param value 接收到的数据
 * @param field 字段名
 * @return int
 */
int sqlControl::callback(void *arg, int col, char **value, char **field)
{
    string fie;
    string val;

    _rowSign++;

    if (_rowSign == 1)
    {
        _col = col;
    }

    for (int i = 0; i < col; i++)
    {
        fie = field[i];

        if (value[i] == NULL)
        {
            val = "0";
        }
        else
        {
            val = value[i];
        }

        // 将数据存储在_recvData中
        _recvData.push_back(val);

        if (_rowSign == 1)
        {
            // 将字段存储在_parameter中
            _parameter.push_back(fie);
        }
    }
    return 0;
}

/**
 * @brief 打印选择的数据
 *
 */
void sqlControl::printfData()
{
    for (int i = 0; i < _col; i++)
    {
        printf("%s ", _parameter[i].c_str());
    }
    puts("");

    for (int i = 0; i < _row; i++)
    {
        for (int j = 0; j < _col; j++)
        {
            printf("%s ", _recvData[i * _col + j].c_str());
        }
        puts("");
    }
}

/**
 * @brief 打开数据库
 *
 * @param path 数据库名及路径
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlOpen(const char *path)
{
    if (sqlite3_open(path, &_db) != SQLITE_OK)
    {
        _err = "open faild: " + *sqlite3_errmsg(_db);
        cout << _err << endl;
        return -1;
    }
    return 0;
}

/**
 * @brief 通用的操作sqlite的函数（不含查询功能），查询功能见：sqlSelectTable()
 *
 * @param command 具体要操作的命令语句
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlcurrent(string command)
{
    string errmsg;


    if (sqlite3_exec(_db, command.c_str(), NULL, NULL, &_errmsg) != SQLITE_OK)
    {
        errmsg = _errmsg;
        _err = "create table faild: " + errmsg;
        cout << _err << endl;
        return -1;
    }
    return 0;
}

/**
 * @brief 创建表
 *
 * @param tableName 表名
 * @param field 标签语句（格式：<field> <type>,<field> <type>,...）
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlCreateTable(char *tableName, char *field)
{
    string field_ = field;
    string tableName_ = tableName;
    string commandInsertData_ = "create table " + tableName_ + "(" +
                                field_ + ")";

    if (sqlcurrent(commandInsertData_) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 删除表
 *
 * @param tableName 表名
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlDeleteTable(char *tableName)
{
    string tableName_ = tableName;
    string commandDeleteTable_ = "drop table " + tableName_;

    if (sqlcurrent(commandDeleteTable_) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 修改表中的数据
 *
 * @param tableName 表名
 * @param updateValue 要修改/更新的值
 * @param updateOption 要修改/更新的选项
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlUpdateData(char *tableName,
                              char *updateValue,
                              char *updateOption)
{
    string tableName_ = tableName;
    string updateValue_ = updateValue;
    string updateOption_;
    string commandUpdateData_;

    if (updateOption == NULL)
    {
        //选项updateOption列均设置为updateValue
        commandUpdateData_ = "update " + tableName_ + " set " + updateValue;
    }
    else
    {
        updateOption_ = updateOption;
        commandUpdateData_ = "update " + tableName_ + " set " + updateValue +
                             " where " + updateOption_;
    }

    if (sqlcurrent(commandUpdateData_) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 向表中添加一行数据
 *
 * @param tableName 表名
 * @param value 数据行语句(格式：<value>,<value>,...)
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlInsertData(char *tableName, const char *value)
{
    string tableName_ = tableName;
    string value_ = value;
    string commandInsertData_ = "insert into " + tableName_ + " values(" +
                                value_ + ");";

    if (sqlcurrent(commandInsertData_) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 删除表中符合选项的一行(多行)数据/表中的所有数据
 *
 * @param tableName 表名
 * @param deleteOption 删除选项语句（格式：<field>=<value>），若为NULL则删除表中所有数据
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlDeleteData(char *tableName, const char *deleteOption)
{
    string tableName_ = tableName;
    string deleteOption_;
    string commandDeleteData_;

    if (deleteOption == NULL) // 删除表中所有的数据
    {
        commandDeleteData_ = "delete from " + tableName_;
    }
    else
    {
        deleteOption_ = deleteOption;
        commandDeleteData_ = "delete from " + tableName_ + " where " +
                             deleteOption_;
    }

    if (sqlcurrent(commandDeleteData_) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 添加字段
 *
 * @param tableName 表名
 * @param fieldName 字段名
 * @param fieldType 字段类型
 * @param defaultValue 默认值，若为NULL则该字段默认为0
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlInsertField(char *tableName,
                               char *fieldName,
                               char *fieldType,
                               char *defaultValue)
{
    string tableName_ = tableName;
    string fieldName_ = fieldName;
    string fieldType_ = fieldType;
    string defaultValue_;
    string commandInsertField_;

    if (defaultValue == NULL)
    {
        commandInsertField_ = "alter table " + tableName_ + " add column " +
                              fieldName_ + " " + fieldType_ + " default 0";
    }
    else
    {
        defaultValue_ = defaultValue;
        commandInsertField_ = "alter table " + tableName_ + " add column " +
                              fieldName_ + " " + fieldType_ + " default " +
                              defaultValue_;
    }

    if (sqlcurrent(commandInsertField_) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 删除字段
 *
 * @param tableName 表名
 * @param fieldName 字段名
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlDeleteField(char *tableName, char *fieldName)
{
    string tmp;
    string val;
    string createValue_;
    string tableName_ = tableName;
    string fieldName_ = fieldName;
    string value = "select * from " + tableName_ + " company limit 1";

    // 查询原表中的字段名
    if (sqlite3_exec(_db, value.c_str(), sqlControl::callback, NULL,
                     &_errmsg) != SQLITE_OK)
    {
        _err = "elsect data faild: " + *_errmsg;
        cout << _err << endl;
        return -1;
    }
    else
    {
        // 存储行数，并将行标志位置0
        _row = _rowSign;
        _rowSign = 0;

        // printf("elsect data ok\n");
    }

    for (int i = 0; i < _col; i++)
    {
        tmp = _parameter[i];

        if (tmp != fieldName_)
        {
            val += tmp;
        }
        else
        {
            continue;
        }

        if (i < _col - 2)
        {
            val += ",";
        }
    }

    // 通过原表创建一个不包含字段名fieldName的新表
    createValue_ = "create table " + tableName_ + "Copy" + " as select " + val +
                   " from " + tableName_;

    if (sqlcurrent(createValue_) == -1)
    {
        return -1;
    }

    // 删除原表
    if (sqlDeleteTable(tableName) == -1)
    {
        return -1;
    }

    // 重命名为原表名
    if (sqlRenameTable((tableName_ + "copy").c_str(), tableName) == -1)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief 查询数据
 *
 * @param tableName 表名
 * @param selectOption 查询字段选项（格式："*" / <field>,<field>,...）
 * @param selectCondition 查询条件语句
 * @return int
 */
int sqlControl::sqlSelectData(char *tableName,
                              char *selectOption,
                              char *selectCondition)
{
    string tableName_ = tableName;
    string selectOption_ = selectOption;
    string selectCondition_;
    string commandSelectData_;

    // 判断条件项是否为空
    if (selectCondition == NULL)
    {
        commandSelectData_ = "select " + selectOption_ + " from " + tableName_;
    }
    else
    {
        selectCondition_ = selectCondition;
        commandSelectData_ = "select " + selectOption_ + " from " + tableName_ +
                             " where " + selectCondition_;
    }

    if (sqlite3_exec(_db, commandSelectData_.c_str(), sqlControl::callback, NULL,
                     &_errmsg) != SQLITE_OK)
    {
        _err = "select data faild: " + *_errmsg;
        cout << _err << endl;
        return -1;
    }
    else
    {
        // 存储行数，并将行标志位置0
        _row = _rowSign;
        _rowSign = 0;

        // printf("elsect data ok\n");
    }

    return 0;
}

/**
 * @brief 重命名表名
 *
 * @param tableName 原表名
 * @param newTableName 新表名
 * @return int 成功：0，失败：-1
 */
int sqlControl::sqlRenameTable(const char *tableName, char *newTableName)
{
    string tableName_ = tableName;
    string newTableName_ = newTableName;
    string commandRenameTable_ = "alter table " + tableName_ + " rename to " +
                                 newTableName_;

    if (sqlcurrent(commandRenameTable_) == -1)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 关闭数据库
 *
 */
void sqlControl::sqlClose()
{
    sqlite3_close(_db);
}
