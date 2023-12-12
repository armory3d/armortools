package arm.logic;

import iron.System;

class LogicNode {
	public var inputs: Array<LogicNodeInput> = [];
	public var outputs: Array<Array<LogicNode>> = [];

	public function new() {}

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
	public var node: LogicNode;
	public var from: Int; // Socket index

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
