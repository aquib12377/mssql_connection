# PHASE 1 & 2 Diagnostic Implementation Report

## Summary

PHASE 1 and PHASE 2 have been implemented to add comprehensive FreeTDS error diagnostics and configuration capabilities for debugging the Android TLS/pre-login handshake failure with SQL Server 2019.

---

## PHASE 1: Diagnostics (COMPLETED)

### Native Error-Handling Path (Before Changes)

The FreeTDS DB-Lib wrapper already had error handler infrastructure in place:

1. **Handler Installation** (`mssql_client.dart`, lines 76-79):
   - `dberrhandle(kErrHandlerPtr)` - installs DB-Lib error handler
   - `dbmsghandle(kMsgHandlerPtr)` - installs DB-Lib message handler

2. **Error Capture** (`freetds_bindings.dart`, lines 642-713):
   - `_dartDbErrHandler()` captures severity, dberr, oserr, and error strings
   - `_dartDbMsgHandler()` captures msgno, msgstate, severity, and message text
   - Both store messages in `_DbLibErrorStore` via key-value maps per DBPROCESS pointer

3. **Critical Gap**: Errors were **never retrieved** after `dbopen()` failed
   - When `dbopen()` returned `nullptr`, the code just logged and returned `false`
   - The captured error messages were lost (never called `takeLastError()`)
   - This is why connection failures appeared silent in logcat

### Changes Made (Phase 1)

#### 1. **Expose Error via Public Getter**
   - Added `_lastError: String?` field to `MssqlClient`
   - Added `lastError` getter for Dart code to access FreeTDS error message
   - Exposed through `MssqlConnection.lastError` getter

#### 2. **Retrieve Errors on Connection Failure**
   - Modified connection path to call `DBLib.takeLastError()` and `DBLib.takeLastMessage()` 
   - When `dbopen()` returns `nullptr`, error is now captured and stored in `_lastError`
   - Error message is logged to facilitate debugging

#### 3. **Add TDSDUMP Tracing Support**
   - Added FFI binding for `setenv()` from libc.so.6
   - Implemented `setEnvironmentVariable()` method in `DBLib` class
   - Added `_enableTdsDump()` method to set `TDSDUMP` environment variable
   - Creates trace file in system temp directory for FreeTDS protocol logging
   - Added `getTdsDumpContents()` to retrieve and read trace file
   - Optional `enableTraceDump` parameter on `connect()`

#### 4. **Logging Enhancements**
   - Error messages now logged immediately when connection fails
   - TDSDUMP configuration logged for verification
   - All logging prefixed with `connect |` for easy grep/search in logcat

### Key Code Changes

**File: `lib/src/mssql_client.dart`**
```dart
class MssqlClient {
  String? _lastError;
  String? get lastError => _lastError;
  
  Future<bool> connect({
    int loginTimeoutSeconds = 15,
    bool enableTraceDump = false,
  }) async {
    _lastError = null;
    
    if (enableTraceDump) {
      await _enableTdsDump();  // Enable TDSDUMP tracing
    }
    
    // ... DB-Lib setup ...
    
    if (_dbproc == nullptr) {
      // CRITICAL FIX: Retrieve and expose the error
      _lastError = DBLib.takeLastError(nullptr) ?? 
                   DBLib.takeLastMessage(nullptr) ?? 
                   'Connection failed (no additional details)';
      MssqlLogger.e('connect | op=dbopen | error=$_lastError');
      return false;
    }
  }
}
```

**File: `lib/src/ffi/freetds_bindings.dart`**
```dart
// Added setenv FFI binding
typedef _setenvC = Int32 Function(Pointer<Utf8>, Pointer<Utf8>, Int32);
typedef _setenvDart = int Function(Pointer<Utf8>, Pointer<Utf8>, int);

class DBLib {
  bool setEnvironmentVariable(String name, String value) {
    if (_setenv == null) return false;
    final n = name.toNativeUtf8();
    final v = value.toNativeUtf8();
    try {
      final rc = _setenv!(n, v, 1); // overwrite=1
      return rc == 0;
    } finally {
      malloc.free(n);
      malloc.free(v);
    }
  }
}
```

### How to Enable Diagnostics (Phase 1)

