package tech.kinc

import java.util.ArrayList

import android.view.Surface

class KincMoviePlayer(var path: String) {
	companion object {
		var players = ArrayList<KincMoviePlayer?>()

		@JvmStatic
		fun updateAll() {
			for (player in KincMoviePlayer.players) {
				player!!.update()
			}
		}

		fun remove(id: Int) {
			players[id] = null
		}
	}

	private var movieTexture: KincMovieTexture? = null
	var id: Int = players.size

	init {
		players.add(this)
	}
	
	fun init() {
		movieTexture = KincMovieTexture()
		val surface = Surface(movieTexture!!.surfaceTexture)
		nativeCreate(path, surface, id)
		surface.release()
	}

	fun getMovieTexture(): KincMovieTexture? {
		return movieTexture
	}

	fun update(): Boolean {
		return movieTexture!!.update()
	}
	
	fun getTextureId(): Int {
		return movieTexture!!.textureId
	}

	private external fun nativeCreate(path: String, surface: Surface, id: Int)
}
