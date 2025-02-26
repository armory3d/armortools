package tech.kinc

import android.app.NativeActivity
import android.content.Context
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.os.Vibrator
import android.os.VibrationEffect
import android.os.Build
import android.view.KeyEvent
import android.view.View
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import kotlin.system.exitProcess
import android.content.ContentResolver;
import android.util.Log
import android.view.DragAndDropPermissions;
import android.view.DragEvent;
import android.webkit.MimeTypeMap;
import android.view.DragEvent.ACTION_DRAG_STARTED;
import android.view.DragEvent.ACTION_DROP;
import androidx.core.database.getStringOrNull
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import kotlin.math.log

class KincActivity: NativeActivity(), KeyEvent.Callback {
	companion object {
		var instance: KincActivity? = null

		@JvmStatic
		fun showKeyboard() {
			instance!!.inputManager!!.showSoftInput(instance!!.window.decorView, 0)
		}

		@JvmStatic
		fun hideKeyboard() {
			instance!!.inputManager!!.hideSoftInputFromWindow(instance!!.window.decorView.windowToken, 0)
			instance!!.delayedHideSystemUI()
		}

		@JvmStatic
		fun loadURL(url: String) {
			val i = Intent(Intent.ACTION_VIEW, Uri.parse(url))
			instance!!.startActivity(i)
		}

		@JvmStatic
		fun getLanguage(): String {
			return java.util.Locale.getDefault().language
		}

		@JvmStatic
		fun getRotation(): Int {
			val context: Context = instance!!.applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			return manager.defaultDisplay.rotation
		}

		@JvmStatic
		fun getScreenDpi(): Int {
			val context: Context = instance!!.applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			val metrics: android.util.DisplayMetrics = android.util.DisplayMetrics()
			manager.defaultDisplay.getMetrics(metrics)
			return metrics.xdpi.toInt()
		}

		@JvmStatic
		fun getRefreshRate(): Int {
			val context: Context = instance!!.applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			return manager.defaultDisplay.refreshRate.toInt()
		}

		@JvmStatic
		fun getDisplayWidth(): Int {
			val context: Context = instance!!.applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			val size: android.graphics.Point = android.graphics.Point()
			manager.defaultDisplay.getRealSize(size)
			return size.x
		}

		@JvmStatic
		fun getDisplayHeight(): Int {
			val context: Context = instance!!.applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			val size: android.graphics.Point = android.graphics.Point()
			manager.defaultDisplay.getRealSize(size)
			return size.y
		}

		@JvmStatic
		fun stop() {
			instance!!.runOnUiThread {
				fun run() {
					instance!!.finish()
					exitProcess(0)
				}
			}
		}

		class MyHandler(private val kincActivity: KincActivity) : Handler() {
			override fun handleMessage(msg: Message) {
				kincActivity.hideSystemUI()
			}
		}

		@JvmStatic
		public fun pickFile() {
			val intent: Intent = Intent(Intent.ACTION_GET_CONTENT)
			intent.type = "*/*"
			instance!!.startActivityForResult(Intent.createChooser(intent, "Select File"), 1)
		}
	}

	var inputManager: InputMethodManager? = null
	private var isDisabledStickyImmersiveMode = false

	private val hideSystemUIHandler = MyHandler(this)

	override fun onCreate(state: Bundle?) {
		super.onCreate(state)
		hideSystemUI()
		instance = this
		inputManager = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
		isDisabledStickyImmersiveMode = try {
			val ai: ApplicationInfo = packageManager.getApplicationInfo(packageName, PackageManager.GET_META_DATA)
			val bundle: Bundle = ai.metaData
			bundle.getBoolean("disableStickyImmersiveMode")
		} catch (e: PackageManager.NameNotFoundException) {
			false
		} catch (e: NullPointerException) {
			false
		}

		window.decorView.setOnDragListener(
				fun (view: View, dragEvent: DragEvent): Boolean {
					if (dragEvent.action == ACTION_DRAG_STARTED) return true
					if (dragEvent.action == ACTION_DROP) {
						val dropPermissions = requestDragAndDropPermissions(dragEvent)
						importFile(dragEvent.clipData.getItemAt(0).uri)
						dropPermissions.release()
						return true
					}
					return false
				}
		);
	}

    private fun hideSystemUI() {
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_LAYOUT_STABLE or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
    }

    private fun delayedHideSystemUI() {
        hideSystemUIHandler.removeMessages(0)
        if (!isDisabledStickyImmersiveMode) {
            hideSystemUIHandler.sendEmptyMessageDelayed(0, 300)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            delayedHideSystemUI()
        }
        else {
            hideSystemUIHandler.removeMessages(0)
        }
    }

	override fun onKeyMultiple(keyCode: Int, count: Int, event: KeyEvent): Boolean {
		this.nativeKincKeyPress(event.characters)
		return false
	}

	private external fun nativeKincKeyPress(chars: String)

	private external fun onAndroidFilePicked(pickedPath: String)
	private external fun getMobileTitle(): String

	override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent) {
		super.onActivityResult(requestCode, resultCode, data)
		if (requestCode == 1 && resultCode == RESULT_OK) {
			importFile(data.data!!)
		}
	}

	private fun importFile(pickedFile: Uri) {
		val resolver: ContentResolver = applicationContext.contentResolver
		val inps: InputStream = resolver.openInputStream(pickedFile)!!
		try {
			val bis: BufferedInputStream = BufferedInputStream(inps)
			val dir: File = File(filesDir.absolutePath + "/" + getMobileTitle())
			dir.mkdirs()
			var path: List<String> = pickedFile.path!!.split("/")

			// Samsung files app removes extension from fileName
			val filePath: Array<String> = arrayOf(android.provider.MediaStore.Images.Media.DATA)
			val cursor: android.database.Cursor = contentResolver.query(pickedFile, filePath, null, null, null)!!
			cursor.moveToFirst()
			val pickedPath: String? = cursor.getStringOrNull(cursor.getColumnIndex(filePath[0]))
			if (pickedPath != null) {
				path = pickedPath.split("/")
			}
			cursor.close()

			var fileName: String = path[path.size - 1]

			// Extension still unknown
			if (!fileName.contains(".")) {
				var ext: String = MimeTypeMap.getSingleton().getExtensionFromMimeType(contentResolver.getType(pickedFile))!!
				// Note: for obj/fbx file, the extension returned is bin..
				if (ext == "bin") {
					bis.mark(0)
					val header: StringBuilder = StringBuilder()
					for (i in 0..17) {
						val c: Int = bis.read()
						if (c == -1) break
						header.append(c.toChar())
					}
					ext = if (header.toString() == "Kaydara FBX Binary") "fbx" else "obj"
					bis.reset()
				}
				fileName += "." + ext
			}

			val dst: String = filesDir.absolutePath + "/" + getMobileTitle() + "/" + fileName
			val os: OutputStream = FileOutputStream(dst)
			try {
				val buf = ByteArray(1024)
				var len = bis.read(buf)
				while (len > 0) {
					os.write(buf, 0, len)
					len = bis.read(buf)
				}
				onAndroidFilePicked(dst)
			}
			catch (e: IOException) {}
		}
		catch (e: FileNotFoundException) {}
		catch (e: IOException) {}
	}
}