**Enable full logging + tracing:**
```dart
// Enable Dart logging
MssqlLogger.enabled = true;
MssqlClient.enabled = true;

// Connect with tracing
final result = await MssqlConnection.getInstance().connect(
  ip: '192.168.x.x',
  port: '1433',
  databaseName: 'MyDB',
  username: 'user',
  password: 'pass',
  enableTraceDump: true,  // NEW: Enable TDSDUMP protocol tracing
);

if (!result) {
  // NEW: Access the actual FreeTDS error
  final error = MssqlConnection.getInstance().lastError;
  print('Connection failed: $error');  // Will now show real error!
}
```

**Retrieve TDSDUMP trace file:** (requires the trace file path from `_enableTdsDump()`)
```dart
// After failed connection, trace file path is in system temp directory
// File pattern: freetds_trace_<timestamp>.log
final contents = await connection.getTdsDumpContents('/data/local/tmp/freetds_trace_1234567890.log');
print(contents);  // Detailed FreeTDS protocol handshake details
```

---

## PHASE 2: Configuration Knobs (COMPLETED)

### Purpose
Without rebuilding native code, allow testing of TDS version, encryption, and cipher settings to isolate the cause of the handshake failure.

### What Can Be Tested

1. **TDS Version Forcing**
   - Default: FreeTDS auto-negotiates
   - Test: Force TDS 7.3 or 7.2 if TDS 7.4 has issues with this SQL Server version

