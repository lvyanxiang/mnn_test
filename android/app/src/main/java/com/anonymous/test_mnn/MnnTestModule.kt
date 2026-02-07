package com.anonymous.test_mnn

import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReactContextBaseJavaModule
import com.facebook.react.bridge.ReactMethod
import com.facebook.react.bridge.Promise

class MnnTestModule(reactContext: ReactApplicationContext) : ReactContextBaseJavaModule(reactContext) {

    external fun nativeTestCall(): String

    companion object {
        init {
            System.loadLibrary("mnn-test")
        }
    }

    override fun getName(): String {
        return "MnnTestModule"
    }

    @ReactMethod
    fun testCall(promise: Promise) {
        try {
            val result = nativeTestCall()
            promise.resolve(result)
        } catch (e: Exception) {
            promise.reject("NATIVE_ERROR", e.message)
        }
    }
}
