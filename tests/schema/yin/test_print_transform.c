/**
 * \file test_print_transform.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - transforming node-ids schema -> JSON -> schema and printing in both YANG and YIN
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "../../../src/libyang.h"
#include "../../config.h"

#define SCHEMA_FOLDER TESTS_DIR"/schema/yin/files"

static int
diff(const char *data, FILE *model)
{
    char *data2;
    unsigned int length;
    size_t read;

    length = strlen(data);

    data2 = malloc(length);

    read = fread(data2, 1, length, model);
    if (read != length) {
        goto fail;
    }

    if (strncmp(data, data2, length)) {
        goto fail;
    }

    free(data2);
    return 0;

fail:
    free(data2);
    fprintf(stderr, "diff failed on:\n\"%s\"\n", data);
    return 1;
}

static int
setup_ctx(void **state)
{
    //ly_verb(LY_LLVRB);
    (*state) = ly_ctx_new(SCHEMA_FOLDER);
    if (!(*state)) {
        return -1;
    }

    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy((struct ly_ctx *)(*state), NULL);
    (*state) = NULL;

    return 0;
}

static void
test_modules(void **state)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *module;
    char *new;
    FILE *file;
    int ret;

    module = ly_ctx_load_module(ctx, "d2", NULL);
    assert_non_null(module);

    /* YANG */
    ret = lys_print_mem(&new, module, LYS_OUT_YANG, NULL);
    assert_int_equal(ret, 0);

    file = fopen(SCHEMA_FOLDER"/d2_output.yang", "r");
    assert_non_null(file);

    ret = diff(new, file);
    free(new);
    fclose(file);
    assert_int_equal(ret, 0);

    /* YIN */
    ret = lys_print_mem(&new, module, LYS_OUT_YIN, NULL);
    assert_int_equal(ret, 0);

    file = fopen(SCHEMA_FOLDER"/d2_output.yin", "r");
    assert_non_null(file);

    ret = diff(new, file);
    free(new);
    fclose(file);
    assert_int_equal(ret, 0);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
