#pragma once

#include "table.h"
struct InputBUffer;

enum EMetaCommandResult
{
    META_COMMAND_SUCCESS,
    META_COMMAND_EXIT,
    META_COMMAND_UNRECOGNIZED_COMMAND,
};

EMetaCommandResult do_meta_command(Table *table, InputBUffer *buffer);