2. **Encryption Control**
   - `"off"` - disable all encryption (fast way to test if it's TLS-specific)
   - `"request"` - encryption optional (server may not use it)
   - `"require"` - encryption mandatory

3. **OpenSSL Cipher Suite**
   - Constrain to older/newer ciphers
   - Test specific cipher combinations if server/client mismatch suspected

### Implementation

#### 1. **freetds.conf Generation**
```dart
Future<void> _configureFreeTds({
  String? tdsVersion,
  String? encryption,
  String? opensslCiphers,
}) async {
  // Generates freetds.conf with settings:
  // tds version = 7.3
  // encryption = off
  // openssl ciphers = <suite>
  
  // Writes to temp directory
  // Sets FREETDSCONF environment variable to point to it
}
```

#### 2. **Public API**
```dart
// MssqlConnection.connect() now accepts:
Future<bool> connect({
  required String ip,
  required String port,
  required String databaseName,
  required String username,
  required String password,
  int timeoutInSeconds = 15,
  bool enableTraceDump = false,           // NEW (Phase 1)
  String? tdsVersion,                     // NEW (Phase 2)
  String? encryption,                     // NEW (Phase 2)
  String? opensslCiphers,                 // NEW (Phase 2)
}) async {
  // ...
  final ok = await _client!.connect(
    loginTimeoutSeconds: _timeout,
    enableTraceDump: enableTraceDump,
    tdsVersion: tdsVersion,
    encryption: encryption,
    opensslCiphers: opensslCiphers,
  );
}
```

### How to Use Phase 2

**Test 1: Disable encryption (fastest diagnosis)**
```dart
final result = await connection.connect(
  ip: '192.168.x.x',
  port: '1433',
  databaseName: 'MyDB',
  username: 'user',
  password: 'pass',
  encryption: 'off',  // Force no encryption
);

if (result) {
  print('SUCCESS: Connection works without encryption!');
  print('Conclusion: Issue is in TLS/encryption handshake');
} else {
  print('FAILED: Even without encryption fails');
  print('Conclusion: Issue is pre-login TDS layer or non-TLS');
}
```

**Test 2: Force older TDS version**
```dart
final result = await connection.connect(
  ip: '192.168.x.x',
  port: '1433',
  databaseName: 'MyDB',
  username: 'user',
  password: 'pass',
  tdsVersion: '7.3',  // Force TDS 7.3 instead of 7.4
);
```

**Test 3: Constraint cipher suite**
```dart
final result = await connection.connect(
  ip: '192.168.x.x',
  port: '1433',
  databaseName: 'MyDB',
  username: 'user',
  password: 'pass',
  opensslCiphers: 'DEFAULT',  // Or specific ciphers like 'AES256-SHA'
);
```

---

## PHASE 3: Native Library Analysis

### FreeTDS Version: 1.5.4
Binary analysis of `libsybdb.so` (arm64-v8a):
- **Version**: FreeTDS 1.5.4 (from build path: `third_party/freetds-1.5.4`)
- **Build Context**: GitHub Actions (runner/work path indicates CI build)
- **Architecture**: ARM64 (aarch64), dynamically linked
- **Debug Info**: Present (not stripped), BuildID available

### TLS/Encryption Capabilities
Strings analysis shows:
- ✅ TLS support present (references to `enable tls v1`, encryption modes)
- ✅ OpenSSL cipher configuration (`openssl ciphers` directive found)
- ✅ Multiple encryption levels: `TDS7_ENCRYPT_OFF`, `TDS7_ENCRYPT_REQ`, `TDS7_ENCRYPT_ON`
- ✅ Error messages for encryption validation present
- ⚠️ Linked dependencies show only `libc.so`, `libm.so`, `libdl.so` 
  - (Suggests OpenSSL/GnuTLS may be **statically linked** or **not included**)

### Hypothesis for Phase 3

The binary likely has OpenSSL **statically linked**. If the static OpenSSL version is:
- Very old (pre-1.0.2): May not support modern TLS or required ciphers
- Missing specific protocols: Could lack TLS 1.2 or reject certain algorithms
- Incompatible with Windows Server 2022 negotiation: TLS version mismatch

### What Phase 3 Should Address

1. **Verify static/dynamic OpenSSL linking**
   - Use `strings | grep` for OpenSSL version identifiers
   - Check if rebuild needed or if tuning via PHASE 2 options suffices

2. **Determine OpenSSL version used in build**
   - Extract version from compiled binary if possible
   - Check GitHub Actions build history for toolchain version

3. **If rebuild needed**:
   - Create NDK build script with current OpenSSL (1.1.1+)
   - Document exact compilation flags
   - Target: arm64-v8a, armeabi-v7a, x86_64

---

## Testing Instructions

### Recommended Test Sequence

1. **Enable logging** in your app:
```dart
MssqlLogger.enabled = true;  // See logs in Android logcat
```

2. **Enable diagnostics on connect**:
```dart
final ok = await connection.connect(
  ip: serverIp,
  port: '1433',
  databaseName: 'master',
  username: 'sa',
  password: 'YourPassword',
  enableTraceDump: true,  // Captures protocol trace
);
```

3. **Check results**:
```dart
if (!ok) {
  print('Error: ${connection.lastError}');  // Real FreeTDS error message
  // Manually retrieve TDSDUMP file from system temp directory
  // File pattern: /data/local/tmp/freetds_trace_*.log
}
```

4. **Run Phase 2 tests**:
```dart
// Test without encryption first
await connection.connect(..., encryption: 'off');
```

5. **Collect diagnostics**:
   - Logcat output (full MssqlLogger output)
   - TDSDUMP trace file contents
   - lastError message
   - SQL Server error log entries

---

## Summary of Changes

### Files Modified:
1. `lib/src/mssql_client.dart`
   - Added `_lastError` field and getter
   - Added `_enableTdsDump()` method
   - Added `getTdsDumpContents()` method
   - Added `_configureFreeTds()` method
   - Updated `connect()` signature with new parameters

2. `lib/src/mssql_connection.dart`
   - Exposed `lastError` getter
   - Exposed `getTdsDumpContents()` method
   - Added new parameters to public `connect()` API

3. `lib/src/ffi/freetds_bindings.dart`
   - Added `setenv` FFI bindings
   - Added `setEnvironmentVariable()` to DBLib
   - Attempted to load libc.so.6 for environment variable support

### Public API Remains Compatible
- All new parameters are optional with sensible defaults
- Existing code continues to work unchanged
- No breaking changes to existing method signatures

---

## Next Steps (Phase 3 Requires Manual Investigation)

1. Use Phase 1 diagnostics to capture actual error message
2. Use Phase 2 configuration to isolate the issue (encryption on/off, TDS version)
3. Based on findings, determine if native rebuild needed
4. If rebuild required, create reproducible NDK build script

Once Phase 1 diagnostics run against your server, the actual FreeTDS error message will guide Phase 3 decisions.
