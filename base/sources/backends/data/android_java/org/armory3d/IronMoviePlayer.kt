package org.armory3d

import java.util.ArrayList

import android.view.Surface

class IronMoviePlayer(var path: String) {
	companion object {
		var players = ArrayList<IronMoviePlayer?>()

		@JvmStatic
		fun updateAll() {
			for (player in IronMoviePlayer.players) {
				player!!.update()
			}
		}

		fun remove(id: Int) {
			players[id] = null
		}
	}

	private var movieTexture: IronMovieTexture? = null
	var id: Int = players.size

	init {
		players.add(this)
	}

	fun init() {
		movieTexture = IronMovieTexture()
		val surface = Surface(movieTexture!!.surfaceTexture)
		nativeCreate(path, surface, id)
		surface.release()
	}

	fun getMovieTexture(): IronMovieTexture? {
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
