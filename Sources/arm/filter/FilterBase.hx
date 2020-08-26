package arm.filter;

class FilterBase {

    public var name: String;

    public function draw() { /* paint ui of filter base. */ }

    public function getShaderText(color: String) : String { return ""; }
}
