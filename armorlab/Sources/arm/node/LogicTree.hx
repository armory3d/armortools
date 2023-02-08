package arm.node;

class LogicTree extends iron.Trait {

	public var loopBreak = false; // Trigger break from loop nodes

	public function new() {
		super();
	}

	public function add() {}

	var paused = false;

	public function pause() {
		if (paused) return;
		paused = true;

		if (_update != null) for (f in _update) iron.App.removeUpdate(f);
		if (_lateUpdate != null) for (f in _lateUpdate) iron.App.removeLateUpdate(f);
	}

	public function resume() {
		if (!paused) return;
		paused = false;

		if (_update != null) for (f in _update) iron.App.notifyOnUpdate(f);
		if (_lateUpdate != null) for (f in _lateUpdate) iron.App.notifyOnLateUpdate(f);
	}
}
