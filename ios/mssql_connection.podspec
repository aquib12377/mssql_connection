#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint mssql_connection.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'mssql_connection'
  s.version          = '3.0.0'
  s.summary          = 'Flutter/Dart plugin to connect to Microsoft SQL Server using Dart FFI + FreeTDS.'
  s.description      = <<-DESC
Flutter/Dart plugin to connect to Microsoft SQL Server using Dart FFI + FreeTDS.
                       DESC
  s.homepage         = 'https://github.com/Hiteshdon/mssql_connection'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Hiteshdon' => 'noreply@example.com' }
  s.source           = { :path => '.' }

  # Vendor the prebuilt FreeTDS static-library XCFrameworks that lib/src/ffi
  # resolves at runtime via DynamicLibrary.process(). Classes/ holds a single
  # C shim (see mssql_connection_force_link.c) whose only job is to take the
  # address of every DB-Lib symbol the Dart bindings look up -- without it,
  # the linker treats those symbols as unused and dead-strips them, since
  # Dart's dlsym-based FFI lookup isn't visible to it as a "use".
  s.source_files = 'Classes/**/*'
  s.vendored_frameworks = 'FreeTDS/FreeTDS-DB.xcframework', 'FreeTDS/FreeTDS-CT.xcframework'
  s.dependency 'Flutter'
  s.platform = :ios, '12.0'

  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
  }

  s.swift_version = '5.0'
end
