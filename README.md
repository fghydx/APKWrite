# APKWrite
向已经签过名的APK中写入数据(支持2代签名)

当初做这个东西是为了方便把APK给渠道商，以便获取这个APK是由谁分发出去的，在写APK时，通过调用so获取APK中的一段数据(这段数据是在APK签名后才写进去的)。重新打包的方法会很慢，而通过这种方法会相当快
