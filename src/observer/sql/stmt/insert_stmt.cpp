/* Copyright (c) 2021OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/insert_stmt.h"
#include "common/log/log.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include <iostream>
InsertStmt::InsertStmt(Table *table, const Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

RC InsertStmt::create(Db *db, const InsertSqlNode &inserts, Stmt *&stmt)
{
  const char *table_name = inserts.relation_name.c_str();
  if (nullptr == db || nullptr == table_name || inserts.values.empty()) { // 检查数据库名，关系名，插入的值是否为空
    LOG_WARN("invalid argument. db=%p, table_name=%p, value_num=%d",
        db, table_name, static_cast<int>(inserts.values.size()));
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name); // 检查当前数据库中是否存在当前关系表
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // check the fields number 
  const Value     *values     = inserts.values.data(); 
  const int        value_num  = static_cast<int>(inserts.values.size()); 
  const TableMeta &table_meta = table->table_meta();
  const int        field_num  = table_meta.field_num() - table_meta.sys_field_num();
  if (field_num != value_num) { // 检查当前关系表中的字段数量是否与插入值一致
    LOG_WARN("schema mismatch. value num=%d, field num in schema=%d", value_num, field_num);
    return RC::SCHEMA_FIELD_MISSING;
  }

  // check fields type
  const int sys_field_num = table_meta.sys_field_num();
  for (int i = 0; i < value_num; i++) { // 检查每一个插入的值，其类型是否与对应的字段的类型一致
    const FieldMeta *field_meta = table_meta.field(i + sys_field_num);
    const AttrType   field_type = field_meta->type();
    const AttrType   value_type = values[i].attr_type();
    if (field_type != value_type) {  // TODO try to convert the value type to field type
      LOG_WARN("field type mismatch. table=%s, field=%s, field type=%d, value_type=%d",
          table_name, field_meta->name(), field_type, value_type);
      return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
    else if(field_type == DATES){
      //Reference: https://github.com/CentaureaHO/miniob/commit/aa79cab73f851555116cd1a533f1cb7f5cfd2fb3#diff-253ca6b9e9f18d82a8a0bc35d12e570b32432b8ee5415d32ad2bb27d7d56224d
 
      if (!CheckDate(values[i].get_date())) 
        return RC::SCHEMA_FIELD_TYPE_MISMATCH;
    }
  }

  // everything alright
  stmt = new InsertStmt(table, values, value_num); // 以上全部合法，创建 InsertStmt
  return RC::SUCCESS;
}
