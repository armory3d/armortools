package arm.filter;

class FilterBase {

    // public var name: String;

    public function new() {}

    public function name() : String {
        return "";
    }

    public function draw() { /* paint ui of filter base. */ }

    public function getShaderText(color: String) : String { return ""; }
}
