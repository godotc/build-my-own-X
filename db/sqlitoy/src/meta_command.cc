#include "meta_command.h"
#include "input_buffer.h"
#include "table.h"
#include "test_table.h"
#include <cstdio>
#include <cstdlib>
#include <string_view>

EMetaCommandResult do_meta_command(Table *table, InputBUffer *input_buffer)
{

    // exit
    if (std::string_view(".exit") == input_buffer->buffer) {
        return META_COMMAND_EXIT;
    }
    else if (std::string_view(".btree") == input_buffer->buffer) {
        printf("Tree:\n");
        testtable::print_leaf_node(table->get_or_allocate_page(0));
        return META_COMMAND_SUCCESS;
    }
    else if (std::string_view(".constants") == input_buffer->buffer) {
        printf("Constants:\n");
        testtable::print_constants();
        return META_COMMAND_SUCCESS;
    }
    else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}
