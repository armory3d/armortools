package arm.filter;

class FilterFrame extends FilterBase
{

    public var filter: FilterBase = null;

    public function new() {
        this.name = "None";
    }

    override public function draw() {
        super.draw();
        if (filter != null) filter.draw();
    }

}
