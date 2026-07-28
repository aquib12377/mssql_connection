// Forces the FreeTDS DB-Lib symbols that lib/src/ffi/freetds_bindings.dart
// resolves at runtime via DynamicLibrary.process()/dlsym to survive the
// linker's dead-code stripping.
//
// Nothing in native (Obj-C/Swift/C) code calls these functions directly --
// Dart's dlsym-based FFI lookup isn't visible to the linker as a "use" of
// the archive member that defines them -- so without an explicit reference
// here, the unused object files inside the vendored libsybdb.a static
// archive get dropped from the final binary and
// DynamicLibrary.process().lookup('dbinit') fails with "symbol not found".
//
// Referencing each symbol's address (rather than force-loading the whole
// archive with -all_load) pulls in only the object files actually needed,
// avoiding duplicate-symbol clashes with the FreeTDS core objects that
// libsybdb.a and libct.a may both embed.
//
// These are deliberately untyped forward declarations rather than an
// #include of sybdb.h: only the address of each symbol is taken (never
// called through), so the exact signature doesn't matter here, and this
// keeps the shim independent of the xcframework's header search paths.
extern void dbinit(void);
extern void dblogin(void);
extern void dbloginfree(void);
extern void dbsetlname(void);
extern void dbsetlbool(void);
extern void tdsdbopen(void);
extern void dbclose(void);
extern void dbexit(void);
extern void dbcmd(void);
extern void dbsqlexec(void);
extern void dbresults(void);
extern void dbnextrow(void);
extern void dbnumcols(void);
extern void dbcolname(void);
extern void dbcoltype(void);
extern void dbdatlen(void);
extern void dbdata(void);
extern void dbcount(void);
extern void dbsetlogintime(void);
extern void dbsettime(void);
extern void dbuse(void);
extern void dbsetopt(void);
extern void dbrpcinit(void);
extern void dbrpcparam(void);
extern void dbrpcsend(void);
extern void dbsqlok(void);
extern void dberrhandle(void);
extern void dbmsghandle(void);
extern void bcp_init(void);
extern void bcp_bind(void);
extern void bcp_sendrow(void);
extern void bcp_batch(void);
extern void bcp_done(void);
extern void bcp_collen(void);
extern void bcp_colptr(void);
extern void dbconvert(void);

__attribute__((used)) static void *mssql_connection_dblib_force_link[] = {
    (void *)dbinit,
    (void *)dblogin,
    (void *)dbloginfree,
    (void *)dbsetlname,
    (void *)dbsetlbool,
    (void *)tdsdbopen,
    (void *)dbclose,
    (void *)dbexit,
    (void *)dbcmd,
    (void *)dbsqlexec,
    (void *)dbresults,
    (void *)dbnextrow,
    (void *)dbnumcols,
    (void *)dbcolname,
    (void *)dbcoltype,
    (void *)dbdatlen,
    (void *)dbdata,
    (void *)dbcount,
    (void *)dbsetlogintime,
    (void *)dbsettime,
    (void *)dbuse,
    (void *)dbsetopt,
    (void *)dbrpcinit,
    (void *)dbrpcparam,
    (void *)dbrpcsend,
    (void *)dbsqlok,
    (void *)dberrhandle,
    (void *)dbmsghandle,
    (void *)bcp_init,
    (void *)bcp_bind,
    (void *)bcp_sendrow,
    (void *)bcp_batch,
    (void *)bcp_done,
    (void *)bcp_collen,
    (void *)bcp_colptr,
    (void *)dbconvert,
};
