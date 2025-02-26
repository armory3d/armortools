package tech.kinc

import android.graphics.SurfaceTexture
import android.graphics.SurfaceTexture.OnFrameAvailableListener
import android.opengl.GLES20

class KincMovieTexture: OnFrameAvailableListener {
	private val GL_TEXTURE_EXTERNAL_OES: Int = 0x8D65

	var textureId: Int = 0

	init {
		val textures = IntArray(1)
		GLES20.glGenTextures(1, textures, 0)
		textureId = textures[0]

		GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId)
		GLES20.glTexParameteri(
			GL_TEXTURE_EXTERNAL_OES,
			GLES20.GL_TEXTURE_MIN_FILTER,
			GLES20.GL_NEAREST
		)
		GLES20.glTexParameteri(
			GL_TEXTURE_EXTERNAL_OES,
			GLES20.GL_TEXTURE_MAG_FILTER,
			GLES20.GL_LINEAR
		)
		GLES20.glTexParameteri(
			GL_TEXTURE_EXTERNAL_OES,
			GLES20.GL_TEXTURE_WRAP_S,
			GLES20.GL_CLAMP_TO_EDGE
		)
		GLES20.glTexParameteri(
			GL_TEXTURE_EXTERNAL_OES,
			GLES20.GL_TEXTURE_WRAP_T,
			GLES20.GL_CLAMP_TO_EDGE
		)
	}

	var surfaceTexture = SurfaceTexture(textureId)

	init {
		surfaceTexture.setOnFrameAvailableListener(this)
	}

	private var updateTexture = false

	fun update(): Boolean {
		val ret = updateTexture
		if (updateTexture) {
			surfaceTexture.updateTexImage()
			updateTexture = false
		}
		return ret
	}
	
	override fun onFrameAvailable(surface: SurfaceTexture) {
		if (surfaceTexture == surface) {
			updateTexture = true
		}
	}
}
