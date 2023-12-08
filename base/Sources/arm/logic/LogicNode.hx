package arm.logic;

import iron.System;

class LogicNode {
	var tree: LogicTree;
	var inputs: Array<LogicNodeInput> = [];
	var outputs: Array<Array<LogicNode>> = [];

	public function new(tree: LogicTree) {
		this.tree = tree;
	}

	public function addInput(node: LogicNode, from: Int) {
		inputs.push(new LogicNodeInput(node, from));
	}

	public function addOutputs(nodes: Array<LogicNode>) {
		outputs.push(nodes);
	}

	public function get(from: Int, done: Dynamic->Void) {
		done(null);
	}

	public function getAsImage(from: Int, done: Image->Void) {
		done(null);
	}

	public function getCachedImage(): Image {
		return null;
	}

	public function set(value: Dynamic) {}
}

class LogicNodeInput {
	var node: LogicNode;
	var from: Int; // Socket index

	public function new(node: LogicNode, from: Int) {
		this.node = node;
		this.from = from;
	}

	public function get(done: Dynamic->Void) {
		node.get(from, done);
	}

	public function getAsImage(done: Image->Void) {
		node.getAsImage(from, done);
	}

	public function set(value: Dynamic) {
		node.set(value);
	}
}

class LogicTree {

	////
	public var name: String = "";
	public var object: iron.object.Object; // Object this trait belongs to

	var _add: Array<Void->Void> = null;
	var _init: Array<Void->Void> = null;
	var _remove: Array<Void->Void> = null;
	var _update: Array<Void->Void> = null;
	var _lateUpdate: Array<Void->Void> = null;
	var _render: Array<Graphics4->Void> = null;
	var _render2D: Array<Graphics2->Void> = null;

	public function remove() {
		// object.removeTrait(this);
	}

	public function notifyOnAdd(f: Void->Void) {
		if (_add == null) _add = [];
		_add.push(f);
	}

	public function notifyOnInit(f: Void->Void) {
		if (_init == null) _init = [];
		_init.push(f);
		iron.App.notifyOnInit(f);
	}

	public function notifyOnRemove(f: Void->Void) {
		if (_remove == null) _remove = [];
		_remove.push(f);
	}

	public function notifyOnUpdate(f: Void->Void) {
		if (_update == null) _update = [];
		_update.push(f);
		iron.App.notifyOnUpdate(f);
	}

	public function removeUpdate(f: Void->Void) {
		_update.remove(f);
		iron.App.removeUpdate(f);
	}

	public function notifyOnLateUpdate(f: Void->Void) {
		if (_lateUpdate == null) _lateUpdate = [];
		_lateUpdate.push(f);
		iron.App.notifyOnLateUpdate(f);
	}

	public function removeLateUpdate(f: Void->Void) {
		_lateUpdate.remove(f);
		iron.App.removeLateUpdate(f);
	}

	public function notifyOnRender(f: Graphics4->Void) {
		if (_render == null) _render = [];
		_render.push(f);
		iron.App.notifyOnRender(f);
	}

	public function removeRender(f: Graphics4->Void) {
		_render.remove(f);
		iron.App.removeRender(f);
	}

	public function notifyOnRender2D(f: Graphics2->Void) {
		if (_render2D == null) _render2D = [];
		_render2D.push(f);
		iron.App.notifyOnRender2D(f);
	}

	public function removeRender2D(f: Graphics2->Void) {
		_render2D.remove(f);
		iron.App.removeRender2D(f);
	}
	////

	public function new() {}
}
