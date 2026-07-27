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

  # This FFI plugin ships no native source of its own; it only vendors the
  # prebuilt FreeTDS static-library XCFrameworks that lib/src/ffi resolves
  # at runtime via DynamicLibrary.process().
  s.vendored_frameworks = 'FreeTDS/FreeTDS-DB.xcframework', 'FreeTDS/FreeTDS-CT.xcframework'
  s.dependency 'Flutter'
  s.platform = :ios, '12.0'

  # DynamicLibrary.process() resolves symbols via dlsym on the running
  # process image. Static-archive members that nothing in the native (ObjC/
  # Swift/C) link graph references are otherwise dead-stripped by the linker
  # before Dart ever gets a chance to look them up, so force the whole
  # FreeTDS archives to be kept.
  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
    'OTHER_LDFLAGS' => '-all_load',
  }

  s.swift_version = '5.0'
end
