
class NullNode extends LogicNode {

	constructor() {
		super();
	}

	override get = (from: i32, done: (a: any)=>void) => {
		done(null);
	}
}
