package arm.filter;

import arm.node.MakeMaterial;

class FilterBase {

    // public var name: String;

    public function new() {}

    public function name() : String {
        return "";
    }

    public function draw() { /* paint ui of filter base. */ }

    public function getShaderText(color: String) : String { return ""; }

    public function update() {
        // Invoked when the parameter is updated
        MakeMaterial.parseMeshMaterial();
    }
}
