package arm;

import kha.System;

class Player {

	public function new() {
		#if krom_windows
		Krom.setSaveAndQuitCallback(saveAndQuitCallback);
		#end
	}

	static function saveAndQuitCallback(save: Bool) {
		System.stop();
	}
}
