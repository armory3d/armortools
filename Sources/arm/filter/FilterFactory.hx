package arm.filter;

class FilterFactory
{

    public static var filterNames: Array<String> = ["None", "Color Correction"];

    public static function CreateFilter(type: String) : FilterBase {
        if (type == "None") return null;
        if (type == "Color Correction") return new ColorCorrection();

        return null;
    }

    public static function CreateFilterByIndex(index: Int) : FilterBase {
        return CreateFilter(filterNames[index]);
    }

}
