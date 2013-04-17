/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ident "$Id$"
#ident "Copyright (c) 2007 Tokutek Inc.  All rights reserved."
#include "test.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <memory.h>
#include <sys/stat.h>
#include <db.h>


/* verify that the dup flags are written and read from the database file correctly */ 
static void
test_dup_flags (u_int32_t dup_flags) {
    if (verbose) printf("test_dup_flags:%u\n", dup_flags);

    DB_TXN * const null_txn = 0;
    const char * const fname = "test_dup_flags.ft_handle";
    int r;

    r = system("rm -rf " ENVDIR); CKERR(r);
    r = toku_os_mkdir(ENVDIR, S_IRWXU+S_IRWXG+S_IRWXO); CKERR(r);

    /* create the dup database file */
    DB_ENV *env;
    r = db_env_create(&env, 0); assert(r == 0);
    r = env->open(env, ENVDIR, DB_CREATE+DB_PRIVATE+DB_INIT_MPOOL, 0); assert(r == 0);

    DB *db;
    r = db_create(&db, env, 0); assert(r == 0);
    if (IS_TDB) assert(dup_flags==0);
    r = db->set_flags(db, dup_flags);
    assert(r == 0);
    u_int32_t flags; r = db->get_flags(db, &flags); assert(r == 0); assert(flags == dup_flags);
    if (IS_TDB) assert(dup_flags==0);
    r = db->open(db, null_txn, fname, "main", DB_BTREE, DB_CREATE, 0666); 
    assert(r == 0);
    r = db->close(db, 0); assert(r == 0);

    /* verify dup flags match */
    r = db_create(&db, env, 0); assert(r == 0);
    r = db->open(db, null_txn, fname, "main", DB_BTREE, 0, 0666);
    if (r == 0 && verbose) 
        printf("%s:%d: WARNING:open ok:dup_mode:%u\n", __FILE__, __LINE__, dup_flags);
    r = db->close(db, 0); assert(r == 0);

    r = db_create(&db, env, 0); assert(r == 0);
    r = db->set_flags(db, dup_flags); assert(r == 0);
    r = db->open(db, null_txn, fname, "main", DB_BTREE, 0, 0666); assert(r == 0);
    r = db->close(db, 0); assert(r == 0);

    /* verify nodesize match */
    r = db_create(&db, env, 0); assert(r == 0);
    r = db->set_flags(db, dup_flags); assert(r == 0);
    r = db->set_pagesize(db, 4096); assert(r == 0);
    r = db->open(db, null_txn, fname, "main", DB_BTREE, 0, 0666); assert(r == 0);
    r = db->close(db, 0); assert(r == 0);
    r = env->close(env, 0); assert(r == 0);
}

int
test_main(int argc, char *const argv[]) {

    parse_args(argc, argv);
  
    int r;
    r = system("rm -rf " ENVDIR);
    CKERR(r);
    toku_os_mkdir(ENVDIR, S_IRWXU+S_IRWXG+S_IRWXO);

    /* test flags */
    test_dup_flags(0);

    return 0;
}