
class BooleanNode extends LogicNode {

	value: bool;

	constructor(value = false) {
		super();
		this.value = value;
	}

	override get = (from: i32, done: (a: any)=>void) => {
		if (this.inputs.length > 0) this.inputs[0].get(done);
		else done(this.value);
	}

	override set = (value: any) => {
		if (this.inputs.length > 0) this.inputs[0].set(value);
		else this.value = value;
	}
}
