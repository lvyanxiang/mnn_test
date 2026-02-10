package com.anonymous.test_mnn

import android.content.Context
import android.util.Log
import java.io.File
import java.io.FileOutputStream
import java.io.InputStream

object ModelExtractor {
    private const val TAG = "ModelExtractor"
    // RFB-320.mnn  det_10g.mnn
    private const val MODEL_NAME = "RFB-320.mnn"

    init {
        // Load native library when object is initialized
        try {
            Log.i(TAG, "Loading native library: appmodules")
            System.loadLibrary("appmodules")
            Log.i(TAG, "Native library loaded successfully")
        } catch (e: UnsatisfiedLinkError) {
            Log.e(TAG, "Failed to load native library: ${e.message}")
        }
    }

    /**
     * 从 assets 提取模型到应用缓存目录
     * @return 提取后的模型文件路径，失败返回空字符串
     */
    fun extractModelIfNeeded(context: Context): String {
        val cacheDir = context.cacheDir
        val modelFile = File(cacheDir, MODEL_NAME)

        // 检查是否已存在
        if (modelFile.exists()) {
            Log.i(TAG, "Model already exists: ${modelFile.absolutePath}")
            return modelFile.absolutePath
        }

        // 从 assets 提取
        Log.i(TAG, "Extracting model from assets...")
        try {
            val inputStream: InputStream = context.assets.open(MODEL_NAME)
            val outputStream = FileOutputStream(modelFile)

            val buffer = ByteArray(8192)
            var bytesRead: Int
            while (inputStream.read(buffer).also { bytesRead = it } != -1) {
                outputStream.write(buffer, 0, bytesRead)
            }

            outputStream.close()
            inputStream.close()

            Log.i(TAG, "Model extracted to: ${modelFile.absolutePath}")
            return modelFile.absolutePath
        } catch (e: Exception) {
            Log.e(TAG, "Failed to extract model: ${e.message}", e)
            return ""
        }
    }

    /**
     * 获取模型文件路径（如果不存在则自动提取）
     * @return 模型文件的完整路径
     */
    fun getModelPath(context: Context): String {
        val path = extractModelIfNeeded(context)
        if (path.isNotEmpty()) {
            // 设置给 C++
            nativeSetModelPath(path)
            Log.i(TAG, "Model path set via JNI: $path")
        } else {
            Log.e(TAG, "Failed to get model path")
        }
        return path
    }

    /**
     * 检查模型是否已准备好
     * @return true 如果模型文件存在
     */
    fun isModelReady(context: Context): Boolean {
        val modelFile = File(context.cacheDir, MODEL_NAME)
        return modelFile.exists()
    }

    /**
     * Native 方法声明：设置模型路径供 C++ 使用
     */
    private external fun nativeSetModelPath(modelPath: String)
}
